/*
 * get_local_IP.c
 *
 *  Created on: 28 nov. 2019
 *      Author: utnso
 */
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

//Sacado de: https://stackoverflow.com/questions/1570511/c-code-to-get-the-ip-address
//(tiene hechos algunos ajustes)

//A la variable que reciba la IP, se le debera hacer un free

char* get_local_IP() {

	struct ifaddrs *ifaddr, *ifa;
	int family, s;
	char host[NI_MAXHOST]; //Tener en cuenta NI_MAXHOST para el malloc al recibir la IP

	if (getifaddrs(&ifaddr) == -1) {
		perror("getifaddrs");
		exit(EXIT_FAILURE);
	}

	for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
		family = ifa->ifa_addr->sa_family;

		if (family == AF_INET) {
			s = getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in),
					host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
			if (s != 0) {
				printf("getnameinfo() failed: %s\n", gai_strerror(s));
				exit(EXIT_FAILURE);
			}
			if(!strcmp(ifa->ifa_name,"enp0s3")){
				char* ip_a_retornar = (char*)malloc(strlen(host)+1);
				memset(ip_a_retornar, '\0', sizeof(ip_a_retornar));
				strcat(ip_a_retornar,host);

				free(ifaddr);

				return ip_a_retornar;
			}
		}
	}

	return "0"; //En otro caso devuelve 0 (no deberia pasar)
}

