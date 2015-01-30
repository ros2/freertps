#include "freertps/freertps.h"
#include "freertps/sdp.h"
#include <sys/types.h>
#include <sys/socket.h>

//static int g_freertps_sdp_mcast_sock;
//static sockaddr_in g_freertps_sdp_mcast_

void freertps_sdp_init()
{
  FREERTPS_INFO("sdp init\n");
  //g_freertps_sdp_mcast_sock = socket(AF_INET, SOCK_DGRAM, 0);
  //freertps_perish_if(g_freertps_sdp_mcast_sock < 0, "couldn't get socket");
  freertps_hal_udp_add_mcast_rx_port(7400);
}

void freertps_sdp_fini()
{
  // sentinel this someday
  //close(g_freertps_sdp_mcast_sock);
}
