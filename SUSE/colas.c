/*
 * Vamos a manejar los distintos estados con colas.

 * De momento vamos a suponer que tenemos un diccionario con los thread activos. Estos van a estar identificados por un TID y va a ser
   este TID el que se va a pasar entre las colas y nos va a permitir rescatar al thread.

 */

// SE CREA EL HILO

th_create()
{
	t_hilo* hilo = suse_create(/*FUNCION*/) //t_hilo un struct hilo ¿con funcion y TID?

	queue_push(cola_New,hilo)
}


/// ******************************************************************************************************** ///
/// ************************************ PASAR THREADS DE NEW A READY ************************************** ///
/// ******************************************************************************************************** ///

void newToReady()
{
	int i = 0;

	t_hilo*  hilo = queue_pop(cola_New);

	sem_wait(&mx_colaReady);

	queue_push(cola_Ready,proceso);

	sem_post(&mx_colaReady);

	sem_post(&procesosEnReady);

	sem_wait(&mx_log);

	log_info(logger,"Se paso el hilo a la cola Ready \n");

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

	t_hilo* hilo = queue_pop(cola_Ready);

	sem_post(&mx_colaReady);

	sem_wait(&mx_log);

	log_info(logger,"Se paso el proceso a Exec \n");

	sem_post(&mx_log);

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
		sem_wait(&mx_diccionario);

		dictionary_put(diccionarioProcesos,proceso,comandos); // vuelvo a meter la lista de comandos que no ejecutaron en el diccionario

		sem_post(&mx_diccionario);

		sem_wait(&mx_colaReady);

		queue_push(cola_Ready,proceso);

		sem_post(&mx_colaReady);

		sem_post(&procesosEnReady);

		sem_wait(&mx_log);

		log_info(logger,"Se devolvio el proceso a la cola Ready \n");
		log_info(logger,"PID: %s \n",proceso);

		sem_post(&mx_log);

	}
	else
	{
		sem_wait(&mx_diccionario);

		dictionary_remove_and_destroy(diccionarioProcesos, proceso, (void*)elAsesinoIndomableDeProcesos); //termino de ejecutar todos sus comandos -> chau proceso

		sem_post(&mx_diccionario);
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

