/*
 * funciones_aux.h
 *
 *  Created on: 20 nov. 2019
 *      Author: utnso
 */

#ifndef FUNCIONES_AUX_H_
#define FUNCIONES_AUX_H_

#include <stdbool.h>
#include <stdio.h>
#include "globales.h"
#include <string.h>
#include <errno.h>
#include <dirent.h>


nodo_t* encontrar_en_tabla_de_nodos(char* );
void* paquetizar_metadata_de_directorio(t_list*);
int buscar_nodo_libre();
ptrGBloque* obtener_puntero_padre(char*);
char* obtener_nombre_de_archivo(char* );
void leer_cada_archivo_y_borrar(char*);
void liberar_bloque(char*);
void liberar_eliminar_archivo(char*);
bool verificar_path_este_permitido(char*);
void crear_vector_de_punteros(ptrGBloque[],int);
void* encontrar_posicion_en_disco(int);


#endif /* FUNCIONES_AUX_H_ */
