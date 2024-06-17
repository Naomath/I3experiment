#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>

#include "udp_wrapper.h"

#define SYN "aa"
#define ACK_SYN "bb"
#define data "d"
#define header_bytes 2
#define N 100

int set_udp_server(int port) {
	int s = socket(AF_INET, SOCK_DGRAM, 0);
	if (s < 0 ) {
		perror("Fail to create socket of server.\n");
		exit(1);
	}

	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);

	if(bind(s, (struct sockaddr *) &addr, sizeof(addr)) == -1) {
		perror("Failed to bind socket.\n");
		exit(1);
	}
	
	return s;
}

unsigned long get_ip_address(char *str_addr) {
	unsigned long ip_addr = inet_addr(str_addr);
	if(ip_addr == -1) {
		//ホストがurlで指定して入力された場合
		struct hostent *host = gethostbyname(str_addr);
		//名前解決
		if(host == NULL) {
			perror("Failed to resolve %s to IP address.\n");
			exit(1);
		}
		memcpy((char *) &ip_addr, (char *) host->h_addr_list[0], host->h_length);
	}
	return ip_addr;
}

UdpTools *set_socket_udp_client(unsigned long server_ip, int server_port, int client_port) {
	int s = socket(AF_INET, SOCK_DGRAM, 0);
	if (s < 0 ) {
		perror("Fail to create socket of server.\n");
		exit(1);
	}

	struct sockaddr_in client_addr;
	client_addr.sin_family = AF_INET;
	client_addr.sin_port = htons(client_port);
	client_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	if(bind(s, (struct sockaddr *) &client_addr, sizeof(client_addr)) == -1) {
		perror("Failed to bind socket.\n");
		exit(1);
	}

	
	struct sockaddr_in *server_addr = (struct sockaddr_in *)malloc(sizeof(struct sockaddr_in));
	server_addr->sin_family = AF_INET;
	server_addr->sin_port = htons(server_port);
	server_addr->sin_addr.s_addr = server_ip;
	
	UdpTools *tools = (UdpTools *)malloc(sizeof(UdpTools));
	tools->partner = server_addr;
	tools->socket = s;
	return tools;
}

int connect_to_server(UdpTools *tools, char *url, int port) {
	struct sockaddr_in *server = tools->partner;
	int s = tools->socket;

	int url_len = strlen(url);
	int send_size = header_bytes + 4 + url_len + 4;
	unsigned char send_data[send_size];
	memcpy(send_data, (unsigned char *)SYN, header_bytes);
	memcpy(send_data+header_bytes, (unsigned char *)&url_len, 4);
	memcpy(send_data+header_bytes+4, (unsigned char *)url, url_len);
	memcpy(send_data+header_bytes+4+url_len, (unsigned char *)&port, 4);

	struct timeval tv;
    tv.tv_sec = 1;  // 1秒でタイムアウト
	tv.tv_usec = 0; 
    setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
		
    socklen_t server_len = sizeof(*server);
	while(1) {
		//送るデータは最初の1byteがheader
		//次にurlのbytes数(url_len)としてint
		//三つめに、urlとしてurl_len
		//四つめにportとしてint 4bytes
		printf("Send SYN to server\n");
		sendto(s, send_data, send_size, 0 , (struct sockaddr *)server, server_len);
		int n = recvfrom(s, data, N, 0, (struct sockaddr *)server, &server_len);
		if(n != 0 && memcmp(ACK_SYN, data, header_bytes) == 0) {
			//サーバーの応答あり
			return 0;
		} 
	}
}

UdpTools *connect_to_client(int s) {
	struct sockaddr_in *client = (struct sockaddr_in *)malloc(sizeof(struct sockaddr_in));
	unsigned char recv_data[N];
	
	struct timeval tv;
    tv.tv_sec = 1;  // 1秒でタイムアウト
	tv.tv_usec = 0; 
    setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
		
	socklen_t client_len = sizeof(*client);
	while(1) {
		int n = recvfrom(s, data, N, 0, (struct sockaddr *)client, &client_len);
		if(n != 0 && memcmp(SYN, recv_data, header_bytes) == 0) {
			//クライアントから接続あり
			char *url[50];
			int url_len;
			int port;
			memcpy((unsigned char *)&url_len, recv_data+header_bytes, 4);
			memcpy((unsigned char *)url, recv_data+header_bytes+4, url_len);
			memcpy((unsigned char *)&port, recv_data+header_bytes+4+url_len, 4);
			
			unsigned long host_ip = get_ip_address((char *)url);
			//受け取ったクライアントのurlからipアドレスへと名前解決

			struct sockaddr_in *client_addr = (struct sockaddr_in *)malloc(sizeof(struct sockaddr_in));
			client_addr->sin_family = AF_INET;
			client_addr->sin_port = htons(port);
			client_addr->sin_addr.s_addr = host_ip;
			
			UdpTools *tools = (UdpTools *)malloc(sizeof(UdpTools));
			tools->partner = client_addr;
			tools->socket = s;

			unsigned char send_data[10];
			memcpy((char *)send_data, (char *)ACK_SYN, header_bytes);
			int send_size = header_bytes;

			sendto(s, send_data, send_size, 0 , (struct sockaddr *)client_addr, sizeof(*client_addr));
			return tools;
		}
	}
}

