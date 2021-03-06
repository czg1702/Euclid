#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>

#include "utils.h"

void *mem_alloc_0(size_t size)
{
	void *addr = malloc(size);
	memset(addr, 0, size);
	return addr;
}

void *obj_alloc(size_t size, int type) {
	size += sizeof(int);
	int *obj_head = malloc(size);
	memset(obj_head, 0, size);
	*obj_head = type;
	return obj_head + 1;
}

int obj_type_of(void *obj) {
	return *(((int *) obj) - 1);
}

ssize_t read_sock_pkg(int sock_fd, void **buf, size_t *buf_len)
{
	int pkg_capacity;
	int bs_count;			  // count of bytes read at a time
	int buf_cursor = 0;		  // It also indicates the total number of bytes that have been read
	int remaining_retry = 10; // Retry attempts remaining

	do
	{
		bs_count = read(sock_fd, ((void *)&pkg_capacity) + buf_cursor, sizeof(pkg_capacity) - buf_cursor);

		if (bs_count < 0 || (bs_count == 0 && remaining_retry-- == 0))
			return -1;

		buf_cursor += bs_count;

	} while (buf_cursor < sizeof(pkg_capacity));

	if (pkg_capacity <= sizeof(pkg_capacity))
		return -1;

	*buf = (char *)mem_alloc_0(pkg_capacity); // memory buffer must be released elsewhere!
	memcpy(*buf, &pkg_capacity, sizeof(pkg_capacity));

	while (buf_cursor < pkg_capacity)
	{
		bs_count = read(sock_fd, *buf + buf_cursor, pkg_capacity - buf_cursor);
		buf_cursor += bs_count;
	}

	return *buf_len = pkg_capacity;
}

int release_mem(void *addr)
{
	// TODO 2022年5月17日20:19:38
	// free(addr);
	// printf("[ func - release_mem ] This function has not been implemented yet.\n");
	return 0;
}

LinkedQueue *create_lnk_queue()
{
	return (LinkedQueue *)mem_alloc_0(sizeof(LinkedQueue));
}

int lnk_q_add_obj(LinkedQueue *lnk_q, void *obj)
{
	LinkedQueueNode *n = (LinkedQueueNode *)mem_alloc_0(sizeof(LinkedQueueNode));
	n->obj = obj;

	if (lnk_q->tail)
	{
		(lnk_q->tail)->next = n;
		lnk_q->tail = n;
	}
	else
	{
		lnk_q->tail = lnk_q->head = n;
	}

	return 0;
}

void *lnk_q_get(LinkedQueue *lnk_q)
{
	if (lnk_q->head == NULL)
		return NULL;

	LinkedQueueNode *cn = lnk_q->head;
	if (lnk_q->head == lnk_q->tail)
	{
		lnk_q->head = lnk_q->tail = NULL;
	}
	else
	{
		lnk_q->head = cn->next;
	}

	void *res = cn->obj;
	release_mem(cn);
	return res;
}

int sock_conn_to(int *sock_fd, char *ip, int port)
{
	*sock_fd = socket(AF_INET, SOCK_STREAM, 0);

	struct sockaddr_in serv_addr;
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port);
	serv_addr.sin_addr.s_addr = inet_addr(ip);

	if (connect(*sock_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == 0)
		return 0;

	printf("[utils] error. connect %s:%d\n", ip, port);
	return -1;
}

StrArr *str_split(char *orig_str, char *deli_str)
{
	int len = 1;
	char *p = orig_str;

	while (p = strstr(p, deli_str))
	{
		p += strlen(deli_str);
		len++;
	}

	StrArr *str_arr = mem_alloc_0(sizeof(StrArr));
	str_arr->length = len;

	str_arr->head_str_p = mem_alloc_0(sizeof(void *) * len + 1 + strlen(orig_str));

	strcpy(((char *)str_arr->head_str_p) + sizeof(void *) * len, orig_str);
	p = ((char *)str_arr->head_str_p) + sizeof(void *) * len;

	len = 1;
	*(str_arr->head_str_p) = p;

	while (p = strstr(p, deli_str))
	{
		*p = '\0';
		p += strlen(deli_str);
		*(str_arr->head_str_p + len) = p;
		len++;
	}

	show_StrArr(str_arr);
	return str_arr;
}

void destory_StrArr(StrArr *arr_address)
{
	if (!arr_address)
		return;

	release_mem(arr_address->head_str_p);
	release_mem(arr_address);
}

void show_StrArr(StrArr *arr)
{
	printf("\n");
	int i;
	for (i = 0; i < arr->length; i++)
	{
		printf("%d\t%s\n", i, *(arr->head_str_p + i));
	}
	printf("\n");
}

char *str_arr_get(StrArr *sa, unsigned int i)
{
	if (sa == NULL || i >= sa->length)
		return NULL;

	return *((sa->head_str_p) + i);
}

