#include "mist.h"

/*---------------------------------------------------------------------------*/
PROCESS(router_node_process, "Router node process");
AUTOSTART_PROCESSES(&router_node_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(router_node_process, ev, data)
{
  PROCESS_BEGIN();

  /* Set us up as a RPL root node. */
  printf("RNode RPL init\r\n");
  simple_rpl_init_dag();

  /* Initialize the IP64 module so we'll start translating packets */
  ip64_init();
  printf("Router node begin!\r\n");
  /* ... and do nothing more. */

  while(1) {
    PROCESS_WAIT_EVENT();
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
