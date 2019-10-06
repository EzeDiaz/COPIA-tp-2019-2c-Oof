/*
 * globales.h
 *
 *  Created on: 1 oct. 2019
 *      Author: utnso
 */

#ifndef GLOBALES_H_
#define GLOBALES_H_
#include <commons/collections/queue.h>
#include <commons/collections/list.h>
#include <commons/collections/dictionary.h>
#include <stdlib.h>
#include <commons/log.h>

#include <commons/config.h>
#include <semaphore.h>
#include <stdio.h>
//////////////////PARA COSAS QUE SON COMUNES EN TODO EL PROYECTO\\\\\\\\\\\\\\\\\\
//////////////////INICIALIZAR EN EL MAIN\\\\\\\\\\\\\\\\\\\\\\\ TODO

//Semaforos
//MUTEX
sem_t* semaforo_log_colas;
sem_t* semaforo_diccionario_de_procesos;
sem_t* semaforo_diccionario_procesos_x_queues;
sem_t* semaforo_diccionario_procesos_x_semaforo;

//CONTADORES
sem_t* procesos_x_grado_de_multiprogramacion;
sem_t* procesos_en_Ready;
sem_t* procesos_en_New;
sem_t* semaforo_lista_procesos_finalizados;
//Logs
t_log* log_colas;

//Diccionarios
t_dictionary* diccionario_de_procesos;
t_dictionary* diccionario_procesos_x_queues; // Va a ser una tupla de (PID;VectorDeColas)
t_dictionary* diccionario_de_procesos_x_semaforo;

//Queues
t_queue* cola_new;

float alpha;

#endif /* GLOBALES_H_ */
