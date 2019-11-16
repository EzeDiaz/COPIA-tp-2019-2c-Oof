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


//NO SE SUPONE QUE VOY A TENER VARIOS DE ESTOS ESTADOS READY?
void newToReady(){

	sem_wait(grado_de_multiprogramacion_contador);
	hilo_t*  hilo = queue_pop(cola_new);

	t_queue*cola_ready = obtener_cola_ready_de(hilo->PID);

	queue_push(cola_ready,hilo);

	hilo->estado_del_hilo = READY;

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

void encolar_en_new(hilo_t* hilo){

	queue_push(cola_new,hilo);
}

/// ******************************************************************************************************** ///
/// ************************************* PASAR THREADS DE READY A EXEC ************************************ ///
/// ******************************************************************************************************** ///


void readyToExec(int PID)
{
	// Habria que chequear que entre UN SOLO thread a exec POR proceso

	t_queue* cola_Exec = obtener_cola_exec_de(PID);

	while(1){
		if(esta_vacia(cola_Exec)){
			char* tiempo_inicio= temporal_get_string_time();
			char** tiempo_inicio_separado_por_dos_puntos = string_split(tiempo_inicio,":");
			int milisegundos_inicial= string_itoa(tiempo_inicio_separado_por_dos_puntos[3]);

			hilo_t* hilo=suse_scheduler_next(PID);

			sem_wait(&semaforo_diccionario_procesos_x_semaforo);
			sem_t* semaforo_exec_x_proceso = dictionary_get(diccionario_de_procesos_x_semaforo,string_itoa(PID));
			sem_post(&semaforo_diccionario_procesos_x_semaforo);

			sem_wait(&semaforo_exec_x_proceso);
			queue_push(cola_Exec,hilo);

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


bool esta_vacia(t_queue* cola_Exec){

	return queue_is_empty(cola_Exec);

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
	char* tiempo_inicio= temporal_get_string_time();
	char** tiempo_inicio_separado_por_dos_puntos = string_split(tiempo_inicio,":");
	int milisegundos_inicial= string_itoa(tiempo_inicio_separado_por_dos_puntos[3]);

	//TODO SEMAFOROS PROPIOS PARA ECHAR A LOS HILOS A BLOCKED
	//sem_wait(&semaforo_diccionario_de_procesos);

	//dictionary_put(diccionario_de_procesos,proceso,comandos); // vuelvo a meter la lista de comandos que no ejecutaron en el diccionario

	//sem_post(&semaforo_diccionario_de_procesos);

	ejecutar_funcion(hilo);

	char* tiempo_final= temporal_get_string_time();
	char** tiempo_final_separado_por_dos_puntos = string_split(tiempo_inicio,":");
	int milisegundos_final= string_itoa(tiempo_inicio_separado_por_dos_puntos[3]);

	hilo->metricas->tiempo_de_uso_del_cpu += milisegundos_final-milisegundos_inicial;


	exec_to_exit(hilo);




}

void ejecutar_funcion(hilo_t* hilo){




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
	sem_wait(&semaforo_lista_procesos_finalizados);


	list_add(hilos_finalizados,hilo);
	sem_post(&semaforo_lista_procesos_finalizados);
	sem_post(grado_de_multiprogramacion_contador);
	mostrar_metricas();

}



// ESTADO BLOCKED

void blocked(char* nombre_semaforo, hilo_t* un_hilo)
{
	// espera a que el recurso que lo metio aca sea liberado, es decir que el respectivo semaforo que lo metio lo deje salir
	while(/*semaforo_ocupado(nombre_semaforo) TODO  */ 1){

		//esto puede ser un wait de un semaforo posta y nos olvidamos de la espera activa del wait TODO
	}

	t_queue* cola_ready= obtener_cola_ready_de(un_hilo->PID);
	queue_push(cola_ready,un_hilo);

}

