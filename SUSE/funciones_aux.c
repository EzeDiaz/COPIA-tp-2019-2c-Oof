/*
 * funciones_aux.c
 *
 *  Created on: 6 oct. 2019
 *      Author: utnso
 */

#include "metricas.h"
#include "funciones_aux.h"

#include <commons/collections/dictionary.h>

float calcular_sjf(hilo_t*un_hilo){

return ( (1-ALPHA_SJF)* un_hilo->prioridad + ALPHA_SJF * un_hilo->metricas->tiempo_de_ejecucion);

}


//HILOS

hilo_t* buscar_hilo_por_TID(int TID,int PID){

	hilo_t* hilo_a_retornar;

	char*pid= string_itoa(PID);

	if(dictionary_has_key(diccionario_de_procesos,pid)){

		proceso_t* un_proceso=dictionary_get(diccionario_de_procesos,pid);

		void buscar_hilo(hilo_t* un_hilo){
			if(un_hilo->hilo_informacion->tid==TID)
			hilo_a_retornar=un_hilo;
		}

		list_iterate(un_proceso->hilos_del_programa,buscar_hilo);

	}


	return hilo_a_retornar;

}


// COLAS

t_queue* obtener_cola_ready_de(char* PID){
	return obtener_cola_de(PID,COLA_READY);
}

t_queue* obtener_cola_exec_de(char* PID){
	return obtener_cola_de(PID,COLA_EXEC);
}

t_queue* obtener_cola_de(char* PID, int cola){

	sem_wait(&semaforo_diccionario_procesos_x_queues);
	t_list* vector_cola= dictionary_get(diccionario_procesos_x_queues,PID);

	sem_post(&semaforo_diccionario_procesos_x_queues);
	return list_get(vector_cola,cola);
}

// LOGS

void incializar_logs_sistema(){


	remove("log_metricas.log");//no va a pasar como el tp anterior que tenias un log de 15 MB
	remove("log_colas.log");
	remove("log_deserializacion.log");
	remove("log_metricas_programa.log");
	remove("log_metricas_hilo.log");
	remove("logger.log");

	logger_de_deserializacion= log_create("log_deserializacion.log","Deserializacion",0,LOG_LEVEL_INFO);
	log_metricas_programa = log_create("log_metricas_programa.log","log_metricas_programa",0,LOG_LEVEL_INFO);
	log_metricas_hilo = log_create("log_metricas_hilo.log","log_metricas_hilo",0,LOG_LEVEL_INFO);
	log_metricas_sistema = log_create("log_metricas.log","log_metricas",0,LOG_LEVEL_INFO);
	log_colas = log_create("log_colas.log","log_colas",0,LOG_LEVEL_INFO);
	logger = log_create("logger.log","logger",0,LOG_LEVEL_INFO);

}

void destruir_logs_sistema(){
	log_destroy(logger_de_deserializacion);
	log_destroy(log_metricas_programa);
	log_destroy(log_metricas_hilo);
	log_destroy(log_metricas_sistema);//destruyo el log y borro el archivo
	log_destroy(log_colas);
	log_destroy(logger);



}

// CONFIG

void leer_config_y_setear(){

	char* nombre_config;
	t_config* config;

	//PUERTO_ESCUCHA= (char*) malloc(4);
	IP=string_new();

	nombre_config = readline("Ingresar nombre del config a utilizar: ");

	config = config_create(nombre_config);

	char* aux = string_new();
	string_append(&aux, "Leimos el config: ");
	string_append(&aux, nombre_config);
	string_append(&aux, "\n");
	log_info(logger,aux);
	free(aux);

	strcpy(IP,config_get_string_value(config, "IP"));

	PUERTO_ESCUCHA = config_get_int_value(config,"PUERTO_ESCUCHA");

	config_destroy(config);
}



