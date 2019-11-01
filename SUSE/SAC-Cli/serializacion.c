/*
 * serializacion.c
 *
 *  Created on: 30 oct. 2019
 *      Author: utnso
 */
#include "serializacion.h"
#include <fuse.h>
int serializar_fs_readdir(const char *path, void *buffer, fuse_fill_dir_t puntero_a_funcion, off_t offset, struct fuse_file_info *fi){
	void* paquete = serializar_paquete_para_leer_directorio(path, buffer ,puntero_a_funcion,offset,fi);
	void* resultado = enviar_paquete(paquete);
	free(paquete);
	int retorno;
	memcpy(&retorno,resultado,sizeof(int));
	free(resultado);
	return resultado;
}

int serializar_fs_mkdir(const char *path, mode_t mode){
	void* paquete = serializar_paquete_para_crear_directorio(path,mode);
	void* resultado = enviar_paquete(paquete);
	free(paquete);
	int retorno;
	memcpy(&retorno,resultado,sizeof(int));
	free(resultado);
	return resultado;

}

void* serializar_paquete_para_leer_directorio(const char *path, void *buffer, fuse_fill_dir_t puntero_a_funcion, off_t offset, struct fuse_file_info *fi){

	char* nombres_de_archivos=leer_nombres_de_archivos_y_directorios(buffer);
	int peso = 0;
	int peso_path = string_length(path)+1;
	int peso_buffer = string_length(nombres_de_archivos)+1;
	peso=peso_path+peso_buffer + 3* sizeof(int);
	int desplazamiento=0;
	int codigo_de_operacion = LISTAR_DIRECTORIO_Y_ARCHIVOS;
	void*paquete = malloc(peso+sizeof(int));


	memcpy(paquete,&peso,sizeof(int));
	desplazamiento+=sizeof(int);
	memcpy(paquete+desplazamiento,&codigo_de_operacion,sizeof(int));
	desplazamiento+=sizeof(int);
	memcpy(paquete+desplazamiento,&peso_path,sizeof(int));
	desplazamiento+=sizeof(int);
	memcpy(paquete+desplazamiento,path,peso_path);
	desplazamiento+=peso_path;
	memcpy(paquete+desplazamiento,&peso_buffer,sizeof(int));
	desplazamiento+=sizeof(int);
	memcpy(paquete+desplazamiento,nombres_de_archivos,peso_path);

	return paquete;
}

void* serializar_paquete_para_crear_directorio(char* path,mode_t mode){

	int peso=0;
	int peso_path=string_length(path)+1;
	int peso_mode= sizeof(mode);
	peso=peso_path+peso_mode + 2* sizeof(int);
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
	memcpy(paquete+offset,mode,sizeof(mode_t));

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

//Funciones Auxiliares

char* leer_nombres_de_archivos_y_directorios(void* buffer){

	return (char*) buffer;

}
