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

//Funcionalidades con archivos
bool crear_archivo(){
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

bool borrar_archivo(){
	bool flag = false;
	return flag;
}

bool agregar_metadata_de_archivo(){
	bool flag= false;


	return flag;


}

//Funcionalidades con directorios
bool crear_directorio(char* path, mode_t mode){
	bool flag = false;
	mkdir(path, mode);
	if(errno!=EEXIST)
		flag=true;
	return flag;
}

t_list* listar_directorio_y_archivos(){
	t_list* lista_de_retorno = list_create();
	return lista_de_retorno;
}

bool eliminar_directorio(){
	bool flag = false;
	return flag;
}
