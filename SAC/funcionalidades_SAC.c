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
//Funcionalidades con archivos
bool crear_archivo(){
	bool flag = false;
	//falta meterlo en el bitmap y todas esas cosas TODO
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

bool borrar_archivo(){
	bool flag = false;
	//sirve? no es lo mismo que el rmdir?
	return flag;
}

bool agregar_metadata_de_archivo(){
	bool flag= false;
//TODO

	return flag;


}

//Funcionalidades con directorios
bool crear_directorio(char* path, mode_t mode){
	bool flag = false;
	mkdir(path, mode);
	//falta meterlo en el bitmap y todas esas cosas TODO
	flag=errno!=EEXIST;
	return flag;
}

void* listar_directorio_y_archivos(char*path, char*string_de_archivos){

	t_list* lista_de_retorno = list_create();
	char* directorio = string_new();
	string_append(&directorio, PUNTO_DE_MONTAJE);
	string_append(&directorio, path);
	struct dirent *dir;

	//opendir
	//readdir
	//por cada leida pregunto el d_name esta en el string_de_archivo
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

bool eliminar_directorio(char* path){
	bool flag = false;
	if(verificar_path_este_permitido(path)){
	remove(path);
	//falta meterlo en el bitmap y todas esas cosas TODO
	flag=true;
	}
	free(path);
	return flag;
}



nodo_t* encontrar_en_tabla_de_nodos(char* nombre_de_nodo){

	bool buscar_nodo_por_nombre(nodo_t* un_nodo){

		return !strcmp(un_nodo->nombre_de_archivo,nombre_de_nodo);
	}
	sem_wait(mutex_tabla_de_nodos);
	nodo_t* el_nodo_a_retornar = list_find(tabla_de_nodos,buscar_nodo_por_nombre);
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
		memcpy(retorno+offset,un_nodo->estado,1);//TODO esto puede romper
		offset+=1;
		memcpy(retorno+offset,&un_nodo->fecha_de_creacion,sizeof(long int));
		offset+=sizeof(long int);
		memcpy(retorno+offset,&un_nodo->fecha_de_modificacion,sizeof(long int));
		offset+=sizeof(long int);
		memcpy(retorno+offset,&un_nodo->puntero_padre,sizeof(int));
		offset+=sizeof(int);
		memcpy(retorno+offset,un_nodo->tamanio_del_archivo,sizeof(int));
		offset+=sizeof(int);
		memcpy(retorno+offset,un_nodo->nombre_de_archivo,72);
		offset+=72;

	}
	list_iterate(lista, paquetizar);

	return retorno;
}


//FUNCIONES AUXILIARES


int verificar_path_este_permitido(char*path){

	//puede mejorar TODO
	return string_contains(path, PUNTO_DE_MONTAJE);

}
