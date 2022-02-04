#include <stdlib.h>

#include "cfg.h"
#include "net.h"
#include "command.h"
#include "vce.h"

int main(int argc, char *argv[])
{
	init_cfg(argc, argv);

	vce_init();

	init_command_module();

	if (get_cfg()->mode == WORKER_MODE)
		join_cluster();

	net_service_startup();

	return EXIT_SUCCESS;
}
