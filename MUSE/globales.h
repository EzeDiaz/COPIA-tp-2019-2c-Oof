/*
 * globales.h
 *
 *  Created on: 95 oct. 2019
 *      Author: utnso
 */

#ifndef GLOBALES_H_
#define GLOBALES_H_

#include <commons/bitarray.h>

void* mp_pointer;	//puntero a la memoria principal
t_log* logger;	//hace falta que mencione esto?
t_config* config;	//lo mismo que el de arriba pero para config
t_list* all_address_space;	//tabla de segmentacion implementado a traves de una t_list*
uint8_t* memory_controller;	//tal vez despues lo usamos
t_list* client_list;

pthread_mutex_t mp_semaphore;
pthread_mutex_t logger_semaphore;
pthread_mutex_t segmentation_table_semaphore;
pthread_mutex_t memory_controller_semaphore;

int listen_port;
int memory_size;
int page_size;
int swap_size;
t_bitarray* bitmap;

int memory_left;

#endif /* GLOBALES_H_ */
