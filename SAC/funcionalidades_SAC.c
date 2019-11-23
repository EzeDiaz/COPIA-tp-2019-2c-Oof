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
	flag=agregar_metadata_de_archivo();
	return flag;
}

t_list* listar_metadata(){
	t_list* lista_de_retorno = list_create();
	return lista_de_retorno;
}

bool escribir_archivo(){
	bool flag = false;
	return flag;
}

t_list* leer_archivo(){
	t_list* lista_de_retorno = list_create();
	return lista_de_retorno;
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
	if(verificar_path_este_permitido(path))
		return open(path, flags,mode);

	return -1;
}

//Funcionalidades con directorios
bool crear_directorio(char* path, mode_t mode){
	bool flag = false;
	if(verificar_path_este_permitido(path)){
		mkdir(path, mode);
		flag=errno!=EEXIST;
	}
	//falta meterlo en el bitmap y todas esas cosas TODO

	return flag;
}

void* listar_directorio_y_archivos(char*path, char*string_de_archivos){

	if(verificar_path_este_permitido(path)){
		t_list* lista_de_retorno = list_create();
		char* directorio = string_new();
		string_append(&directorio, path);
		struct dirent *dir;

		//TODO

		DIR* directorio_tabla= opendir(directorio);
		if(directorio_tabla != NULL){

			while ((dir = readdir(directorio_tabla)) != NULL){
				if(!strcmp(dir->d_name,"..") || !strcmp(dir->d_name, ".")){

				}else{
					if(string_contains(string_de_archivos,dir->d_name))
						list_add(lista_de_retorno,encontrar_en_tabla_de_nodos(dir->d_name));
				}
			}
		}


		return paquetizar_metadata_de_directorio(lista_de_retorno);
	}
	return NULL;
}

bool eliminar_directorio(char* path){
	bool flag = false;
	if(verificar_path_este_permitido(path)){
		leer_cada_archivo_y_borrar(path);
		liberar_bloque(path);
		remove(path);
		//falta meterlo en el bitmap y todas esas cosas TODO
		flag=true;
	}
	free(path);
	return flag;
}





