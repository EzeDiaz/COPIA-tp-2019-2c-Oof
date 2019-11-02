/*
 * SUSE-Cli.c
 *
 *  Created on: 2 nov. 2019
 *      Author: utnso
 */

#include "globales.h"
#include <sys/socket.h>

void* enviar_paquete(void*);
void* recibir_resultado(int* );

int main(){
	return 0;
}

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
		return buffer;
	}
}
