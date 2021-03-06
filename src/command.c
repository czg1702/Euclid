#include <stdio.h>
#include <string.h>

#include "command.h"
#include "utils.h"
#include "cfg.h"
#include "mdx.h"
#include "mdd.h"

extern Stack YC_STC;

// CCI - constant command intention
static EuclidCommand *CCI_ALLOW;
static EuclidCommand *CCI_CHILD_NODE_JOIN;
static EuclidCommand *CCI_TERML_CTRL;

static LinkedQueue *_ec_pool;
static pthread_mutex_t _ec_p_mtx;
static pthread_cond_t _ec_p_cond;

static int command_processor_thread_startup();

// static pthread_t _command_processor_thread_id;

static void *do_process_command(void *addr);

static int execute_command(EuclidCommand *ec);

extern void *parse_mdx(char *mdx);

int init_command_module()
{
	// init CCI_ALLOW
	void *addr = mem_alloc_0(SZOF_USG_INT + SZOF_USG_SHORT);
	*((unsigned int *)addr) = SZOF_USG_INT + SZOF_USG_SHORT;
	*((unsigned short *)(addr + SZOF_USG_INT)) = INTENT__ALLOW;
	CCI_ALLOW = create_command(addr);

	// init CCI_CHILD_NODE_JOIN
	addr = mem_alloc_0(SZOF_USG_INT + SZOF_USG_SHORT);
	*((unsigned int *)addr) = SZOF_USG_INT + SZOF_USG_SHORT;
	*((unsigned short *)(addr + SZOF_USG_INT)) = INTENT__CHILD_NODE_JOIN;
	CCI_CHILD_NODE_JOIN = create_command(addr);

	// init CCI_TERML_CTRL
	addr = mem_alloc_0(SZOF_USG_INT + SZOF_USG_SHORT);
	*((unsigned int *)addr) = SZOF_USG_INT + SZOF_USG_SHORT;
	*((unsigned short *)(addr + SZOF_USG_INT)) = INTENT__TERMINAL_CONTROL;
	CCI_TERML_CTRL = create_command(addr);

	// init EuclidCommand pool and mutex and cond
	// init_LinkedList(&_ec_pool);
	_ec_pool = create_lnk_queue();
	pthread_mutex_init(&_ec_p_mtx, NULL);
	pthread_cond_init(&_ec_p_cond, NULL);

	command_processor_thread_startup();

	return 0;
}

EuclidCommand *create_command(char *bytes)
{
	EuclidCommand *ec_p = (EuclidCommand *)mem_alloc_0(sizeof(EuclidCommand));
	ec_p->bytes = bytes;
	return ec_p;
}

intent ec_get_intent(EuclidCommand *ec)
{
	if (ec == NULL || ec->bytes == NULL)
		return INTENT__UNKNOWN;

	return *((intent *)((ec->bytes) + sizeof(int)));
}

int ec_release(EuclidCommand *ec)
{
	int a = release_mem(ec->bytes);
	int b = release_mem(ec);
	return (a == 0) && (b == 0);
}

EuclidCommand *get_const_command_intent(intent inte)
{
	if (inte == INTENT__ALLOW)
		return CCI_ALLOW;

	if (inte == INTENT__CHILD_NODE_JOIN)
		return CCI_CHILD_NODE_JOIN;

	if (inte == INTENT__TERMINAL_CONTROL)
		return CCI_TERML_CTRL;

	return NULL;
}

int ec_get_capacity(EuclidCommand *ec)
{
	return *((int *)(ec->bytes));
}

int submit_command(EuclidCommand *ec)
{
	pthread_mutex_lock(&_ec_p_mtx);
	int res = lnk_q_add_obj(_ec_pool, ec);
	pthread_cond_signal(&_ec_p_cond);
	pthread_mutex_unlock(&_ec_p_mtx);
	return res;
}

