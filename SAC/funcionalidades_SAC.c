/*
 * funcionalidades_SAC.c
 *
 *  Created on: 6 oct. 2019
 *      Author: utnso
 */

#include "funcionalidades_SAC.h"
#include <errno.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <commons/string.h>
#include "globales.h"
#include <commons/collections/list.h>
#include<sys/types.h>
#include<sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>
#include <sys/statvfs.h>


//Funcionalidades con archivos
int crear_archivo(char* path,mode_t mode){
	int flag=0;
	char*fname[71];
	obtener_nombre_de_archivo(fname,path);
	GFile* nodo=encontrar_en_tabla_de_nodos(fname);
	if(nodo==NULL){
		realizar_mknod(path,0,0);

	}
	flag=abrir_archivo(path,0,0);



	return flag;
}

t_list* listar_metadata(){
	t_list* lista_de_retorno = list_create();
	return lista_de_retorno;
}


void* obtener_atributos(char* path){

	char fname[71];
	if(!strcmp(path,"/")){fname[0]='/';}else{
		obtener_nombre_de_archivo(fname,path);
	}
	GFile* nodo=encontrar_en_tabla_de_nodos(fname);
	int peso=sizeof(struct stat);
	void* resultado=malloc(peso+sizeof(int));
	int offset=0;
	struct stat *unos_stats=(struct stat*)malloc(sizeof(struct stat));


	unos_stats->st_atim.tv_nsec=time(NULL);
	unos_stats->st_atim.tv_sec=time(NULL)/1000;
	unos_stats->st_mtim.tv_sec=time(NULL)/1000;
	unos_stats->st_mtim.tv_nsec=nodo->m_date;
	unos_stats->st_gid=getgid();
	unos_stats->st_uid=getuid();
	unos_stats->st_mode=07;
	unos_stats->st_nlink=1 +(int) !strcmp(path,"/");
	unos_stats->st_size=nodo->file_size;


	memcpy(resultado+offset,&peso,sizeof(int));
	offset+=sizeof(int);



	memcpy(resultado+offset,&unos_stats->st_atim.tv_nsec,sizeof(unos_stats->st_atim));
	offset+=sizeof(unos_stats->st_atim);
	memcpy(resultado+offset,&unos_stats->st_atim.tv_sec,sizeof(unos_stats->st_atim));
	offset+=sizeof(unos_stats->st_atim);
	memcpy(resultado+offset,&unos_stats->st_gid,sizeof(unos_stats->st_gid));
	offset+=sizeof(unos_stats->st_gid);
	memcpy(resultado+offset,&unos_stats->st_mode,sizeof(unos_stats->st_mode));
	offset+=sizeof(unos_stats->st_mode);
	memcpy(resultado+offset,&unos_stats->st_mtim.tv_nsec,sizeof(unos_stats->st_mtim));
	offset+=sizeof(unos_stats->st_mtim);
	memcpy(resultado+offset,&unos_stats->st_mtim.tv_sec,sizeof(unos_stats->st_mtim));
	offset+=sizeof(unos_stats->st_mtim);
	memcpy(resultado+offset,&unos_stats->st_nlink,sizeof(unos_stats->st_nlink));
	offset+=sizeof(unos_stats->st_nlink);
	memcpy(resultado+offset,&unos_stats->st_size,sizeof(unos_stats->st_size));
	offset+=sizeof(unos_stats->st_size);
	memcpy(resultado+offset,&unos_stats->st_uid,sizeof(unos_stats->st_uid));


	return resultado;
}

int realizar_mknod(char* name,mode_t mode,dev_t dev){

	/*int current_node=0;
	 * GFile* node=list_get(tabla_de_nodos,current_node);
	while(node->state!=0 && current_node>1024){
		current_node++;
		node=list_get(tabla_de_nodos,current_node);
	}*/



	GFile* nodo_to_set= buscar_nodo_libre();

	if(nodo_to_set!=NULL){
		nodo_to_set->state=OCUPADO;
		nodo_to_set->c_date= (long int) time(NULL) ;
		nodo_to_set->m_date=nodo_to_set->c_date;
		obtener_nombre_de_archivo(nodo_to_set->fname,name);
		nodo_to_set->file_size=0;
		crear_vector_de_punteros(nodo_to_set->blk_indirect,1000);
		nodo_to_set->parent_dir_block=obtener_puntero_padre(name);

		return 0;
	}else{return -1;}


}

