#include "colas.h"


/*
 * Vamos a manejar los distintos estados con colas.

 * De momento vamos a suponer que tenemos un diccionario con los thread activos. Estos van a estar identificados por un TID y va a ser
   este TID el que se va a pasar entre las colas y nos va a permitir rescatar al thread.

 */

// SE CREA EL HILO


/*void th_create()
{
	hilo_t* hilo = suse_create(/*FUNCION*/ //); //t_hilo un struct hilo ¿con funcion y TID?
/*
	queue_push(cola_new,hilo);
}*/


/// ******************************************************************************************************** ///
/// ************************************ PASAR THREADS DE NEW A READY ************************************** ///
/// ******************************************************************************************************** ///



void newToReady(){

	sem_wait(&grado_de_multiprogramacion_contador);
	hilo_t*  hilo = queue_pop(cola_new);
	char* pid=string_itoa(hilo->PID);
	t_queue*cola_ready = obtener_cola_ready_de(pid);

	queue_push(cola_ready,hilo);

	hilo->estado_del_hilo = READY;

	sem_wait(&semaforo_diccionario_de_procesos);
	proceso_t* un_proceso = dictionary_get(diccionario_de_procesos,pid);
	sem_post(&semaforo_diccionario_de_procesos);
	sem_post(&un_proceso->procesos_en_ready);

	sem_wait(&semaforo_log_colas);

	log_info(log_colas,"Se paso el hilo a la cola Ready \n");

	free(pid);

}

void * estadoNew()
{
	// El booleano finConsola esta en false desde el inicio, en el momento en el que el kernel quiera frenar la planificiacion esta variable pasara a true
	// y se frenara la planificacion

	while(!finDePlanificacion)
	{
		if(!finDePlanificacion&& suse_esta_atendiendo && (cola_new->elements->elements_count>0))
		{
			sem_wait(&procesos_en_new);

			newToReady();
			//Tiene que chequear el grado de multiprogramacion y si hay lugar lo pasa a Ready

		}
	}

	return NULL;
}

void encolar_en_new(hilo_t* hilo){

	queue_push(cola_new,hilo);
}

/// ******************************************************************************************************** ///
/// ************************************* PASAR THREADS DE READY A EXEC ************************************ ///
/// ******************************************************************************************************** ///


void readyToExec(int PID)
{
	// Habria que chequear que entre UN SOLO thread a exec POR proceso

	char* pid=string_itoa(PID);
	t_queue* cola_exec = obtener_cola_exec_de(pid);

	while(1){

		if(esta_vacia(cola_exec)){
			char* tiempo_inicio= temporal_get_string_time();
			char** tiempo_inicio_separado_por_dos_puntos = string_split(tiempo_inicio,":");
			long milisegundos_inicial= string_itoa(tiempo_inicio_separado_por_dos_puntos[3]);

			hilo_t* hilo=suse_schedule_next(PID);

			if(hilo == NULL){

			}else{
			sem_wait(&semaforo_diccionario_procesos_x_semaforo);
			sem_t* semaforo_exec_x_proceso = dictionary_get(diccionario_de_procesos_x_semaforo,pid);
			sem_post(&semaforo_diccionario_procesos_x_semaforo);

			sem_wait(&semaforo_exec_x_proceso);
			queue_push(cola_exec,hilo);

			hilo->estado_del_hilo = EXECUTE;

			char* tiempo_fin= temporal_get_string_time();
			char** tiempo_fin_separado_por_dos_puntos = string_split(tiempo_inicio,":");
			int milisegundos_final= string_itoa(tiempo_fin_separado_por_dos_puntos[3]);

			hilo->metricas->tiempo_de_espera += milisegundos_final-milisegundos_inicial;

			sem_post(&semaforo_exec_x_proceso);


			sem_wait(&semaforo_log_colas);
			log_info(log_colas,"Se paso el proceso a Exec \n");
			sem_post(&semaforo_log_colas);
			}
		}
	}

	free(pid);
}


bool esta_vacia(t_queue* cola_exec){
	bool cacona = queue_is_empty(cola_exec);
	cacona=cola_exec->elements->elements_count==0;
	return cacona;

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

			sem_wait(un_proceso->procesos_en_ready);


			readyToExec(PID);

		}

		// Quito el primer elemento de la cola de ready, valido que no haya sido finalizado y lo pongo en la cola de exec.
		// En caso de no encontrar uno para poder trabajar no hago nada
	}
	return NULL;
}

/// ******************************************************************************************************** ///
/// ****************************************** PROCESOS EN EXEC  ******************************************* ///
/// ******************************************************************************************************** ///

void exec(hilo_t* hilo){

	char* tiempo_inicio= temporal_get_string_time();
	char** tiempo_inicio_separado_por_dos_puntos = string_split(tiempo_inicio,":");
	int milisegundos_inicial= string_itoa(tiempo_inicio_separado_por_dos_puntos[3]);

	ejecutar_funcion(hilo);

	char* tiempo_final= temporal_get_string_time();
	char** tiempo_final_separado_por_dos_puntos = string_split(tiempo_inicio,":");
	int milisegundos_final= string_itoa(tiempo_inicio_separado_por_dos_puntos[3]);

	hilo->metricas->tiempo_de_uso_del_cpu += milisegundos_final-milisegundos_inicial;







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

	char* clave=string_new();
	string_append(&clave,string_itoa(hilo->PID));
	string_append(&clave,string_itoa(hilo->hilo_informacion->tid));

	t_list* bloqueados_por_join=dictionary_get(diccionario_bloqueados_por_semafaro,clave);

	if(bloqueados_por_join->elements_count>0){
		t_queue* cola_ready=obtener_cola_ready_de(hilo->PID);
		void pusheador(hilo_t* un_hilo){

			queue_push(cola_ready,un_hilo);

		}

		list_iterate(bloqueados_por_join,pusheador);
	}

	queue_push(cola_exit,hilo);
	hilo->estado_del_hilo=EXIT;

}


// ESTADO EXIT

void exit_thread(hilo_t* hilo){
	//Cuando termina de ejecutar la funcion del hilo, este "muere" y viene a exit ¿haciendo una cola de threads terminados?

	exec_to_exit(hilo);
	sem_wait(&semaforo_lista_procesos_finalizados);
	list_add(hilos_finalizados,hilo);
	sem_post(&semaforo_lista_procesos_finalizados);
	sem_post(&grado_de_multiprogramacion_contador);
	mostrar_metricas();
	sem_post(&procesos_en_new);

}


