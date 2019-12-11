/*
 * colas2.c
 *
 *  Created on: 5 dic. 2019
 *      Author: utnso
 */

/*
#include "colas.h"

void encolar_en_new(hilo_t* hilo){

	long milisegundos_new= time(NULL);

	hilo->tiempos->tiempo_llegada_a_NEW = milisegundos_new;

	queue_push(cola_new,hilo);
}

void * estadoNew(){
	// El booleano finConsola esta en false desde el inicio, en el momento en el que el kernel quiera frenar la planificiacion esta variable pasara a true
	// y se frenara la planificacion

	while(suse_esta_atendiendo)
	{
		if(cola_new->elements->elements_count>0)
		{
			sem_wait(&procesos_en_new);
			newToReady();
			//Tiene que chequear el grado de multiprogramacion y si hay lugar lo pasa a Ready
		}
	}

	return NULL;
}

void newToReady(){

	sem_wait(&grado_de_multiprogramacion_contador);
	hilo_t*  hilo = queue_pop(cola_new); // LO SACO DE NEW

	char* pid=string_itoa(hilo->PID);
	t_queue*cola_ready = obtener_cola_ready_de(pid);

	long milisegundos_ready= time(NULL);
	hilo->tiempos->tiempo_llegada_a_READY = milisegundos_ready;

	queue_push(cola_ready,hilo); //LO METO EN READY
	hilo->estado_del_hilo = READY;

	sem_wait(&semaforo_diccionario_de_procesos);
	proceso_t* un_proceso = dictionary_get(diccionario_de_procesos,pid);
	sem_post(&semaforo_diccionario_de_procesos);
	sem_post(&un_proceso->procesos_en_ready);

	sem_wait(&semaforo_log_colas);
	log_info(log_colas,"Se paso el hilo a la cola Ready \n");
	sem_post(&semaforo_log_colas);

	free(pid);

}

void * estadoReady(int PID)
{
	// El booleano finConsola esta en false desde el inicio, en el momento en el que el kernel quiera frenar la planificiacion esta variable pasara a true
	// y se frenara la planificacion
	char*pid=string_itoa(PID);
	while(!finDePlanificacion)
	{
		if(!finDePlanificacion)
		{
			sem_wait(&semaforo_diccionario_de_procesos);
			proceso_t* un_proceso = dictionary_get(diccionario_de_procesos,pid);
			sem_post(&semaforo_diccionario_de_procesos);

			//sem_wait(&un_proceso->procesos_en_ready);

			readyToExec(PID);
		}

		// Quito el primer elemento de la cola de ready, valido que no haya sido finalizado y lo pongo en la cola de exec.
		// En caso de no encontrar uno para poder trabajar no hago nada
	}
	free(pid);
	return NULL;
}

void readyToExec(int PID){
	// Habria que chequear que entre UN SOLO thread a exec POR proceso

	char* pid=string_itoa(PID);
	t_queue* cola_exec = obtener_cola_exec_de(pid);
	t_queue* cola_ready = obtener_cola_ready_de(pid);

	while(1){

		//poner semaforo, mucha espera activa
		if(queue_is_empty(cola_exec) && !queue_is_empty(cola_ready)){

			int tid=suse_schedule_next(PID);

			bool tienen_mismo_tid(hilo_t*un_hilo){

				return un_hilo->hilo_informacion->tid == hilo->hilo_informacion->tid;
			}

			//hilo_t* hilo= buscar_hilo_por_TID(tid,PID);

			proceso_t* un_proceso=dictionary_get(diccionario_de_procesos,pid);
			sem_wait(&un_proceso->procesos_en_ready);

			bool mismo_tid(hilo_t* un_hilo){
				return un_hilo->hilo_informacion->tid==tid;
			}
			// Se fija que el tid del suse_schedule_next sea igual al que esta en la cola_ready
			hilo_t* hilo = list_remove_by_condition(cola_ready->elements,mismo_tid); //LO SACO DE READY

			sem_wait(&semaforo_diccionario_procesos_x_semaforo);
			pthread_mutex_t* semaforo_exec_x_proceso = dictionary_get(diccionario_de_procesos_x_semaforo,pid);
			sem_post(&semaforo_diccionario_procesos_x_semaforo);

			sem_wait(semaforo_exec_x_proceso);

			queue_push(cola_exec,hilo);
			hilo->estado_del_hilo = EXECUTE;
			estadoExec(hilo); //LO METO DE EXEC

			sem_post(semaforo_exec_x_proceso);


			sem_wait(&semaforo_log_colas);
			log_info(log_colas,"Se paso el proceso a Exec \n");
			sem_post(&semaforo_log_colas);

		}
	}

	free(pid);
}


void estadoExec(hilo_t* hilo){

	long milisegundos_exec= time(NULL);

	hilo->tiempos->tiempo_llegada_a_EXEC = milisegundos_exec;
	hilo->tiempos->sumatoria_tiempos_en_READY += hilo->tiempos->tiempo_llegada_a_EXEC - hilo->tiempos->tiempo_llegada_a_READY;
}
*/
