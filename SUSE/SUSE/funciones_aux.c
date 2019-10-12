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

return ( (1-alpha)* un_hilo->prioridad + alpha * un_hilo->metricas->tiempo_de_ejecucion);


}

// COLAS

t_queue* obtener_cola_ready_de(int PID){
	return obtener_cola_de(PID,COLA_READY);
}

t_queue* obtener_cola_exec_de(int PID){
	return obtener_cola_de(PID,COLA_EXEC);
}

t_queue* obtener_cola_de(int PID, int cola){
	sem_wait(semaforo_diccionario_procesos_x_queues);
	t_queue** vector_cola = dictionary_get(diccionario_procesos_x_queues,PID);

	sem_post(&semaforo_diccionario_procesos_x_queues);
	return vector_cola[cola];
}

// LOGS

void incializar_logs_sistema(){

	log_metricas_sistema = log_create("log_metricas.log","log_metricas",0,LOG_LEVEL_INFO);
	log_colas = log_create("log_colas.log","log_colas",0,LOG_LEVEL_INFO);


}

void destruir_logs_sistema(){

	log_destroy(log_metricas_sistema);//destruyo el log y borro el archivo
	log_destroy(log_colas);

	remove("log_metricas.log");//no va a pasar como el tp anterior que tenias un log de 15 MB
	remove("log_colas.log");

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



