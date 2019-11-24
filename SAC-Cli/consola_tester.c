/*
 * consola_tester.c
 *
 *  Created on: 2 nov. 2019
 *      Author: utnso
 */


#define _FILE_OFFSET_BITS 64

#include <readline/readline.h>
#include <stdio.h>
#include <readline/history.h>
#include <commons/string.h>
#include <commons/collections/list.h>
#include "consola_tester.h"




int consola(){

	char *linea = " ";
	char* resultado;
	while(linea != '\0'){
		printf("Ingrese un comando...\n");
		linea = readline(">");

		if(!strncmp(linea, "create",5)){

		}

		if(!strncmp(linea, "mkdir",5)){
			char* path=(char*)malloc(71);
			mode_t mode;
			if(leer_parametros_mkdir(path,&mode, linea)){
				if(serializar_fs_mkdir(path,mode)){
					printf("Creamos la tabla :D \n");
				} else {
					printf("La tabla ya existe \n");
				}
				free(path);
			}else{
				printf("No pudimos leer bien los paramatros => no creamos nada\n");
			}

			//leer parametros y llamar a crearLissandra

		}else{
			if(!strncmp(linea, "readdir",7)){
				const char *path;
				void * buffer;
				fuse_fill_dir_t puntero_a_funcion;
				struct fuse_file_info* fi;
				off_t offset;
				if(leer_parametros_readdir(path, buffer,puntero_a_funcion,&offset,fi, linea)){
							if(serializar_fs_readdir(path, buffer,puntero_a_funcion,offset,fi)){
								printf("preparate para el readdir \n");
							} else {
								printf("fallo el readdir \n");
							}
							free(path);
							free(buffer);
							free(fi);
						}else{
							printf("No pudimos leer bien los paramatros => no creamos nada\n");
						}
							}else{

								printf("Comando invalido, volver a intentar :(\n");
							}

						}
					}
	free(linea);
	printf("Se cerro la consola de testeo. GG WP!\n");
	return 0;
}

//ESTO YA SON FUNCIONES AUXILIARES NO SE SI LAS QUIEREN DELEGAR EN ALGUN OTRO .C
int leer_parametros_mkdir(char* path,mode_t* mode,char* linea){

	char** parametros = string_split(linea, " ");
	if(parametros[1]==NULL){

		printf("Faltan parametros \n");
		return 0;

	}else{
		strcpy(path ,parametros[1]);
		*mode= atoi(parametros[2]);
		return 1;
	}}

int leer_parametros_readdir(char* path, void * buffer,fuse_fill_dir_t puntero_a_funcion,unsigned int* offset,struct fuse_file_info *fi,char* linea){

	char** parametros = string_split(linea, " ");
	if(parametros[1]==NULL){//TODO

		}else{

	}
	return 1;
}






