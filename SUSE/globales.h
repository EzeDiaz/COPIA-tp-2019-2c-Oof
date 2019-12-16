#ifndef GLOBALES_H_
#define GLOBALES_H_

#include <commons/collections/queue.h>
#include <commons/collections/list.h>
#include <commons/collections/dictionary.h>
#include <stdlib.h>
#include <commons/log.h>
#include <pthread.h>
#include <commons/config.h>
#include <semaphore.h>
#include <stdio.h>


//SEMAFOROS

//Mutex
pthread_mutex_t semaforo_log_colas;
pthread_mutex_t semaforo_diccionario_de_procesos;
pthread_mutex_t semaforo_diccionario_procesos_x_queues;
pthread_mutex_t semaforo_diccionario_procesos_x_semaforo;
pthread_mutex_t mutex_log_servidor;
pthread_mutex_t semaforo_lista_procesos_finalizados;
pthread_mutex_t semaforo_diccionario_por_semaforo;
pthread_mutex_t mutex_lista_block;
pthread_mutex_t mutex_diccionario;
pthread_mutex_t mutex_lista;
pthread_mutex_t mutex_cola_new;
pthread_mutex_t mutex_calcular_sjf;
pthread_mutex_t mutex_elementos_en_new;
pthread_mutex_t mutex_log_metricas;
pthread_mutex_t mutex_new_to_ready;
pthread_mutex_t mutex_exec;
pthread_mutex_t mutex_encolar_new;
pthread_mutex_t mutex_suse_schedule_next;
pthread_mutex_t mutex_listas_metricas;
pthread_mutex_t mutex_obtener_tiempo_total;
pthread_mutex_t mutex_lista_hilos_programa;
pthread_mutex_t mutex_wait;
pthread_mutex_t mutex_signal;
pthread_mutex_t mutex_cola_exit;
pthread_mutex_t mutex_cronometro;
pthread_mutex_t mutex_lista_bloqueados;
pthread_mutex_t mutex_bloquear_hilo;
pthread_mutex_t mutex_desbloquear_hilo;
pthread_mutex_t mutex_diccionario_sincro_wait_signal;

//Contador
sem_t grado_de_multiprogramacion_contador;
sem_t procesos_en_new;
//sem_t* semaforo_lista_procesos_finalizados;

//Binario
sem_t sem_encolar_en_new;


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
t_dictionary* diccionario_sincro_wait_signal;

//COLAS
t_queue* cola_new;
t_queue* cola_exit;
t_list* bloqueados;
//VARIABLES
float ALPHA_SJF;
int PUERTO_ESCUCHA;
char* IP;
int METRICS_TIMER;
int MAX_MULTIPROG;
char** SEM_IDS;
char** SEM_INIT;
char** SEM_MAX;
bool suse_esta_atendiendo;

#endif /* GLOBALES_H_ */
