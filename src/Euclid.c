#include <stdlib.h>
// #include <pthread.h>

#include "cfg.h"
#include "net.h"
#include "command.h"
#include "vce.h"

// extern pthread_mutex_t mutex_AAA;

int main(int argc, char *argv[])
{
	// pthread_mutex_init(&mutex_AAA, NULL);

	init_cfg(argc, argv);

	vce_init();

	init_command_module();

	if (get_cfg()->mode == WORKER_MODE)
		join_cluster();

	net_service_startup();

	return EXIT_SUCCESS;
}
