#include "colas.h"

/*
 * Vamos a manejar los distintos estados con colas.

 * De momento vamos a suponer que tenemos un diccionario con los thread activos. Estos van a estar identificados por un TID y va a ser
   este TID el que se va a pasar entre las colas y nos va a permitir rescatar al thread.

 */

/// ******************************************************************************************************** ///
/// ************************************ PASAR THREADS DE NEW A READY ************************************** ///
/// ******************************************************************************************************** ///



void newToReady(){

	sem_wait(&grado_de_multiprogramacion_contador);

	sem_wait(&mutex_cola_new);
	hilo_t*  hilo = queue_pop(cola_new);
	sem_post(&mutex_cola_new);

	char* pid=string_itoa(hilo->PID);
	t_queue*cola_ready = obtener_cola_ready_de(pid);
	t_queue* cola_exec = obtener_cola_exec_de(pid);
	sem_wait(&semaforo_diccionario_de_procesos);
	proceso_t* un_proceso = dictionary_get(diccionario_de_procesos,pid);
	sem_post(&semaforo_diccionario_de_procesos);

	long milisegundos_ready= time(NULL);
	hilo->tiempos->tiempo_llegada_a_READY = milisegundos_ready;

	sem_wait(&un_proceso->mutex_cola_ready);
	queue_push(cola_ready,hilo);
	sem_post(&un_proceso->mutex_cola_ready);

	sem_wait(&hilo->mutex_estado_del_hilo);
	hilo->estado_del_hilo = READY;
	sem_post(&hilo->mutex_estado_del_hilo);

	sem_post(&un_proceso->procesos_en_ready);

	sem_wait(&semaforo_log_colas);
	log_info(log_colas,"Se paso el hilo a la cola Ready \n");
	sem_post(&semaforo_log_colas);

	/// ******************************************************************************************************** ///
	/// ************************************ ESTOY PROBANDO ALGO********* ************************************** ///
	/// ******************************************************************************************************** ///
	/*if(queue_is_empty(cola_exec) && !queue_is_empty(cola_ready)){

		sem_wait(&mutex_suse_schedule_next);
		suse_schedule_next(hilo->PID); //Quizas estaria bueno igualar esto a "tid"
		sem_post(&mutex_suse_schedule_next);
	}
	 */
	poner_cola_exec_de(pid,cola_exec);
	poner_cola_ready_de(pid,cola_ready);
	free(pid);

}

void estadoNew()
{
	// El booleano finConsola esta en false desde el inicio, en el momento en el que el kernel quiera frenar la planificiacion esta variable pasara a true
	// y se frenara la planificacion

	while(suse_esta_atendiendo)
	{
		sem_wait(&sem_encolar_en_new);
		sem_wait(&procesos_en_new);

		//ESTOS SEMAFOROS NO SE SI VAN
		sem_wait(&mutex_new_to_ready);
		newToReady();
		sem_post(&mutex_new_to_ready);
		//Tiene que chequear el grado de multiprogramacion y si hay lugar lo pasa a Ready

		//return NULL;
	}
}

void encolar_en_new(hilo_t* hilo){

	long milisegundos_new= time(NULL);
	hilo->tiempos->tiempo_llegada_a_NEW = milisegundos_new;

	sem_wait(&mutex_cola_new);
	queue_push(cola_new,hilo);
	sem_post(&sem_encolar_en_new);
	sem_post(&mutex_cola_new);

}


/// ******************************************************************************************************** ///
/// ****************************************** PROCESOS EN EXEC  ******************************************* ///
/// ******************************************************************************************************** ///

void exec(hilo_t* hilo){

	long milisegundos_exec= time(NULL);
	sem_wait(&hilo->mutex_tiempo_exec);
	hilo->tiempos->tiempo_llegada_a_EXEC = milisegundos_exec;
	sem_post(&hilo->mutex_tiempo_exec);
	sem_wait(&hilo->mutex_sumatoria_ready);
	hilo->tiempos->sumatoria_tiempos_en_READY += (milisegundos_exec- hilo->tiempos->tiempo_llegada_a_READY);
	sem_post(&hilo->mutex_sumatoria_ready);
}

void ejecutar_funcion(hilo_t* hilo){

	//TODO


}
/* void * estadoExec()
{
	while(!finDePlanificacion())
	{
		if(!finDePlanificacion())
		{
			sem_wait(&procesosEnExec);
			sem_wait(&cpuDisponible);

			execToReady();

		}
	}

	return NULL;
}
 */

void exec_to_exit(hilo_t* hilo){

	char* pid = string_itoa(hilo->PID);
	char* hilo_info = string_itoa(hilo->hilo_informacion->tid);

	sem_wait(&hilo->mutex_tiempo_exec);
	sem_wait(&hilo->mutex_sumatoria_exec);
	int milisegundos_exit= time(NULL);
	hilo->tiempos->sumatoria_tiempos_en_EXEC += milisegundos_exit - hilo->tiempos->tiempo_llegada_a_EXEC;
	hilo->tiempos->tiempo_en_ejecucion_real = milisegundos_exit - hilo->tiempos->tiempo_llegada_a_EXEC;
	sem_post(&hilo->mutex_sumatoria_exec);
	sem_post(&hilo->mutex_tiempo_exec);

	proceso_t* un_proceso=obtener_proceso(hilo->PID);
	t_queue* cola_ready=obtener_cola_ready_de(pid);
	t_list* lista_de_joineados_x_hilo=dictionary_get(un_proceso->diccionario_joineados_por_tid,hilo_info);

	if(lista_de_joineados_x_hilo!=NULL && !list_is_empty(lista_de_joineados_x_hilo)){

		for (int i=0; i<lista_de_joineados_x_hilo->elements_count;i++){
			int tid_para_ready= list_remove(lista_de_joineados_x_hilo,0);
			bool mismo_tid(hilo_t* un_hilo){

				bool mismo_pid=un_hilo->PID==hilo->PID;
				bool mismo_tid_que_el_hilo=un_hilo->hilo_informacion->tid==tid_para_ready;
				return mismo_tid_que_el_hilo&&mismo_pid;

			}
			dictionary_put(un_proceso->diccionario_joineados_por_tid,hilo_info,lista_de_joineados_x_hilo);
			sem_wait(&mutex_lista_bloqueados);
			hilo_t* hilo_joineado=list_remove_by_condition(bloqueados,mismo_tid);
			sem_post(&mutex_lista_bloqueados);
			sem_wait(&un_proceso->mutex_cola_ready);

			long milisegundos_ready= time(NULL);
			hilo->tiempos->tiempo_llegada_a_READY = milisegundos_ready;

			queue_push(cola_ready,hilo_joineado);
			sem_post(&un_proceso->mutex_cola_ready);
			sem_post(&un_proceso->procesos_en_ready);
		}
	}

	sem_wait(&hilo->mutex_estado_del_hilo);
	hilo->estado_del_hilo=EXIT;
	sem_post(&hilo->mutex_estado_del_hilo);

	sem_wait(&mutex_cola_exit);
	queue_push(cola_exit,hilo);
	sem_post(&mutex_cola_exit);

	free(pid);
	free(hilo_info);

}


// ESTADO EXIT

void exit_thread(hilo_t* hilo){
	//Cuando termina de ejecutar la funcion del hilo, este "muere" y viene a exit ¿haciendo una cola de threads terminados?

	exec_to_exit(hilo);

	sem_post(&grado_de_multiprogramacion_contador);
	mostrar_metricas();
	sem_post(&procesos_en_new);

}


