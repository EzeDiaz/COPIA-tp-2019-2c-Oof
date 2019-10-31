/*
 * serializacion.c
 *
 *  Created on: 30 oct. 2019
 *      Author: utnso
 */
#include "serializacion.h"

static int serializar_fs_mkdir(const char *path, mode_t mode){
	void* paquete= serializar_paquete_para_crear_directorio(path,mode);
	void* resultado =enviar_paquete(paquete);
	free(paquete);
	int retorno;
	memcpy(&retorno,resultado,sizeof(int));
	free(resultado);
	return resultado;

}
void* serializar_paquete_para_crear_directorio(char* path,mode_t mode){

	int peso=0;
	int peso_path=string_length(path)+1;
	int peso_mode= sizeof(mode);
	peso=peso_path+peso_mode + 3* sizeof(int);
	int offset=0;
	int codigo_de_operacion= CREAR_DIRECTORIO;
	void*paquete=malloc(peso+sizeof(int));

	memcpy(paquete,&peso,sizeof(int));
	offset+=sizeof(int);
	memcpy(paquete+offset,&codigo_de_operacion,sizeof(int));
	offset+=sizeof(int);
	memcpy(paquete+offset,&peso_path,sizeof(int));
	offset+=sizeof(int);
	memcpy(paquete+offset,path,peso_path);
	offset+=peso_path;
	memcpy(paquete+offset,peso_mode,sizeof(int));
	offset+=sizeof(int);
	memcpy(paquete+offset,mode,peso_mode);

	return paquete;
}
void* enviar_paquete(void*paquete){

	int peso;
	memcpy(&peso,paquete,sizeof(int));
	send(socket_sac_server,paquete,peso + sizeof(int), 0);
	int alocador;
	return recibir_resultado(&alocador);

}


void* recibir_resultado(int* alocador){

	void* buffer;

		if(recv(socket_sac_server, alocador, 4, MSG_WAITALL)!=0){
			buffer = malloc(*alocador);
			recv(socket_sac_server, buffer, *alocador, MSG_WAITALL);
			return buffer;
		}else{
			*alocador = 0;
			return buffer;
		}
}