static int command_processor_thread_startup()
{
	int i;
	for (i = 0; i < get_cfg()->ec_threads_count; i++)
	{
		pthread_t thread_id;
		pthread_create(&thread_id, NULL, do_process_command, NULL);
		int detach_r = pthread_detach(thread_id);
		printf("EuclidCommand processor thread [%lu] <%d>.\n", thread_id, detach_r);
	}

	return 0;
}

static void *do_process_command(void *addr)
{
	EuclidCommand *ec = NULL;
	while (1)
	{
		pthread_mutex_lock(&_ec_p_mtx);
		while (ec == NULL)
		{
			pthread_cond_wait(&_ec_p_cond, &_ec_p_mtx);
			ec = (EuclidCommand *)lnk_q_get(_ec_pool);
		}
		pthread_mutex_unlock(&_ec_p_mtx);

		execute_command(ec);

		ec = NULL;
	}

	return NULL;
}

static int execute_command(EuclidCommand *ec)
{
	intent inte = ec_get_intent(ec);
	if (inte == INTENT__INSERT_CUBE_MEARSURE_VALS)
	{
		distribute_store_measure(ec);
	}
	else if (inte == INTENT__MDX)
	{
		parse_mdx((ec->bytes) + 10);
		void *ids_type;
		stack_pop(&YC_STC, &ids_type);
		if (ids_type == IDS_STRLS_CRTDIMS)
		{
			ArrayList *dim_names_ls;
			stack_pop(&YC_STC, (void **)&dim_names_ls);
			create_dims(dim_names_ls);
		}
		else if (ids_type == IDS_STRLS_CRTMBRS)
		{
			ArrayList *mbrs_info_als;
			stack_pop(&YC_STC, (void **)&mbrs_info_als);
			create_members(mbrs_info_als);
		}
		else if (ids_type == IDS_OBJLS_BIUCUBE)
		{
			ArrayList *measures_ls, *dims_roles_ls;
			char *cube_name;
			stack_pop(&YC_STC, (void **)&measures_ls);
			stack_pop(&YC_STC, (void **)&dims_roles_ls);
			stack_pop(&YC_STC, (void **)&cube_name);
			build_cube(cube_name, dims_roles_ls, measures_ls);
		}
		else if (ids_type == IDS_CXOBJ_ISRTCUBEMEARS)
		{
			ArrayList *ls_vms;
			char *cube_name;
			stack_pop(&YC_STC, (void **)&ls_vms);
			stack_pop(&YC_STC, (void **)&cube_name);
			// // // printf("[debug] IDS_CXOBJ_ISRTCUBEMEARS - %s - %s\n", cube_name, ls_vms->desc);
			insert_cube_measure_vals(cube_name, ls_vms);
		}
		else if (ids_type == IDS_MULTI_DIM_SELECT_DEF)
		{
			SelectDef *select_def;
			stack_pop(&YC_STC, (void **)&select_def);
			exe_multi_dim_queries(select_def);
			printf("// TODO ....................... %s:%d\n", __FILE__, __LINE__); // TODO should be return a multi-dim-result
		}
	}
	return 0;
}

EuclidCommand *build_intent_command_mdx(char *mdx)
{
	unsigned int capacity = SZOF_USG_INT + SZOF_USG_SHORT + SZOF_USG_INT + strlen(mdx);

	EuclidCommand *ec = (EuclidCommand *)mem_alloc_0(sizeof(EuclidCommand));
	void *addr = mem_alloc_0(capacity);

	*((unsigned int *)addr) = capacity;
	*((unsigned short *)(addr + SZOF_USG_INT)) = INTENT__MDX;
	*((unsigned int *)(addr + SZOF_USG_INT + SZOF_USG_SHORT)) = strlen(mdx);

	memcpy(addr + SZOF_USG_INT + SZOF_USG_SHORT + SZOF_USG_INT, mdx, strlen(mdx));

	ec->bytes = addr;

	return ec;
}
