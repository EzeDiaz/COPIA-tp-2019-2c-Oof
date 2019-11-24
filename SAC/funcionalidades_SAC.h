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

//Funcionalidades con archivos
bool crear_archivo();
bool escribir_archivo();
t_list* leer_archivo();
bool borrar_archivo(char*);
bool agregar_metadata_de_archivo();


//Funcionalidades con directorios
bool crear_directorio();
void* listar_directorio_y_archivos(char*, char*);
t_list* listar_metadata();
bool eliminar_directorio();



#endif /* FUNCIONALIDADES_SAC_H_ */
