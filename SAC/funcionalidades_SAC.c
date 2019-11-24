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


//Funcionalidades con archivos
bool crear_archivo(char* path,mode_t mode){
	bool flag = false;
	flag=agregar_metadata_de_archivo(path);
	return flag;
}

t_list* listar_metadata(){
	t_list* lista_de_retorno = list_create();
	return lista_de_retorno;
}

bool escribir_archivo(uint32_t direccion,const void *cosas_a_escribir, size_t cantidad_a_escribir){
	bool flag = false;
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
	if(nodo_libre!=NULL){
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
	if(verificar_path_este_permitido(path)){
		char nombre_archivo[71];
		obtener_nombre_de_archivo(nombre_archivo,path);
		GFile* nodo=encontrar_en_tabla_de_nodos(nombre_archivo);
		return nodo->blk_indirect;
	}


	return -1;
}

//Funcionalidades con directorios
bool crear_directorio(char* path, mode_t mode){
	bool flag = false;
	if(verificar_path_este_permitido(path)){

		flag=agregar_metadata_de_archivo(path);
	}

	return flag;
}

void* listar_directorio_y_archivos(char*path, char*string_de_archivos){

	if(verificar_path_este_permitido(path)){

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





