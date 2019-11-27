/*
 * SUSE-Cli.h
 *
 *  Created on: 7 nov. 2019
 *      Author: utnso
 */

#ifndef SUSE_CLI_H_
#define SUSE_CLI_H_
#include "serializar.h"


void suse_close(int);
void suse_join(int);
void suse_signal(char*);
void suse_scheduler_next();
void suse_wait(char*);
void suse_create(int);

void start_up();
//void conectar_con_servidor(int argc, char* argv);

void conectar_con_servidor(int argc, char* argv) {

	char* server_name=config_get_string_value(config,"IP");
	int server_port, socket_fd;
	struct hostent *server_host;
	struct sockaddr_in server_address;

	/* Get server name from command line arguments or stdin. */


	/* Get server port from command line arguments or stdin. */
	server_port = argc > 2 ? atoi(argv[2]) : 0;
	if (!server_port) {
		server_port=config_get_int_value(config,"PUERTO_ESCUCHA");
	}

	/* Get server host from server name. */
	server_host = gethostbyname(server_name);

	/* Initialise IPv4 server address with server host. */
	memset(&server_address, 0, sizeof server_address);
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(server_port);
	memcpy(&server_address.sin_addr.s_addr, server_host->h_addr, server_host->h_length);

	/* Create TCP socket. */
	socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_fd == -1) {
		perror("Error en la creacion del socket");
		exit(1);
	}
	//printf(strerror(errno));



	/* Connect to socket with server address. */
	if (connect(socket_fd, (struct sockaddr *)&server_address, sizeof server_address) == -1) {
		perror("Error en la conexion del socket");
		exit(1);
	}

	socket_suse_server=socket_fd;

}


#endif /* SUSE_CLI_H_ */