int escribir_archivo(char* direccion,char* cosas_a_escribir, size_t cantidad_a_escribir,off_t offset){
	int flag = 0;
	t_list* bloques_a_escribir=list_create();
	int bloque_a_escribir=0;

	while(cantidad_a_escribir>BLOCK_SIZE*bloques_a_escribir->elements_count && bloques_a_escribir>0){
		bloque_a_escribir=buscar_bloque_libre(BUSQUEDA_ARCHIVO);
		if(bloque_a_escribir>0)
			list_add(bloques_a_escribir,bloque_a_escribir);
	}
	if(bloques_a_escribir->elements_count>0){
		agregar_datos_de_escritura_a_tabla_de_nodo(direccion,bloques_a_escribir);
		int offset=0;
		void escribir_cosas_en_disco(int bloque){

			escribir_en_disco(cosas_a_escribir+offset,bloque_a_escribir);
			offset+=BLOCK_SIZE;
		}
		list_iterate(bloques_a_escribir,escribir_cosas_en_disco);
		flag=true;
	}


	list_destroy(bloques_a_escribir);
	return flag;
}

void* leer_archivo(uint32_t direccion, void *cosas_a_leer, size_t count){
	t_list* lista_de_retorno = list_create();
	cosas_a_leer=leer_en_disco((void*)direccion,count);
	return cosas_a_leer;
}

bool borrar_archivo(char* path){
	bool flag = false;
	if(verificar_path_este_permitido(path)){
		char fname[71];
		obtener_nombre_de_archivo(fname,path);
		GFile* nodo_a_borrar=encontrar_en_tabla_de_nodos(fname);
		nodo_a_borrar->state=BORRADO;
		borrar_del_bitmap(nodo_a_borrar->blk_indirect);
		flag=true;
	}
	free(path);
	return flag;
}

bool agregar_metadata_de_archivo(char* path){
	bool flag= false;

	GFile* nodo_libre=buscar_nodo_libre();
	if(nodo_libre!=NULL && obtener_puntero_padre(path)!=-1){
		nodo_libre->state=OCUPADO;
		nodo_libre->c_date= (long int) time(NULL) ;
		nodo_libre->m_date=nodo_libre->c_date;
		obtener_nombre_de_archivo(nodo_libre->fname,path);
		nodo_libre->file_size=0;
		crear_vector_de_punteros(nodo_libre->blk_indirect,1000);
		nodo_libre->parent_dir_block=obtener_puntero_padre(path);
		flag=true;
		escribir_en_disco(preparar_nodo_para_grabar(nodo_libre),buscar_bloque_libre(BUSQUEDA_NODO));
	}

	return flag;
}

int abrir_archivo(char* path, int flags, mode_t mode){

	char nombre_archivo[71];
	obtener_nombre_de_archivo(nombre_archivo,path);
	GFile* nodo=encontrar_en_tabla_de_nodos(nombre_archivo);

	if(nodo!=NULL){
		return nodo->blk_indirect;
	}else{
		return -1;
	}


}

//Funcionalidades con directorios
int crear_directorio(char* path, mode_t mode){

	int flag;
	char fname[71];
	obtener_nombre_de_archivo(fname,path);
	GFile* nodo=encontrar_en_tabla_de_nodos(fname);
	if(nodo==NULL){
		nodo=buscar_nodo_libre();

		int bloque_a_dar=buscar_bloque_libre();
		strcpy(nodo->fname,fname);
		nodo->m_date=time(NULL);
		nodo->c_date=time(NULL);
		nodo->file_size=0;
		nodo->state=DIRECTORIO;
		nodo->parent_dir_block=obtener_puntero_padre(path);
		crear_vector_de_punteros(nodo->blk_indirect,1000);
		flag=0;
	}else{
		if(nodo->state!=DIRECTORIO){
			strcpy(nodo->fname,fname);
			nodo->m_date=time(NULL);
			nodo->c_date=time(NULL);
			nodo->file_size=0;
			nodo->state=DIRECTORIO;
			nodo->parent_dir_block=obtener_puntero_padre(path);
			crear_vector_de_punteros(nodo->blk_indirect,1000);
			flag=0;
		}else{
			flag= -1;
		}
	}
	return flag;
}

