/*
 * SUSE-Cli.c
 *
 *  Created on: 2 nov. 2019
 *      Author: utnso
 */

#include "SUSE-Cli.h"
#include <errno.h>


void* enviar_paquete(void*);
void* recibir_resultado(int* );


void* enviar_paquete(void*paquete){

	int peso;
	memcpy(&peso,paquete,sizeof(int));
	send(socket_suse_server,paquete,peso + sizeof(int), 0);
	int alocador;
	return recibir_resultado(&alocador);

}


void* recibir_resultado(int* alocador){

	void* buffer;

	int resultado = recv(socket_suse_server, alocador, 4, MSG_WAITALL);

	if(resultado!=0){
		buffer = malloc(*alocador);
		recv(socket_suse_server, buffer, *alocador, MSG_WAITALL);
		return buffer;
	}else{
		*alocador = 0;
		return NULL;
	}
}

void suse_close(int tid){
	void* paquete = serializar_suse_close(tid);
	enviar_paquete(paquete);
}
void suse_join(int tid){
	void* paquete = serializar_suse_join(tid);
	enviar_paquete(paquete);
}
void suse_signal(int tid,char* semaforo){
	void* paquete = serializar_suse_signal(tid,semaforo);
	enviar_paquete(paquete);
}
int suse_schedule_next(){
	void* paquete = serializar_suse_scheduler_next();
	void* resultado=enviar_paquete(paquete);
	int retorno;
	memcpy(&retorno, resultado,sizeof(int));
	return retorno;
}
void suse_wait(int tid, char* semaforo){
	void* paquete = serializar_suse_wait(tid,semaforo);
	enviar_paquete(paquete);
}
void suse_create(int tid){
	void* paquete = serializar_suse_create(tid);
	enviar_paquete(paquete);
}


void _hilolay_init(){

	void* paquete = serializar_hilolay_init();
	enviar_paquete(paquete);
}



void start_up(){

	char* nombre_de_config = readline("Ingresar nombre de config: \n >");
	config = config_create(nombre_de_config);
	//free(nombre_de_config);

}

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


void hilolay_init(void){

	char* nombre_de_config = readline("Ingresar nombre de config: \n >");
	config = config_create(nombre_de_config);

	int parm1;
	char** param2;
	conectar_con_servidor(parm1, param2[1]);

	_hilolay_init();



}







