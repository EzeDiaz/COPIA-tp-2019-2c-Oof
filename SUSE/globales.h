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


//SEMAFOROS

//Mutex
sem_t* semaforo_log_colas;
sem_t* semaforo_diccionario_de_procesos;
sem_t* semaforo_diccionario_procesos_x_queues;
sem_t* semaforo_diccionario_procesos_x_semaforo;
sem_t* mutex_log_servidor;
sem_t* semaforo_lista_procesos_finalizados;
sem_t* semaforo_estado_blocked;
sem_t* semaforo_diccionario_por_semaforo;

//Contador
sem_t* grado_de_multiprogramacion_contador;
sem_t* procesos_en_New;
sem_t* semaforo_lista_procesos_finalizados;

//LOGS
t_log* log_colas;
t_log* logger;
t_log* log_metricas_sistema;
t_log* log_metricas_programa;
t_log* log_metricas_hilo;
t_log* logger_de_deserializacion;

//DICCIONARIOS
t_dictionary* diccionario_de_procesos;
t_dictionary* diccionario_procesos_x_queues; // Va a ser una tupla de (PID;VectorDeColas)
t_dictionary* diccionario_de_procesos_x_semaforo;
t_dictionary* diccionario_bloqueados_por_semafaro;

//COLAS
t_queue* cola_new;
t_queue* cola_exit;

//VARIABLES
float ALPHA_SJF;
int PUERTO_ESCUCHA;
char* IP;
int METRICS_TIMER;
int MAX_MULTIPROG;
char** SEM_IDS;
char** SEM_INIT;
char** SEM_MAX;

#endif /* GLOBALES_H_ */
