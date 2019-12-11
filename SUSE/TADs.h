#ifndef TADS_H_
#define TADS_H_

#include <commons/collections/list.h>
#include <stdlib.h>
#include <stdint.h>
#include "globales.h"
#include <hilolay/hilolay.h>


//Enums
enum colas{
	COLA_READY = 0,
	COLA_EXEC = 1,
};

enum estado{
	NEW = 0,
	READY = 1,
	EXECUTE=2,
	BLOCKED = 3,
	EXIT=4

};

typedef struct{

	t_list* hilos_del_programa;//lista de hilos_t*
	sem_t procesos_en_ready;
	pthread_mutex_t proceso_en_exec;
	pthread_mutex_t mutex_cola_exec_scn;
	pthread_mutex_t mutex_cola_exec_sj;
	pthread_mutex_t mutex_cola_exec_sc;
	pthread_mutex_t mutex_cola_ready;
	pthread_mutex_t semaforo_scheduler_next;
	pthread_mutex_t	semaforo_join;
	pthread_mutex_t semaforo_close;
	pthread_mutex_t mutex_cola_exec;
	t_dictionary* diccionario_joineados_por_tid;
}proceso_t;

typedef struct{
	char* nombre_del_semaforo;
	int tid;
}semaforo_descifrado_t;

typedef struct{

	int valor_max;
	int valor_inicial;
	int valor_actual;

}valores_semaforo_t;


typedef struct{
	int tiempo_de_ejecucion;// en ms
	int tiempo_de_espera; // en ms
	int tiempo_de_uso_del_cpu;// en ms
	float porcentaje_total_tiempo_de_ejecucion_de_hilos;
}metricas_t;

typedef struct{
	long tiempo_en_ejecucion_real;
	long tiempo_llegada_a_EXEC;
	long tiempo_llegada_a_NEW;
	long tiempo_llegada_a_READY;
	long sumatoria_tiempos_en_EXEC;
	long sumatoria_tiempos_en_READY;
}tiempos_t;


typedef struct{
	int PID; // Del programa al que pertenece
	hilolay_t* hilo_informacion; // Unico por thread
	int estado_del_hilo;
	tiempos_t* tiempos;
	metricas_t* metricas;
	float prioridad;
}hilo_t;


#endif /* TADS_H_ */
