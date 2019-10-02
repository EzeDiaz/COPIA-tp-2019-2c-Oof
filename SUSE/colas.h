/*
 * colas.h
 *
 *  Created on: 1 oct. 2019
 *      Author: utnso
 */

#ifndef COLAS_H_
#define COLAS_H_

#include <commons/collections/list.h>
#include <commons/collections/dictionary.h>
#include <stdlib.h>
#include <commons/log.h>
#include "TADs.h"
#include <commons/config.h>
#include <semaphore.h>

//////////////////INICIALIZAR EN EL MAIN\\\\\\\\\\\\\\\\\\\\\\\ TODO

//Semaforos
//MUTEX
sem_t* semaforo_log_colas;
sem_t* semaforo_diccionario_de_procesos;

//CONTADORES
sem_t* procesos_x_grado_de_multiprogramacion;

//Logs
t_log* log_colas;

//Diccionarios
t_dictionary* diccionario_de_procesos;

//queues
t_queue* cola_new;


#endif /* COLAS_H_ */
