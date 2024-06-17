#pragma once

struct udp_tools {
	struct sockaddr_in *partner;
	int socket;
};

typedef struct udp_tools UdpTools;
int set_udp_server(int port);
unsigned long get_ip_address(char *str_addr);
UdpTools *set_socket_udp_client(unsigned long server_ip, int server_port, int client_port);
int connect_to_server(UdpTools *tools, char *url, int port);
UdpTools *connect_to_client(int s);
