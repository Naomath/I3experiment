#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/select.h>
#include <string.h>

#include "udp_wrapper.h"

#define N 10000

int main(int argc, char **argv) {
	if(argc != 2 && argc != 6) {
		perror("For server, $./thread <MY_PORT_NUM>,or for client, $./thread <NGROK_URL> <NGROK_PORT_NUM> <SERVER_URL> <SERVER_PORT_NUM> <MY_PORT_NUM>");
		exit(1);
	}

	UdpTools *tools = NULL;

	if(argc == 2) {
		//server
		int socket = set_udp_server(atoi(argv[1]));
		tools = connect_to_client(socket);
		printf("Connect to client\n");
	} else if (argc == 6){
		//client
		unsigned long server_ip = get_ip_address(argv[3]);
		printf("server IP:%lx\n", server_ip);	
		int server_port = atoi(argv[4]);
		int client_port = atoi(argv[5]);
		tools = set_socket_udp_client(server_ip, server_port, client_port);
		connect_to_server(tools, argv[1], atoi(argv[2]));
		printf("Connect to server\n");
	}
	FILE *fp;
    char *cmdline = "rec -t raw -b 16 -c 1 -e s -r 48000 -";
    if ((fp=popen(cmdline, "r")) == NULL) {
        perror ("popen failed");
        exit(EXIT_FAILURE);
    }
	int fd = fileno(fp);
	unsigned char send_data[N];
	unsigned char recv_data[N];

	struct timeval tv;
    fd_set readfds, fds;
    int ret_select;
    //int ret_recv;

    /* タイムアウト時間を設定 */
    tv.tv_sec = 0;
    tv.tv_usec = 30;

    /* 読み込みFD集合を空にする */
    FD_ZERO(&readfds);
    // 読み込みFD集合にsockfdを追加
    FD_SET(tools->socket, &readfds);
    while (1)
    {
		memcpy(&fds, &readfds, sizeof(fd_set));
        ret_select = select(tools->socket + 1, &fds, NULL, NULL, &tv);
        if (ret_select == -1) {
            /* select関数がエラー */
            printf("select error\n");
            return -1;
        }

        if (ret_select == 0) {
            /* 読み込み可能になったFDの数が0なのでタイムアウトと判断 */
            int send_size = read(fd, send_data, N);
            sendto(tools->socket, send_data, send_size, 0 , (struct sockaddr *)tools->partner, sizeof(*(tools->partner)));
        }
        else {
            /* 読み込み可能 */
            struct sockaddr_in client;
            socklen_t client_len = sizeof(client);
            int n = recvfrom(tools->socket, recv_data, N, 0, (struct sockaddr *)&client, &client_len);
            write(1, recv_data, n);
        }

    }
	pclose(fp);
}