int stack_push(Stack *s, void *obj)
{
	if (s == NULL || s->top_idx == STACK_MAX_DEEP)
		return -1;

	(s->bucket)[(s->top_idx)++] = obj;

	return 0;
}

int stack_pop(Stack *s, void **addr)
{
	if (s == NULL || s->top_idx == 0)
		return -1;

	*addr = (s->bucket)[--(s->top_idx)];

	return 0;
}

char *str_clone(char *str)
{
	if (str == NULL)
		return NULL;

	char *str_b_cl = (char *)mem_alloc_0(strlen(str) + 1);

	strcpy(str_b_cl, str);

	return str_b_cl;
}

int open_serv_sock(int *ss_fd_p, int port)
{
	int ss_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (ss_fd < 0)
	{
		printf("error at fn:open_serv_sock\n");
		return -1;
	}

	struct sockaddr_in serv_addr;
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port);
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(ss_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
		goto failed_close_exit;

	if (listen(ss_fd, SOMAXCONN) < 0)
		goto failed_close_exit;

	*ss_fd_p = ss_fd;
	return 0;

failed_close_exit:
	printf("failed at fn:open_serv_sock.\n");
	if (close(ss_fd) != 0)
		printf("failed at fn:open_serv_sock.\n");

	return -1;
}

long now_microseconds()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_sec * 1000000 + tv.tv_usec;
}

int append_file_data(char *file_path, char *data, size_t sz)
{
	char f[128];
	memset(f, 0, 128);
	getcwd(f, 80);
	if (*file_path != '/')
		strcat(f, "/");
	strcat(f, file_path);
	FILE *fp = fopen(f, "a+");
	fwrite(data, sz, 1, fp);
	fclose(fp);
	return 0;
}

FILE *open_file(char *r_path, char *modes)
{
	char f[128];
	memset(f, 0, 128);
	getcwd(f, 80);
	if (*r_path != '/')
		strcat(f, "/");
	strcat(f, r_path);
	return fopen(f, "a+");
}

// void file_stat(char *data_file, struct stat *f_stat)
// {
// 	char f[128];
// 	memset(f, 0, 128);
// 	getcwd(f, 80);
// 	if (*data_file != '/')
// 		strcat(f, "/");
// 	strcat(f, data_file);

// 	stat(f, f_stat);
// }

int append_file_uint(char *file_path, __uint32_t val)
{
	return append_file_data(file_path, (char *)&val, sizeof(val));
}

ArrayList *als_create(unsigned int init_capacity, char *desc)
{
	ArrayList *als = (ArrayList *)mem_alloc_0(sizeof(ArrayList));
	// printf("[debug] 新建 ArrayList  ---------------  % 12u% 12u\n",als->idx,als->ele_arr_capacity);
	als->ele_arr_capacity = init_capacity;
	als->elements_arr_p = mem_alloc_0(sizeof(void *) * init_capacity);
	if (desc != NULL)
	{
		int desc_len = strlen(desc);
		desc_len = desc_len < COMMON_OBJ_DESC_LEN ? desc_len : COMMON_OBJ_DESC_LEN - 1;
		memcpy(als->desc, desc, desc_len);
	}

	return als;
}

int als_add(ArrayList *als, void *obj)
{
	if (als == NULL)
		return -1;

	// // printf("[debug] KKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKK% 20u% 20u\n", als->idx, als->ele_arr_capacity);

	if (als->idx >= als->ele_arr_capacity)
	{
		// // printf("[debug] @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@  %p  < %u >  < %u >\n", als, als->idx, als->ele_arr_capacity);
		als->ele_arr_capacity *= 2;
		void **new_ele_arr_p = mem_alloc_0(sizeof(void *) * (als->ele_arr_capacity));
		memcpy(new_ele_arr_p, als->elements_arr_p, als->idx * sizeof(void *));
		release_mem(als->elements_arr_p);
		als->elements_arr_p = new_ele_arr_p;
	}

	(als->elements_arr_p)[als->idx++] = obj;

	return 0;
}

void *als_get(ArrayList *als, unsigned int position)
{
	return position < als->idx ? als->elements_arr_p[position] : NULL;
}

unsigned int als_size(ArrayList *als)
{
	return als ? als->idx : -1;
}

int als_remove(ArrayList *als, void *obj)
{
	int i;
	for (i = 0; i < als->idx; i++)
	{
		if (*((int *)(als->elements_arr_p[i])) != *((int *)obj))
			continue;

		als->idx--;
		for (; i < als->idx; i++)
		{
			als->elements_arr_p[i] = als->elements_arr_p[i + 1];
		}
		return 0;
	}
	printf("INFO - als_remove ... no object < %p >\n", obj);
	return 0;
}

void *slide_over_mem(void *addr, ssize_t range, size_t *idx)
{
	size_t _idx = *idx;
	*idx += range;
	return addr + _idx;
}