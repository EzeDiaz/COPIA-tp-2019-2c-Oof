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
	inicializar_nombre(fname);
	if(!strncmp(path,"/",strlen(path))){
		fname[0]='/';
	}else{
		obtener_nombre_de_archivo(fname,path);
	}
	GFile* nodo=encontrar_en_tabla_de_nodos(fname);
	int offset=0;
	struct stat *unos_stats=(struct stat*)malloc(sizeof(struct stat));
	stat(PUNTO_DE_MONTAJE,unos_stats);

	void* resultado;

	if(nodo!=NULL){
		int peso=sizeof(unos_stats->st_atim.tv_nsec)+sizeof(unos_stats->st_atim.tv_sec)
																				+sizeof(unos_stats->st_gid)+sizeof(unos_stats->st_mode)+
																				sizeof(unos_stats->st_mtim.tv_sec)+sizeof(unos_stats->st_mtim.tv_nsec)
																				+sizeof(unos_stats->st_nlink)+sizeof(unos_stats->st_size)
																				+sizeof(unos_stats->st_uid)+sizeof(unos_stats->__pad1)
																				+sizeof(unos_stats->__pad2)+sizeof(unos_stats->st_ino)
																				+sizeof(unos_stats->st_blksize)+sizeof(unos_stats->st_blocks)
																				+sizeof(unos_stats->st_rdev)+sizeof(unos_stats->st_dev);
		resultado=malloc(peso+sizeof(int));
		if(nodo->state==DIRECTORIO){

			unos_stats->st_mode = S_IFDIR | 0755;
			unos_stats->st_nlink = 2;
		}else{

			unos_stats->st_mode = S_IFREG | 0444;
			unos_stats->st_nlink = 1;

		}
		unos_stats->st_atim.tv_nsec=time(NULL);
		unos_stats->st_atim.tv_sec=time(NULL)/1000;
		unos_stats->st_mtim.tv_sec=time(NULL)/1000;
		unos_stats->st_mtim.tv_nsec=nodo->m_date;
		unos_stats->st_gid=getgid();
		unos_stats->st_uid=getuid();
		unos_stats->st_size=nodo->file_size;
		memcpy(resultado+offset,&peso,sizeof(int));
		offset+=sizeof(int);

		memcpy(resultado+offset,&unos_stats->st_atim.tv_nsec,sizeof(unos_stats->st_atim.tv_nsec));
		offset+=sizeof(unos_stats->st_atim.tv_nsec);
		memcpy(resultado+offset,&unos_stats->st_atim.tv_sec,sizeof(unos_stats->st_atim.tv_sec));
		offset+=sizeof(unos_stats->st_atim.tv_sec);
		memcpy(resultado+offset,&unos_stats->st_gid,sizeof(unos_stats->st_gid));
		offset+=sizeof(unos_stats->st_gid);
		memcpy(resultado+offset,&unos_stats->st_mode,sizeof(unos_stats->st_mode));
		offset+=sizeof(unos_stats->st_mode);
		memcpy(resultado+offset,&unos_stats->st_mtim.tv_nsec,sizeof(unos_stats->st_mtim));
		offset+=sizeof(unos_stats->st_mtim.tv_nsec);
		memcpy(resultado+offset,&unos_stats->st_mtim.tv_sec,sizeof(unos_stats->st_mtim));
		offset+=sizeof(unos_stats->st_mtim.tv_sec);
		memcpy(resultado+offset,&unos_stats->st_nlink,sizeof(unos_stats->st_nlink));
		offset+=sizeof(unos_stats->st_nlink);
		memcpy(resultado+offset,&unos_stats->st_size,sizeof(unos_stats->st_size));
		offset+=sizeof(unos_stats->st_size);
		memcpy(resultado+offset,&unos_stats->st_uid,sizeof(unos_stats->st_uid));
		offset+=sizeof(unos_stats->st_uid);

		memcpy(resultado+offset,&unos_stats->__pad1,sizeof(unos_stats->__pad1));
		offset+=sizeof(unos_stats->__pad1);
		memcpy(resultado+offset,&unos_stats->__pad2,sizeof(unos_stats->__pad2));
		offset+=sizeof(unos_stats->__pad2);
		memcpy(resultado+offset,&unos_stats->st_ino,sizeof(unos_stats->st_ino));
		offset+=sizeof(unos_stats->st_ino);
		memcpy(resultado+offset,&unos_stats->st_blksize,sizeof(unos_stats->st_blksize));
		offset+=sizeof(unos_stats->st_blksize);
		memcpy(resultado+offset,&unos_stats->st_blocks,sizeof(unos_stats->st_blocks));
		offset+=sizeof(unos_stats->st_blocks);
		memcpy(resultado+offset,&unos_stats->st_rdev,sizeof(unos_stats->st_rdev));
		offset+=sizeof(unos_stats->st_rdev);
		memcpy(resultado+offset,&unos_stats->st_dev,sizeof(unos_stats->st_dev));



		free(unos_stats);



	}else{

		int res = -ENOENT;
		int peso_error=sizeof(ENOENT);
		resultado=malloc(sizeof(int)+sizeof(ENONET));
		memcpy(resultado,&peso_error,sizeof(int));
		memcpy(resultado+sizeof(int),&res,sizeof(ENONET));

	}
	return resultado;

}

