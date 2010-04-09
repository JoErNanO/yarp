// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-


#include "yarpmin.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>

#ifdef WIN32
#include <winsock2.h>
#else
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#endif

#ifdef WIN32
#define SOCK_CAST(x) ((SOCKET)x)
#else
#define SOCK_CAST(x) ((int)x)
#endif

#if WIN32
#define safe_printf sprintf_s
#else
#define safe_printf snprintf
#endif 

static int yarp_is_initialized = 0;
static yarpAddress yarp_server;

yarpAddressPtr yarp_init() {
    if (yarp_is_initialized) {
        return &yarp_server;
    }
#ifdef WIN32
    int     wsaRc;
    WSADATA wsaData;
    if(wsaRc=WSAStartup(0x0101, &wsaData)) {
        perror("WinSock init");
    }
	if(wsaData.wVersion != 0x0101) {
        WSACleanup();
        perror("wsaData.wVersion");
    }
#endif

    // these lines should actually consult yarp config files, this
    // assumes the name server is on the local machine
    strncpy(yarp_server.host,"127.0.0.1",sizeof(yarp_server.host));
    yarp_server.port_number = 10000;
    yarp_is_initialized = 1;
    return &yarp_server;
}

void yarp_fini() {
}


yarpConnection yarp_connect(yarpAddressPtr address) {
#ifdef WIN32
    SOCKET sd;
#else
	int sd;
#endif
	//struct sockaddr_in sin;
	struct sockaddr_in pin;
	struct hostent *hp;

	// get host information
	if ((hp = gethostbyname(address->host)) == 0) {
		perror("gethostbyname");
		exit(1);
	}

	// set up socket structure
	memset(&pin, 0, sizeof(pin));
	pin.sin_family = AF_INET;
	pin.sin_addr.s_addr = ((struct in_addr *)(hp->h_addr))->s_addr;
	pin.sin_port = htons(address->port_number);

	// get a socket
	if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("socket");
		exit(1);
	}

	// connect to PORT on HOST
	if (connect(sd,(struct sockaddr *)  &pin, sizeof(pin)) == -1) {
		perror("connect");
		exit(1);
	}

    return (long int) sd;
}


int yarp_is_valid(yarpConnection connection) {
#ifdef WIN32
    return (connection != INVALID_SOCKET);
#else
    return (connection != -1);
#endif
}


void yarp_disconnect(yarpConnection connection) {
    if (yarp_is_valid(connection)) {
#ifdef WIN32
        closesocket(SOCK_CAST(connection));
#else
        close(SOCK_CAST(connection));
#endif
    }
}


int yarp_send(yarpConnection connection, const char *msg) {
	// send a message to the server PORT on machine HOST
	if (send(SOCK_CAST(connection), msg, strlen(msg), 0) == -1) {
		perror("send");
		exit(1);
	}
    return 0;
}


int yarp_receive(yarpConnection connection, char *buf, int len) {
    int res = recv(SOCK_CAST(connection), buf, len, 0);
    return res;
}


int yarp_receive_line(yarpConnection connection, char *buf, int len) {
    int i;
    int res = 0;
    // wait for a message to come back from the server
	for (i=0; i<len; i++) {
        buf[i] = '\0';
	}
    while (len>0) {
        res = yarp_receive(connection,buf,len);
        if (res<0) {
            break;
        }
        for (i=0; i<res; i++) {
            if (buf[i]=='\n'||buf[i]=='\r') {
                buf[i] = '\0';
                return 0;
            }
        }
        buf += res;
        len -= res;
    }
    return res;
}


int yarp_parse_registration(yarpAddressPtr address, char *buf) {
    int in_group = 0;
    int expect_ip = 0;
    int expect_port_number = 0;
    int got_ip = 0;
    int got_port_number = 0;
    char txt[1000];
    char at = 0;
    int i;
    address->host[0] = '\0';
    address->port_number = -1;
    if (strncmp(buf,"registration name ",18)!=0) {
        return -1;
    }
    for (i=0; i<strlen(buf)+1; i++) {
        char ch = buf[i];
        if (ch!=' '&&ch!='\t'&&ch!='\r'&&ch!='\n'&&ch!='\0') {
            in_group = 1;
            if (at<sizeof(txt)-1) {
                txt[at] = ch;
                at++;
                txt[at] = '\0';
            }
        } else {
            if (in_group) {
                if (expect_ip) {
                    strncpy(address->host,txt,sizeof(address->host));
                    got_ip = 1;
                }
                if (expect_port_number) {
                    address->port_number = atoi(txt);
                    got_port_number = 1;
                }
                in_group = 0;
                expect_ip = 0;
                expect_port_number = 0;
                if (strcmp(txt,"ip")==0) {
                    expect_ip = 1;
                } else if (strcmp(txt,"port")==0) {
                    expect_port_number = 1;
                }
                at = 0;
                txt[at] = '\0';
            }
        }
    }
    
    if (!(got_ip&&got_port_number)) {
        return -1;
    }

    return 0;
}


int yarp_port_lookup(yarpAddressPtr address, const char *port_name) {
    char query[1000];
    char buf[1000];
    int res;
    safe_printf(query,sizeof(query),"query %s", port_name);
    yarp_rpc(&yarp_server, query, buf, sizeof(buf));
    return yarp_parse_registration(address,buf);
}


int yarp_rpc(yarpAddressPtr address, const char *msg, char *buf, int len) {
    yarpConnection con = yarp_connect(address);
    char ibuf[1000];
    if (!yarp_is_valid(con)) {
        printf("Cannot open connection\n");
        exit(1);
    }
    yarp_send(con,"CONNACK cclient\n");
    yarp_receive_line(con,ibuf,sizeof(ibuf));
    yarp_send(con,"d\n");
    yarp_send(con,msg);
    yarp_send(con,"\n");
    int res = yarp_receive_line(con,buf,len);
    yarp_disconnect(con);
    return res;
}


yarpConnection yarp_prepare_to_read(yarpAddressPtr address) {
    yarpConnection con = yarp_connect(address);
    char ibuf[1000];
    if (!yarp_is_valid(con)) {
        printf("Cannot open connection\n");
        exit(1);
    }
    yarp_send(con,"CONNECT cclient\n");
    yarp_receive_line(con,ibuf,sizeof(ibuf));
    yarp_send(con,"r\n"); // reverse the connection
    return con;
}
