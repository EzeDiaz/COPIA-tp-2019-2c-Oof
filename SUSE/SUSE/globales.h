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

//SEMAFOROS

//Mutex
sem_t* semaforo_log_colas;
sem_t* semaforo_diccionario_de_procesos;
sem_t* semaforo_diccionario_procesos_x_queues;
sem_t* semaforo_diccionario_procesos_x_semaforo;
sem_t* mutex_log_servidor;
sem_t* semaforo_lista_procesos_finalizados;

//Contador
sem_t* procesos_x_grado_de_multiprogramacion;
sem_t* procesos_en_Ready;
sem_t* procesos_en_New;
sem_t* semaforo_lista_procesos_finalizados;

//LOGS
t_log* log_colas;
t_log* logger;

//DICCIONARIOS
t_dictionary* diccionario_de_procesos;
t_dictionary* diccionario_procesos_x_queues; // Va a ser una tupla de (PID;VectorDeColas)
t_dictionary* diccionario_de_procesos_x_semaforo;

//COLAS
t_queue* cola_new;
t_queue* cola_exit;
t_queue* cola_blocked;

//VARIABLES
float alpha;
int PUERTO_ESCUCHA;
char* IP;
int PUNTO_MONTAJE;

#endif /* GLOBALES_H_ */
