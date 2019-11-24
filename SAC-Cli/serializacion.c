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

int serializar_fs_rmdir(char* path){
	void* paquete = serializar_paquete_para_eliminar_directorio(path);
	void* resultado = enviar_paquete(paquete);
	free(paquete);
	int retorno;
	memcpy(&retorno,resultado,sizeof(int));
	usar_y_liberar_resultado(resultado);
	return 0;


}

int serializar_fs_opendir(const char* path){

	void* paquete = serializar_paquete_para_abrir_directorio(path);
	void* resultado = enviar_paquete(paquete);
	free(paquete);
	int retorno;
	memcpy(&retorno,resultado,sizeof(int));
	usar_y_liberar_resultado(resultado);

	return 0;
}

int serializar_fs_create(const char *path, mode_t mode , struct fuse_file_info * fi){
	void* paquete = serializar_paquete_para_crear_archivo(path,mode);
	void* resultado = enviar_paquete(paquete);
	free(paquete);
	int retorno;
	memcpy(&retorno,resultado,sizeof(int));
	free(resultado);
	return resultado;

	return 0;
}

int serializar_fs_getattr(const char *path, struct stat *unos_stats){

	void* paquete = serializar_paquete_para_obtener_atributos(path);
	void* resultado = enviar_paquete(paquete);
	free(paquete);

	int offset=0;
	memcpy(&unos_stats->__pad1,resultado+offset,sizeof(unos_stats->__pad1));
	offset+=sizeof(unos_stats->__pad1);
	memcpy(&unos_stats->__pad2,resultado+offset,sizeof(unos_stats->__pad2));
	offset+=sizeof(unos_stats->__pad2);
	memcpy(&unos_stats->__st_ino,resultado+offset,sizeof(unos_stats->__st_ino));
	offset+=sizeof(unos_stats->__st_ino);
	memcpy(&unos_stats->st_atim,resultado+offset,sizeof(unos_stats->st_atim));
	offset+=sizeof(unos_stats->st_atim);
	memcpy(&unos_stats->st_blksize,resultado+offset,sizeof(unos_stats->st_blksize));
	offset+=sizeof(unos_stats->st_blksize);
	memcpy(&unos_stats->st_blocks,resultado+offset,sizeof(unos_stats->st_blocks));
	offset+=sizeof(unos_stats->st_blocks);
	memcpy(&unos_stats->st_ctim,resultado+offset,sizeof(unos_stats->st_ctim));
	offset+=sizeof(unos_stats->st_ctim);
	memcpy(&unos_stats->st_dev,resultado+offset,sizeof(unos_stats->st_dev));
	offset+=sizeof(unos_stats->st_dev);
	memcpy(&unos_stats->st_gid,resultado+offset,sizeof(unos_stats->st_gid));
	offset+=sizeof(unos_stats->st_gid);
	memcpy(&unos_stats->st_ino,resultado+offset,sizeof(unos_stats->st_ino));
	offset+=sizeof(unos_stats->st_ino);
	memcpy(&unos_stats->st_mode,resultado+offset,sizeof(unos_stats->st_mode));
	offset+=sizeof(unos_stats->st_mode);
	memcpy(&unos_stats->st_mtim,resultado+offset,sizeof(unos_stats->st_mtim));
	offset+=sizeof(unos_stats->st_mtim);
	memcpy(&unos_stats->st_nlink,resultado+offset,sizeof(unos_stats->st_nlink));
	offset+=sizeof(unos_stats->st_nlink);
	memcpy(&unos_stats->st_rdev,resultado+offset,sizeof(unos_stats->st_rdev));
	offset+=sizeof(unos_stats->st_rdev);
	memcpy(&unos_stats->st_size,resultado+offset,sizeof(unos_stats->st_size));
	offset+=sizeof(unos_stats->st_size);
	memcpy(&unos_stats->st_uid,resultado+offset,sizeof(unos_stats->st_uid));


	return 0;



}

