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
	creat(path,mode);
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
		remove(path);
		//falta meterlo en el bitmap y todas esas cosas TODO
		flag=true;
	}
	free(path);
	return flag;
}

bool agregar_metadata_de_archivo(char* path){
	bool flag= false;


	nodo_t* nodo_libre=buscar_nodo_libre();
	if(nodo_libre!=NULL){
		nodo_libre->estado=OCUPADO;
		nodo_libre->fecha_de_creacion= (long int) time(NULL) ;
		nodo_libre->fecha_de_modificacion=nodo_libre->fecha_de_creacion;
		nodo_libre->nombre_de_archivo=obtener_nombre_de_archivo(path);
		nodo_libre->tamanio_del_archivo=0;
		crear_vector_de_punteros(nodo_libre->punteros_indirectos_simples,1000);
		nodo_libre->puntero_padre=obtener_puntero_padre(path);
		flag=true;
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



nodo_t* encontrar_en_tabla_de_nodos(char* nombre_de_nodo){

	nodo_t* buscar_nodo_por_nombre(char* nombre){

		for(int i=0;i<1024;i++){

			if(!strcmp(tabla_de_nodos[i]->nombre_de_archivo,nombre))
				return tabla_de_nodos[i];

		}

		return NULL;
	}
	sem_wait(mutex_tabla_de_nodos);
	nodo_t* el_nodo_a_retornar = buscar_nodo_por_nombre(nombre_de_nodo);
	sem_post(mutex_tabla_de_nodos);
	return el_nodo_a_retornar;
}

void* paquetizar_metadata_de_directorio(t_list*lista){

	int peso=103;
	int peso_total=((peso)* lista->elements_count)+sizeof(int)+sizeof(int);
	void* retorno=malloc(peso_total);
	int offset=0;
	memcpy(retorno+offset,&peso,sizeof(int));
	offset+=sizeof(int);
	memcpy(retorno+offset,&lista->elements_count,sizeof(int));
	offset+=sizeof(int);

	void paquetizar(nodo_t* un_nodo){

		memcpy(retorno+offset ,&un_nodo->estado,1);//TODO esto puede romper
		offset+=1;
		memcpy(retorno+offset,&un_nodo->fecha_de_creacion,sizeof(long int));
		offset+=sizeof(long int);
		memcpy(retorno+offset,&un_nodo->fecha_de_modificacion,sizeof(long int));
		offset+=sizeof(long int);
		memcpy(retorno+offset,&un_nodo->puntero_padre,sizeof(int));
		offset+=sizeof(int);
		memcpy(retorno+offset,&un_nodo->tamanio_del_archivo,sizeof(int));
		offset+=sizeof(int);
		memcpy(retorno+offset,un_nodo->nombre_de_archivo,72);
		offset+=72;

	}
	list_iterate(lista, paquetizar);

	return retorno;
}


//FUNCIONES AUXILIARES


bool verificar_path_este_permitido(char*path){

	//puede mejorar TODO
	return string_contains(path, PUNTO_DE_MONTAJE);
}


nodo_t* buscar_nodo_libre(){

	int i=0;
	while(i<1024){

		if(tabla_de_nodos[i]->estado==BORRADO){
			return tabla_de_nodos[i];
		}
		i++;
	}
	return NULL;
}

void crear_vector_de_punteros(ptrGBloque array_de_bloques[], int n){

	for(int i=0;i<n;i++){
		for(int j=0; j<1024;j++){

			array_de_bloques[i].bloques[j].bloque=NULL;
		}
	}
}


ptrGBloque* obtener_puntero_padre(char* path){


	char** vector= string_split(path, "/");

	int i =0;
	while(vector[i]!=NULL){
		i++;
	}

	char* nombre_nodo_padre=vector[i-2];

	nodo_t* nodo_padre =encontrar_en_tabla_de_nodos(nombre_nodo_padre);
	return nodo_padre->punteros_indirectos_simples;


}


char* obtener_nombre_de_archivo(char* path){

	char** vector= string_split(path, "/");

	int i =0;
	while(vector[i]!=NULL){
		i++;
	}

	return vector[i-2];


}


void leer_cada_archivo_y_borrar(char* path){


	char* directorio = string_new();
	struct dirent *dir;
	DIR* directorio_tabla= opendir(directorio);
	if(directorio_tabla != NULL){

		while ((dir = readdir(directorio_tabla)) != NULL){
			if(!strcmp(dir->d_name,"..") || !strcmp(dir->d_name, ".")){

			}else{
				borrar_archivo(strcat(path,dir->d_name));
			}
		}
	}
}



void liberar_bloque(char* path){

//TODO


}
