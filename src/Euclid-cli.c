#include <stdio.h>
#include <string.h>

#include "cfg.h"
//#include "net.h"
#include "command.h"
#include "utils.h"

// extern void *parse_mdx(char *mdx);

int main(int argc, char *argv[])
{
	if (argc < 2)
		return 1;

	int i, statement_len = 0;
	for (i = 1; i < argc; i++) {
		statement_len += strlen(argv[i]) + 1;
	}
	char *statement = (char *) mem_alloc_0(statement_len);
	for (i = 1; i < argc; i++) {
		strncat(statement, argv[i], strlen(argv[i]));
		if (i < argc - 1)
			strncat(statement, " ", 1);
	}

	printf("<<<<<<\n%s\n>>>>>>\n", statement);

	// parse_mdx(statement);

	init_cfg(argc, argv);

	init_command_module();

	EuclidConfig *cfg = get_cfg();

	int sock_fd;
	sock_conn_to(&sock_fd, cfg -> cli_ctrl_node_host, cfg -> cli_ctrl_node_port);

	EuclidCommand *terml_ctrl_ec = get_const_command_intent(INTENT__TERMINAL_CONTROL);

	send(sock_fd, terml_ctrl_ec -> bytes, ec_get_capacity(terml_ctrl_ec), 0);

	void *buf = NULL;
	size_t buf_len;
	if (read_sock_pkg(sock_fd, &buf, &buf_len) < 0) {
		if (buf)
			release_mem(buf);
		goto _exit_;
	}

	EuclidCommand *allow_f_serv = create_command(buf);
	if(ec_get_intent(allow_f_serv) != INTENT__ALLOW) {
		goto _exit_;
	}

	EuclidCommand *mdx_ec = build_intent_command_mdx(statement);
	send(sock_fd, mdx_ec -> bytes, ec_get_capacity(mdx_ec), 0);

	_exit_:
	close(sock_fd);

	return 0;
}
