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

t_log* log_servidor;
int PUERTO_ESCUCHA;
char* IP;

typedef struct{
	void* bloque;

}ptrGBloque;

typedef struct{
	char* identificador;
	int version;
	ptrGBloque* bloque_inicio_bitmap;
	int bloques_que_ocupa;
	void* relleno;

}estructura_header;

typedef struct{

	estructura_header* head;
	void* bitmap;// un bit por cada bloque
	t_list* tabla_de_nodos;// same as bitmap pero va a contener metadata del bloque
	int cantidad_maxima_de_nodos;// 1024
	t_list* bloques_de_datos;
	int cantidad_maxima_de_archivos;// (tamanio_file_system/BLOCK_SIZE/8)/BLOCK_SIZE redondeo para arriba
	int tamanio_file_system;//
	int BLOCK_SIZE;//4096 generalmente



}estructura_file_system;

//Semaforos
sem_t* mutex_log_servidor;

#endif /* GLOBALES_H_ */
