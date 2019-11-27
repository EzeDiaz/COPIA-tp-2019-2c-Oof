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

	if(recv(socket_suse_server, alocador, 4, MSG_WAITALL)!=0){
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
void suse_signal(char* semaforo){
	void* paquete = serializar_suse_signal(semaforo);
	enviar_paquete(paquete);
}
void suse_schedule_next(){
	void* paquete = serializar_suse_scheduler_next();
	enviar_paquete(paquete);
}
void suse_wait(char* semaforo){
	void* paquete = serializar_suse_wait(semaforo);
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










