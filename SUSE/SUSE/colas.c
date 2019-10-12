#include "colas.h"


/*
 * Vamos a manejar los distintos estados con colas.

 * De momento vamos a suponer que tenemos un diccionario con los thread activos. Estos van a estar identificados por un TID y va a ser
   este TID el que se va a pasar entre las colas y nos va a permitir rescatar al thread.

 */

// SE CREA EL HILO

hilo_t* suse_create(/*FUNCION*/){


	return NULL;
}

void th_create()
{
	hilo_t* hilo = suse_create(/*FUNCION*/); //t_hilo un struct hilo ¿con funcion y TID?

	queue_push(cola_new,hilo);
}


/// ******************************************************************************************************** ///
/// ************************************ PASAR THREADS DE NEW A READY ************************************** ///
/// ******************************************************************************************************** ///


//NO SE SUPONE QUE VOY A TENER VARIOS DE ESTOS ESTADOS READY?
void newToReady()
{
	int i = 0;

	hilo_t*  hilo = queue_pop(cola_new);

	t_queue*cola_ready = obtener_cola_ready_de(hilo->PID);

	queue_push(cola_ready,hilo);

	sem_post(&procesos_en_Ready);

	sem_wait(&semaforo_log_colas);

	log_info(log_colas,"Se paso el hilo a la cola Ready \n");

}

void * estadoNew()
{
	// El booleano finConsola esta en false desde el inicio, en el momento en el que el kernel quiera frenar la planificiacion esta variable pasara a true
	// y se frenara la planificacion

	while(!finDePlanificacion)
	{
		if(!finDePlanificacion)
		{
			sem_wait(&procesos_en_New);

			newToReady();
			//Tiene que chequear el grado de multiprogramacion y si hay lugar lo pasa a Ready

		}
	}

	return NULL;
}


/// ******************************************************************************************************** ///
/// ************************************* PASAR THREADS DE READY A EXEC ************************************ ///
/// ******************************************************************************************************** ///


void readyToExec(int PID)
{
	// Habria que chequear que entre UN SOLO thread a exec POR proceso


	t_queue* cola_Ready = obtener_cola_ready_de(PID);
	t_queue* cola_Exec = obtener_cola_exec_de(PID);
	void sjf(hilo_t* un_hilo){
		un_hilo->prioridad = calcular_sjf(un_hilo);
	}

	list_iterate(cola_Ready->elements,sjf);

	bool elemento_mas_grande(hilo_t* hilo_mas_prioridad,hilo_t*hilo_menor_prioridad){
		return hilo_mas_prioridad->prioridad>hilo_menor_prioridad->PID;
	}
	list_sort(cola_Ready->elements,elemento_mas_grande);

	hilo_t* hilo= list_remove(cola_Ready->elements,0);

	sem_wait(&semaforo_diccionario_procesos_x_semaforo);
	sem_t* semaforo_exec_x_proceso = dictionary_get(diccionario_de_procesos_x_semaforo,string_itoa(PID));
	sem_post(&semaforo_diccionario_procesos_x_semaforo);

	sem_wait(&semaforo_exec_x_proceso);
	queue_push(cola_Exec,hilo);
	sem_post(&semaforo_exec_x_proceso);

	sem_wait(&semaforo_log_colas);
	log_info(log_colas,"Se paso el proceso a Exec \n");
	sem_post(&semaforo_log_colas);

}

void * estadoReady(int PID)
{
	// El booleano finConsola esta en false desde el inicio, en el momento en el que el kernel quiera frenar la planificiacion esta variable pasara a true
	// y se frenara la planificacion

	while(!finDePlanificacion)
	{
		if(!finDePlanificacion)
		{
			sem_wait(&procesos_en_Ready);

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

void exec(hilo_t* hilo)
{

// IGNORAR


		//TODO SEMAFOROS PROPIOS PARA ECHAR A LOS HILOS A BLOCKED
		//sem_wait(&semaforo_diccionario_de_procesos);

		//dictionary_put(diccionario_de_procesos,proceso,comandos); // vuelvo a meter la lista de comandos que no ejecutaron en el diccionario

		//sem_post(&semaforo_diccionario_de_procesos);

		t_queue* cola_Ready= obtener_cola_ready_de(hilo->PID);
		queue_push(cola_Ready,hilo);

		sem_post(&procesos_en_Ready);

		sem_wait(&semaforo_log_colas);

		log_info(log_colas,"Se devolvio el hilo a la cola Ready \n");
		log_info(log_colas,"TID: %d \n",hilo->hilo_informacion->tid);

		sem_post(&semaforo_log_colas);

		exit_thread(hilo);

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

// ESTADO EXIT

void exit_thread(hilo_t* hilo){
	//Cuando termina de ejecutar la funcion del hilo, este "muere" y viene a exit ¿haciendo una cola de threads terminados?
			sem_wait(&semaforo_lista_procesos_finalizados);


			list_add(hilos_finalizados,hilo);
			sem_post(&semaforo_lista_procesos_finalizados);


}



// ESTADO BLOCKED

void blocked()
{
	// Cuando hay un evento que traba la ejecucion del thread (semaforo)


}

