/*
 * funcionalidades_SAC.h
 *
 *  Created on: 6 oct. 2019
 *      Author: utnso
 */

#ifndef FUNCIONALIDADES_SAC_H_
#define FUNCIONALIDADES_SAC_H_

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <commons/collections/list.h>
#include "globales.h"
#include "funciones_aux.h"

//Funcionalidades con archivos
int crear_archivo(char* path,mode_t mode);
int escribir_archivo(char* ,char* , size_t ,off_t);
void* leer_archivo(uint32_t , void *, size_t );
bool borrar_archivo(char*);
bool agregar_metadata_de_archivo();


//Funcionalidades con directorios
int crear_directorio(char* , mode_t );
void* listar_directorio_y_archivos(char*, char*);
t_list* listar_metadata();
bool eliminar_directorio();
t_list* lista_hijos_de(GFile*);
int calcular_peso_readdir(GFile* );
bool es_hijo_de(GFile*,GFile*);



#endif /* FUNCIONALIDADES_SAC_H_ */
