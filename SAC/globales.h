/*
 * globales.h
 *
 *  Created on: 6 oct. 2019
 *      Author: utnso
 */

#ifndef GLOBALES_H_
#define GLOBALES_H_

#include <commons/collections/list.h>
#include <commons/log.h>
#include <semaphore.h>
#include <stdint.h>
#include <commons/bitarray.h>
#include "sac.h"



//TADS
typedef struct{
	int numero_bloque;
	void* bloque;

}Bloque_datos;



typedef struct{

	GHeader* head;
	void* bitmap;// un bit por cada bloque
	t_list* tabla_de_nodos;// same as bitmap pero va a contener metadata del bloque
	int cantidad_maxima_de_nodos;// 1024
	t_list* bloques_de_datos;
	int cantidad_maxima_de_archivos;// (tamanio_file_system/BLOCK_SIZE/8)/BLOCK_SIZE redondeo para arriba
	int tamanio_file_system;//
	int BLOCK_SIZE;//4096 generalmente



}estructura_file_system;

enum {

	BORRADO=0,
	OCUPADO=1,
	DIRECTORIO=2

}estado_nodo;

enum{

	BUSQUEDA_NODO=0,
	BUSQUEDA_ARCHIVO=1,

}tipos_de_busqueda;

//Semaforos
sem_t* mutex_log_servidor;
sem_t*mutex_tabla_de_nodos;


//variables globales
int PUERTO_ESCUCHA;
char* IP;
char* PUNTO_DE_MONTAJE;
GFile* tabla_de_nodos[1024];
int BLOCK_SIZE;
int CANT_MAX_BLOQUES;
char* NOMBRE_DEL_DISCO;
t_bitarray* bitarray;
void* primer_bloque;
int cantidad_de_bloques_reservados;
long tamanio_disco;



#endif /* GLOBALES_H_ */
