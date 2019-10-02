#include <stdio.h>
#include <stdlib.h>
#include "TADs.h"
#include "colas.h"
#include <hilolay.h>
#include <commons/collections/queue.h>

/*
 * Vamos a manejar los distintos estados con colas.

 * De momento vamos a suponer que tenemos un diccionario con los thread activos. Estos van a estar identificados por un TID y va a ser
   este TID el que se va a pasar entre las colas y nos va a permitir rescatar al thread.

 */

// SE CREA EL HILO

th_create()
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

	sem_wait(&mx_colaReady);

	queue_push(cola_Ready,proceso);

	sem_post(&mx_colaReady);

	sem_post(&procesosEnReady);

	sem_wait(&semaforo_log_colas);

	log_info(log_colas,"Se paso el hilo a la cola Ready \n");

}

void * estadoNew()
{
	// El booleano finConsola esta en false desde el inicio, en el momento en el que el kernel quiera frenar la planificiacion esta variable pasara a true
	// y se frenara la planificacion

	while(!finDePlanificacion())
	{
		if(!finDePlanificacion())
		{
			sem_wait(&procesosEnNew);

			//Tiene que chequear el grado de multiprogramacion y si hay lugar lo pasa a Ready

			newToReady();
		}
	}

	return NULL;
}


/// ******************************************************************************************************** ///
/// ************************************* PASAR THREADS DE READY A EXEC ************************************ ///
/// ******************************************************************************************************** ///


void readyToExec()
{
	// Habria que chequear que entre UN SOLO thread a exec POR programa

	sem_wait(&mx_colaReady);

	hilo_t* hilo = queue_pop(cola_Ready);

	sem_post(&mx_colaReady);

	sem_wait(&semaforo_log_colas);

	log_info(log_colas,"Se paso el proceso a Exec \n");

	sem_post(&semaforo_log_colas);

}

void * estadoReady()
{
	// El booleano finConsola esta en false desde el inicio, en el momento en el que el kernel quiera frenar la planificiacion esta variable pasara a true
	// y se frenara la planificacion

	while(!finDePlanificacion())
	{
		if(!finDePlanificacion())
		{
			sem_wait(&procesosEnReady);

			readyToExec();

		}

		// Quito el primer elemento de la cola de ready, valido que no haya sido finalizado y lo pongo en la cola de exec.
		// En caso de no encontrar uno para poder trabajar no hago nada
	}
	return NULL;
}

/// ******************************************************************************************************** ///
/// ****************************************** PROCESOS EN EXEC  ******************************************* ///
/// ******************************************************************************************************** ///

void exec()
{

// IGNORAR
	if(!list_is_empty(comandos))
	{
		sem_wait(&semaforo_diccionario_de_procesos);

		dictionary_put(diccionario_de_procesos,proceso,comandos); // vuelvo a meter la lista de comandos que no ejecutaron en el diccionario

		sem_post(&semaforo_diccionario_de_procesos);

		sem_wait(&mx_colaReady);

		queue_push(cola_Ready,proceso);

		sem_post(&mx_colaReady);

		sem_post(&procesosEnReady);

		sem_wait(&semaforo_log_colas);

		log_info(log_colas,"Se devolvio el proceso a la cola Ready \n");
		log_info(log_colas,"PID: %s \n",proceso);

		sem_post(&semaforo_log_colas);

	}
	else
	{
		sem_wait(&semaforo_diccionario_de_procesos);

		dictionary_remove_and_destroy(diccionario_de_procesos, proceso, (void*)elAsesinoIndomableDeProcesos); //termino de ejecutar todos sus comandos -> chau proceso

		sem_post(&semaforo_diccionario_de_procesos);
	}

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

void exit()
{
	//Cuando termina de ejecutar la funcion del hilo, este "muere" y viene a exit ¿haciendo una cola de threads terminados?


}



// ESTADO BLOCKED

void blocked()
{
	// Cuando hay un evento que traba la ejecucion del thread (semaforo)


}