int realizar_mknod(char* name,mode_t mode,dev_t dev){




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
	inicializar_nombre(fname);
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
			nodo->state=OCUPADO;
			nodo->parent_dir_block=obtener_puntero_padre(path);
			crear_vector_de_punteros(nodo->blk_indirect,1000);
			flag=0;
		}else{
			flag= -1;
		}
	}
	return flag;
}

int calcular_peso_readdir(GFile* nodo){
	int peso=0;

	if((strncmp(nodo->fname,tabla_de_nodos[0]->fname,strlen(nodo->fname)))){

		peso+=strlen(".")+1;
		peso+=sizeof(int);
		peso+=strlen("..")+1;
		peso+=sizeof(int);
	}else{
		peso+= (strlen("file54")+1);
		peso+= sizeof(int);
		peso+= (strlen("file349")+1);
		peso+= sizeof(int);


	}

	t_list* lista_hijos=lista_hijos_de(nodo);

	void sumar_peso(GFile* nodo_hijo){
		peso+=strlen(nodo->fname)+1;
		peso+=sizeof(int);
	}

	list_iterate(lista_hijos,sumar_peso);

	return peso;

}

t_list* lista_hijos_de(GFile*nodo){

	t_list* lista_retorno=list_create();

	for(int i=0;i<GFILEBYTABLE;i++){
		if(es_hijo_de(nodo,tabla_de_nodos[i])){
			list_add(lista_retorno,tabla_de_nodos[i]);
		}
	}

	return lista_retorno;

}

bool es_hijo_de(GFile*nodo_padre,GFile*posible_hijo){


	return posible_hijo->parent_dir_block==calcula_dir_padre(nodo_padre);//todo
}



void* listar_directorio_y_archivos(char*path, char*string_de_archivos){


	char nombre[71];
	inicializar_nombre(nombre);
	obtener_nombre_de_archivo(nombre,path);
	GFile* nodo=encontrar_en_tabla_de_nodos(nombre);
	int peso=calcular_peso_readdir(nodo);
	peso+=sizeof(int);
	void* retorno=malloc(peso+sizeof(int));
	int offset=0;
	int longitud_siguiente=0;

	memcpy(retorno,&peso,sizeof(int));
	offset+=sizeof(int);

	if(nodo->state==DIRECTORIO){



		t_list* nodos_hijos=buscar_todo_los_nodos_hijos(nodo);
		int cantidad_de_dirs=nodos_hijos->elements_count+2;
		memcpy(retorno+offset,&cantidad_de_dirs,sizeof(int));
		offset+=sizeof(int);

		if(!strncmp(nodo->fname,tabla_de_nodos[0]->fname,strlen(path))){
			longitud_siguiente=strlen("file54")+1;
			memcpy(retorno+offset,&longitud_siguiente,sizeof(int));
			offset+=sizeof(int);
			memcpy(retorno+offset, "file54" ,longitud_siguiente);
			offset+=longitud_siguiente;

			int longitud_siguiente2=strlen("file349")+1;
			memcpy(retorno+offset,&longitud_siguiente2,sizeof(int));
			offset+=sizeof(int);
			memcpy(retorno+offset, "file349" ,longitud_siguiente2);
			offset+=longitud_siguiente2;
		}else{

			longitud_siguiente=strlen(".")+1;
			memcpy(retorno+offset,&longitud_siguiente,sizeof(int));
			offset+=sizeof(int);
			memcpy(retorno+offset, "." ,longitud_siguiente);
			offset+=longitud_siguiente;
			longitud_siguiente=strlen("..")+1;
			memcpy(retorno+offset,&longitud_siguiente,sizeof(int));
			offset+=sizeof(int);
			memcpy(retorno+offset, ".." ,longitud_siguiente);
			offset+=longitud_siguiente;

		}


		void meter_en_puntero(GFile* nodo_hijo){
			longitud_siguiente=strlen(nodo_hijo->fname)+1;
			memcpy(retorno+offset,&longitud_siguiente,sizeof(int));
			offset+=sizeof(int);
			memcpy(retorno+offset, nodo_hijo->fname ,longitud_siguiente);
			offset+=longitud_siguiente;

		}


		list_iterate(nodos_hijos,meter_en_puntero);


	}
	return retorno;
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
	if(nodo==NULL)
		return -1;
	return buscar_posicion_en_tabla_de_nodos(nodo);



}

int buscar_posicion_en_tabla_de_nodos(GFile*nodo){

	int i=0;
	GFile*nodo_comparar=tabla_de_nodos[0];
	while(strcmp(nodo->fname,nodo_comparar->fname)){
		i++;
	}

	return i;
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

	return resultado;



}
void* acceder(char* path,int flags){
	return NULL; //TODO

}

void inicializar_nombre(char fname[]){

	for(int i=0;i<GFILENAMELENGTH;i++){

		fname[i]='\0';

	}


}