void* listar_directorio_y_archivos(char*path, char*string_de_archivos){

	char nombre[71];
	obtener_nombre_de_archivo(nombre,path);
	GFile* nodo_padre=encontrar_en_tabla_de_nodos(nombre);
	if(nodo_padre->state==DIRECTORIO){
		t_list* nodos_hijos=buscar_todo_los_nodos_hijos(nodo_padre);

		int peso_de_buffer=0;

		for(int i=0;i<nodos_hijos->elements_count;i++){
			GFile* nodo_hijo=list_get(nodos_hijos,i);
			peso_de_buffer+=strlen(nodo_hijo->fname)+1;
			peso_de_buffer+=sizeof(int);

		}
		void*buffer_de_archivos=malloc(peso_de_buffer+sizeof(int));
		memcpy(buffer_de_archivos,&peso_de_buffer,sizeof(int));

		int offset=sizeof(int);
		for(int j=0;j<nodos_hijos->elements_count;j++){

			GFile* nodo_hijo=list_get(nodos_hijos,j);
			int peso_nombre=strlen(nodo_hijo->fname)+1;
			memcpy(buffer_de_archivos+offset,&peso_nombre,sizeof(int));
			offset+=sizeof(int);
			memcpy(buffer_de_archivos+offset,nodo_hijo->fname,peso_nombre);
			offset+=peso_nombre;

		}

		return buffer_de_archivos;
	}
	return NULL;

}

bool eliminar_directorio(char* path){
	bool flag = false;
	if(verificar_path_este_permitido(path)){
		leer_cada_archivo_y_borrar(path);
		borrar_archivo(path);

		//falta meterlo en el bitmap y todas esas cosas TODO
		flag=true;
	}
	free(path);
	return flag;
}



int abrir_directorio(char*path){

	char nombre_archivo[71];
	obtener_nombre_de_archivo(nombre_archivo,path);
	GFile* nodo=encontrar_en_tabla_de_nodos(nombre_archivo);
	return nodo->blk_indirect;



}

void* truncar(char* path,off_t offset){

	//TODO
	return NULL;

}

void* estadisticas_fs(char* path,struct statvfs* stats){
	statvfs(path, stats);
	stats->f_namemax=NAME_MAX;
	stats->f_blocks=BLOCK_SIZE;
	int bloques_ocupados=0;
	for(int i=0; i< CANT_MAX_BLOQUES;i++){
		if(bitarray_test_bit(bitarray,i)){

			bloques_ocupados++;
		}


	}
	stats->f_bfree=CANT_MAX_BLOQUES-bloques_ocupados;
	//no se de donde sacar los demas datos

	void* resultado=malloc(sizeof(struct statvfs)+sizeof(int));
	int offset=0;
	int peso= sizeof(struct statvfs);
	memcpy(resultado,&peso,sizeof(int));
	offset+=sizeof(int);
	memcpy(resultado+offset,&stats->__f_unused,sizeof(int));
	offset+=sizeof(int);
	memcpy(resultado+offset,&stats->f_bavail,sizeof(__fsblkcnt64_t));
	offset+=sizeof(__fsblkcnt64_t);
	memcpy(resultado+offset,&stats->f_bfree,sizeof(__fsblkcnt64_t));
	offset+=sizeof(__fsblkcnt64_t);
	memcpy(resultado+offset,&stats->f_blocks,sizeof(__fsblkcnt64_t));
	offset+=sizeof(__fsblkcnt64_t);
	memcpy(resultado+offset,&stats->f_bsize,sizeof(long int));
	offset+=sizeof(long int);
	memcpy(resultado+offset,&stats->f_ffree,sizeof(__fsfilcnt64_t));
	offset+=sizeof(__fsfilcnt64_t);
	memcpy(resultado+offset,&stats->f_files,sizeof(__fsfilcnt64_t));
	offset+=sizeof(__fsfilcnt64_t);
	memcpy(resultado+offset,&stats->f_frsize,sizeof(long int));
	offset+=sizeof(long int);
	memcpy(resultado+offset,&stats->f_namemax,sizeof(long int));
	offset+=sizeof(long int);
	memcpy(resultado+offset,&stats->__f_unused,sizeof(int));
	offset+=sizeof(int);





}
void* acceder(char* path,int flags){
	return NULL; //TODO

}

