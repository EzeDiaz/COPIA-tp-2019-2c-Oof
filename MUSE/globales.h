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
t_log* logger;
t_config* config;
t_list* all_address_spaces;
uint8_t* memory_controller;	//tal vez despues lo usamos (will we? esta el bitmap)
t_list* client_list;
t_list* mapped_files;
char* swap_file; //Archivo utilizado para swapear de memoria a disco y viceversa. Abrir con mmap
t_bitarray* bitmap; //Estructura de control para los frames
int clock_pointer; //Desde que frame arranca la proxima iteracion del clock
pageFrame* clock_table[]; //Array donde se guardan las paginas
//Necesitamos una estructura de control para el archivo de swap?

pthread_mutex_t mp_semaphore;
pthread_mutex_t logger_semaphore;
pthread_mutex_t segmentation_table_semaphore;
pthread_mutex_t memory_controller_semaphore;

int listen_port;
int memory_size;
int page_size;
int swap_size;

int memory_left;

#endif /* GLOBALES_H_ */
