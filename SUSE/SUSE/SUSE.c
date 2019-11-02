/*
 * SUSE.c
 *
 *  Created on: 18 sep. 2019
 *      Author: utnso
 */

#include "funciones_aux.h"
#include "metricas.h"
#include "colas.h"
#include "globales.h"
#include "funciones_aux.h"
#include <unistd.h>
#include <pthread.h>
#include <hilolay/hilolay.h>
#include <semaphore.h>
#include <stdint.h>
#include <sys/types.h>
#include <commons/collections/dictionary.h>
#include <commons/collections/list.h>
#include <commons/string.h>

#define ATTR_C11_THREAD ((void*)(uintptr_t)-1)
void* _suse_create(hilolay_t*, hilolay_attr_t*, void*(*funcion_main)(void*), void*);
void funcion_test(char* );

int main(){

	//_suse_init();

	hilolay_t* hiloprueba;

	while(1){
		printf("hacemos 1 hilo\n");
		_suse_create(hiloprueba,NULL,funcion_test,NULL);
		printf("%d \n",hiloprueba->tid);
	}


	/*DESARROLLO*/

	//terminate_SUSE();
	return 0;
}

void funcion_test(char* cosa){
	printf("saraza");
}

void _suse_init(){

	/*INICIALIZO RECURSOS*/

	hilos_finalizados = list_create();

	/*INICIALIZO LOG*/

	incializar_logs_sistema();

	/*SETEAR CONFIG*/
	/*INICIALIZO DICCIONARIOS*/

	diccionario_de_procesos = dictionary_create();
	diccionario_procesos_x_queues = dictionary_create();
	diccionario_de_procesos_x_semaforo = dictionary_create();

	/*INICIALIZO COLAS*/

	cola_new = queue_create();
	cola_exit = queue_create();

	/*INICIALIZO SEMAFOROS*/

	/*Mutex*/
	sem_init(&semaforo_log_colas,0,1);
	sem_init(&semaforo_diccionario_de_procesos,0,1);
	sem_init(&semaforo_diccionario_procesos_x_queues,0,1);
	sem_init(&semaforo_diccionario_procesos_x_semaforo,0,1);
	sem_init(&semaforo_lista_procesos_finalizados,0,1);
	sem_init(&mutex_log_servidor,0,1);

	/*Contador*/

	sem_init(&procesos_x_grado_de_multiprogramacion,0,0/*MULTIPROGRAMACION*/);
	sem_init(&procesos_en_Ready,0,0);
	sem_init(&procesos_en_New,0,0);
	sem_init(&semaforo_lista_procesos_finalizados,0,0);

}

/*int puesta_en_marcha_del_hilo(hilolay_t* hilo, hilolay_attr_t* atributos_del_hilo, void*(*funcion_main)(void*),void*  argumento){

	const struct hilolay_attr_t* atributo_auxiliar = (struct hilolay_attr_t*) atributos_del_hilo;
	struct hilolay_attr_t atributos_default;
	bool esta_libre_cpu = false;
	bool c11 = (atributos_del_hilo == ATTR_C11_THREAD);
	if(atributo_auxiliar == NULL || c11){
		lll_lock
	}


}*/

void* suse_create(hilolay_t* hilo, hilolay_attr_t* atributos_del_hilo, void*(*funcion_main)(void*), void* argumento){

	hilo=(hilolay_t*)malloc(sizeof(hilolay_t*));
	pthread_t* hilo2;
	pthread_create(&hilo2, NULL, (void*)funcion_test, NULL);
	//memcpy(&(hilo->tid),&hilo2,sizeof(pthread_t*));

	hilo_t* nuevo_hilo= (hilo_t*)malloc(sizeof(hilo_t*)+sizeof(hilolay_t*));


	proceso_t* proceso_correspondiente=dictionary_get(diccionario_de_procesos,getpid());

	if(proceso_correspondiente->hilos_del_programa->elements_count<proceso_correspondiente->grado_de_multiprogramacion)
	{
		metricas_t* metricas=(metricas_t*)malloc(sizeof(metricas_t));
		metricas->tiempo_de_ejecucion=0;
		metricas->tiempo_de_espera=0;
		metricas->tiempo_de_uso_del_cpu=0;
		metricas->porcentaje_total_tiempo_de_ejecucion_de_hilos=0;
		nuevo_hilo->metricas=metricas;

		nuevo_hilo->PID=getpid();
		nuevo_hilo->estado_del_hilo=NEW;
		nuevo_hilo->hilo_informacion=hilo;
		nuevo_hilo->prioridad;
		list_add(proceso_correspondiente->hilos_del_programa,nuevo_hilo);
		hilo->tid=rand()%proceso_correspondiente->grado_de_multiprogramacion;
		pthread_detach(hilo2);
		return hilo;
	}else{

		log_info(logger,"El proceso esta al mango \n");
	}


	return 0;
}



void terminate_SUSE(){


	/*LIBERO RECURSOS GLOBALES*/
	destruir_logs_sistema();


	//destruir_semaforos();

}

void aceptar_proceso(int PID){

	sem_t* semaforo_exec;
	sem_init(semaforo_exec,0,1);
	sem_wait(semaforo_diccionario_procesos_x_semaforo);
	dictionary_put(diccionario_de_procesos_x_semaforo,string_itoa(PID),semaforo_exec);
	sem_post(semaforo_diccionario_procesos_x_semaforo);
	sem_post(procesos_en_New);
}















