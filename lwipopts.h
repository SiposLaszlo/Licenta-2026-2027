
#ifndef LWIPOPTS_H
#define LWIPOPTS_H

#define NO_SYS 1
#define LWIP_SOCKET 0
#define LWIP_NETCONN 0

#define LWIP_IPV4 1
#define LWIP_TCP 1
#define LWIP_DHCP 1
#define LWIP_DNS 1
#define LWIP_MQTT 1

#define MEM_ALIGNMENT 4
#define MEM_SIZE 16000

#define MEMP_NUM_TCP_PCB 10
#define MEMP_NUM_TCP_SEG 32
#define MEMP_NUM_SYS_TIMEOUT 12

#define PBUF_POOL_SIZE 24

#define TCP_MSS 1460
#define TCP_WND (8 * TCP_MSS)
#define TCP_SND_BUF (8 * TCP_MSS)
#define TCP_SND_QUEUELEN 32

#define LWIP_NETIF_STATUS_CALLBACK 1
#define LWIP_NETIF_LINK_CALLBACK 1

#define LWIP_STATS 0
#define LWIP_PROVIDE_ERRNO 1

#endif