int serializar_fs_readdir(const char *path, void *buffer, fuse_fill_dir_t puntero_a_funcion, off_t offset, struct fuse_file_info *fi){
	const struct stat *stbuf;
	stat(path,stbuf);
	puntero_a_funcion(buffer, path,  stbuf, offset);
	void* paquete = serializar_paquete_para_leer_directorio(path, buffer);
	void* resultado = enviar_paquete(paquete);
	free(paquete);
	int retorno;
	memcpy(&retorno,resultado,sizeof(int));
	usar_y_liberar_resultado(resultado);
	return 0;
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


int serializar_fs_open(const char *pathname,
		int flags, mode_t mode){
	void* paquete = serializar_paquete_para_abrir_archivo(pathname,flags,mode);
	void* resultado = enviar_paquete(paquete);
	free(paquete);
	int retorno;
	memcpy(&retorno,resultado,sizeof(int));
	free(resultado);
	return resultado;


}
int serializar_fs_read(int fd, void *buf, size_t count){
	void* paquete = serializar_paquete_para_leer_archivo(fd,buf,count);
	void* resultado = enviar_paquete(paquete);
	free(paquete);
	int retorno;
	memcpy(buf,resultado,count);
	free(resultado);
	return count;


}
int serializar_fs_write(int fd, const void *buf, size_t count){
	// TODO esto quizas esta medio mal, el prototipo de FUSE me dice algo y la syscall otra cosa
	void* paquete = serializar_paquete_para_escribir_archivo(fd,buf,count);
	void* resultado = enviar_paquete(paquete);
	free(paquete);
	int retorno;
	memcpy(&retorno,resultado,sizeof(int));
	free(resultado);
	return resultado;

}

void* serializar_paquete_para_crear_archivo(const char *path, mode_t mode){
	int peso=0;
	int peso_path=string_length(path)+1;
	int peso_mode= sizeof(mode);
	peso=peso_path+peso_mode + 2* sizeof(int);
	int offset=0;
	int codigo_de_operacion= CREAR_ARCHIVO;
	void*paquete=malloc(peso+sizeof(int));

	memcpy(paquete,&peso,sizeof(int));
	offset+=sizeof(int);
	memcpy(paquete+offset,&codigo_de_operacion,sizeof(int));
	offset+=sizeof(int);
	memcpy(paquete+offset,&peso_path,sizeof(int));
	offset+=sizeof(int);
	memcpy(paquete+offset,path,peso_path);
	offset+=peso_path;
	memcpy(paquete+offset,&mode,sizeof(mode_t));

	return paquete;
}



void* serializar_paquete_para_eliminar_directorio(char* path){

	int peso_path = string_length(path)+1;
	int peso=peso_path+ 2* sizeof(int);
	int desplazamiento=0;
	int codigo_de_operacion = ELIMINAR_DIRECTORIO;
	void*paquete = malloc(peso+sizeof(int));

	memcpy(paquete,&peso,sizeof(int));
	desplazamiento+=sizeof(int);
	memcpy(paquete+desplazamiento,&codigo_de_operacion,sizeof(int));
	desplazamiento+=sizeof(int);
	memcpy(paquete+desplazamiento,&peso_path,sizeof(int));
	desplazamiento+=sizeof(int);
	memcpy(paquete+desplazamiento,path,peso_path);

	return paquete;



}
void* serializar_paquete_para_leer_directorio(const char *path, void *buffer){

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

void* serializar_paquete_para_crear_directorio(const char* path,mode_t mode){

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
	memcpy(paquete+offset,&mode,sizeof(mode_t));

	return paquete;
}


void* serializar_paquete_para_leer_archivo(int fd, void *buf, size_t count){
	int peso=0;
	int peso_buffer=sizeof(&buf);//hay que checkear esto TODO
	int peso_count= sizeof(count);
	peso=peso_count+peso_buffer + 4* sizeof(int);
	int offset=0;
	int codigo_de_operacion= LEER_ARCHIVO;
	void*paquete=malloc(peso+sizeof(int));

	memcpy(paquete,&peso,sizeof(int));
	offset+=sizeof(int);
	memcpy(paquete+offset,&codigo_de_operacion,sizeof(int));
	offset+=sizeof(int);
	memcpy(paquete+offset,&fd,sizeof(int));
	offset+=sizeof(int);
	memcpy(paquete+offset,&peso_buffer,sizeof(int));
	offset+=sizeof(int);
	memcpy(paquete+offset,buf,peso_buffer);
	offset+=peso_buffer;
	memcpy(paquete+offset,&peso_count,sizeof(int));
	offset+=sizeof(int);
	memcpy(paquete+offset,&count,peso_count);
	offset+=peso_count;

	return paquete;



}
void* serializar_paquete_para_escribir_archivo(int fd,const void *buf, size_t count){

	int peso=0;
	int peso_buffer=sizeof(&buf);//hay que checkear esto TODO
	int peso_count= sizeof(count);
	peso=peso_count+peso_buffer + 4* sizeof(int);
	int offset=0;
	int codigo_de_operacion= ESCRIBIR_ARCHIVO;
	void*paquete=malloc(peso+sizeof(int));

	memcpy(paquete,&peso,sizeof(int));
	offset+=sizeof(int);
	memcpy(paquete+offset,&codigo_de_operacion,sizeof(int));
	offset+=sizeof(int);
	memcpy(paquete+offset,&fd,sizeof(int));
	offset+=sizeof(int);
	memcpy(paquete+offset,&peso_buffer,sizeof(int));
	offset+=sizeof(int);
	memcpy(paquete+offset,buf,peso_buffer);
	offset+=peso_buffer;
	memcpy(paquete+offset,&peso_count,sizeof(int));
	offset+=sizeof(int);
	memcpy(paquete+offset,&count,peso_count);
	offset+=peso_count;

	return paquete;





}
void* serializar_paquete_para_abrir_archivo(const char *pathname,
		int flags, mode_t mode){
	int peso=0;
	int peso_path=string_length(pathname)+1;
	int peso_mode= sizeof(mode);
	peso=peso_mode+peso_path + 4* sizeof(int);
	int offset=0;
	int codigo_de_operacion= ABRIR_ARCHIVO;
	void*paquete=malloc(peso+sizeof(int));

	memcpy(paquete,&peso,sizeof(int));
	offset+=sizeof(int);
	memcpy(paquete+offset,&codigo_de_operacion,sizeof(int));
	offset+=sizeof(int);
	memcpy(paquete+offset,&flags,sizeof(int));
	offset+=sizeof(int);
	memcpy(paquete+offset,&peso_mode,sizeof(int));
	offset+=sizeof(int);
	memcpy(paquete+offset,&mode,peso_mode);
	offset+=peso_mode;
	memcpy(paquete+offset,&peso_path,sizeof(int));
	offset+=sizeof(int);
	memcpy(paquete+offset,pathname,peso_path);
	offset+=peso_path;
	return paquete;






}

void* serializar_paquete_para_obtener_atributos(const char*path){

	int peso_path = string_length(path)+1;
		int peso=peso_path+ 2* sizeof(int);
		int desplazamiento=0;
		int codigo_de_operacion = GET_ATTRIBUTES;
		void*paquete = malloc(peso+sizeof(int));

		memcpy(paquete,&peso,sizeof(int));
		desplazamiento+=sizeof(int);
		memcpy(paquete+desplazamiento,&codigo_de_operacion,sizeof(int));
		desplazamiento+=sizeof(int);
		memcpy(paquete+desplazamiento,&peso_path,sizeof(int));
		desplazamiento+=sizeof(int);
		memcpy(paquete+desplazamiento,path,peso_path);

		return paquete;



}

void* serializar_paquete_para_abrir_directorio(const char* path){
	int peso_path = string_length(path)+1;
	int peso=peso_path+ 2* sizeof(int);
	int desplazamiento=0;
	int codigo_de_operacion = ABRIR_DIRECTORIO;
	void*paquete = malloc(peso+sizeof(int));

	memcpy(paquete,&peso,sizeof(int));
	desplazamiento+=sizeof(int);
	memcpy(paquete+desplazamiento,&codigo_de_operacion,sizeof(int));
	desplazamiento+=sizeof(int);
	memcpy(paquete+desplazamiento,&peso_path,sizeof(int));
	desplazamiento+=sizeof(int);
	memcpy(paquete+desplazamiento,path,peso_path);

	return paquete;



}


//RECIBIR Y ENVIAR A SAC_SERVER
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
