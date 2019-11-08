/*
 * serializacion.c
 *
 *  Created on: 30 oct. 2019
 *      Author: utnso
 */
#include "serializacion.h"
#include <fuse.h>
#include "globales.h"
#include <commons/collections/list.h>

int serializar_fs_readdir(const char *path, void *buffer, fuse_fill_dir_t puntero_a_funcion, off_t offset, struct fuse_file_info *fi){
	void* paquete = serializar_paquete_para_leer_directorio(path, buffer ,puntero_a_funcion,offset,fi);
	void* resultado = enviar_paquete(paquete);
	free(paquete);
	int retorno;
	memcpy(&retorno,resultado,sizeof(int));
	usar_y_liberar_resultado(resultado);
	return 1;
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

void usar_y_liberar_resultado(void* resultado){
	int cantidad_de_archivos;
	int offset=0;
	t_list * lista_de_nodos=list_create();
	memcpy(&cantidad_de_archivos,resultado+offset,sizeof(int));
	offset+=sizeof(int);
	memcpy(&cantidad_de_archivos,resultado+offset,sizeof(int));
	offset+=sizeof(int);
	for(int i=0; i< cantidad_de_archivos;i++){

		nodo_t* un_nodo= malloc(sizeof(nodo_t));

		memcpy(un_nodo->estado,resultado+offset,1);//TODO esto puede romper
		offset+=1;
		memcpy(&un_nodo->fecha_de_creacion,resultado+offset,sizeof(long int));
		offset+=sizeof(long int);
		memcpy(&un_nodo->fecha_de_modificacion,resultado+offset,sizeof(long int));
		offset+=sizeof(long int);
		memcpy(&un_nodo->puntero_padre,resultado+offset,sizeof(int));
		offset+=sizeof(int);
		memcpy(un_nodo->tamanio_del_archivo,resultado+offset,sizeof(int));
		offset+=sizeof(int);
		memcpy(un_nodo->nombre_de_archivo,resultado+offset,72);
		offset+=72;

		list_add(lista_de_nodos,un_nodo);

	}

	printear_lista_de_nodos(lista_de_nodos);

}


void printear_lista_de_nodos(lista_de_nodos){

	void printear_nodo(nodo_t* un_nodo){
		printf("Miren... un nodo! \n");
		printf("Su estado es: %s \n",un_nodo->estado);
		printf("Su puntero padre es: %d \n",un_nodo->puntero_padre);
		printf("Su fecha de creacion es: %d \n",un_nodo->fecha_de_creacion);
		printf("Su fecha de modificacion es: %d \n",un_nodo->fecha_de_modificacion);
		printf("Su nombre es: %s \n",un_nodo->nombre_de_archivo);
		printf("Su tamanio es: %d \n",un_nodo->tamanio_del_archivo);
	}
	list_iterate(lista_de_nodos, printear_nodo);

}
