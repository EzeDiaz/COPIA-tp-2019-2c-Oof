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
bool borrar_archivo();
bool agregar_metadata_de_archivo();


//Funcionalidades con directorios
bool crear_directorio();
void* listar_directorio_y_archivos(char*, char*);
t_list* listar_metadata();
bool eliminar_directorio();

//funciones auxiliares
nodo_t* encontrar_en_tabla_de_nodos(char* );
void* paquetizar_metadata_de_directorio(t_list*);
nodo_t* buscar_nodo_libre();
ptrGBloque* obtener_puntero_padre(char*);
char* obtener_nombre_de_archivo(char* );
void leer_cada_archivo_y_borrar(char*);
void liberar_bloque(char*);
void liberar_eliminar_archivo(char*);
bool verificar_path_este_permitido(char*);
void crear_vector_de_punteros(ptrGBloque[],int);

#endif /* FUNCIONALIDADES_SAC_H_ */
