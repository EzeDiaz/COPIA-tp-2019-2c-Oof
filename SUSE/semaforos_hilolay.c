/*
 * semaforos_hilolay.c
 *
 *  Created on: 28 oct. 2019
 *      Author: utnso
 */

#include "semaforos_hilolay.h"
#include "TADs.h"

void bloquear_hilo(char* nombre_semaforo, int PID){

	t_queue* cola_exec = obtener_cola_exec_de(PID);
	hilo_t* hilo=queue_pop(cola_exec);
	sem_wait(&semaforo_estado_blocked);
	t_list* lista_bloqueados = dictionary_get(diccionario_bloqueados_por_semafaro,nombre_semaforo);
	list_add(lista_bloqueados,hilo);
	hilo->estado_del_hilo = BLOCKED;
	sem_post(&semaforo_estado_blocked);

}

bool wait(char*nombre_semaforo,int PID){

	sem_wait(&semaforo_diccionario_por_semaforo);
	int value=dictionary_get(diccionario_de_valor_por_semaforo,nombre_semaforo);
	sem_post(&semaforo_diccionario_por_semaforo);
	if(value){
		sem_wait(&semaforo_diccionario_por_semaforo);
		dictionary_put(diccionario_de_valor_por_semaforo,nombre_semaforo,value-1);
		sem_post(&semaforo_diccionario_por_semaforo);
	}else{
		hilo_t* hilo=queue_pop(obtener_cola_exec_de(PID));
		bloquear_hilo(nombre_semaforo,hilo->PID);
	}

	return dictionary_has_key(diccionario_de_valor_por_semaforo,nombre_semaforo);
}


int signal(char*nombre_semaforo, int PID){

	sem_wait(&semaforo_diccionario_por_semaforo);
	int value=dictionary_get(diccionario_de_valor_por_semaforo,nombre_semaforo);
	value--;
	dictionary_put(diccionario_de_valor_por_semaforo,nombre_semaforo,value);
	sem_post(&semaforo_diccionario_por_semaforo);
	desbloquear_hilo(nombre_semaforo, PID);



	return (bool)value;
}

void desbloquear_hilo(char* nombre_semaforo, int PID){

	sem_wait(&semaforo_estado_blocked);
	t_list* lista_bloqueados =dictionary_get(diccionario_bloqueados_por_semafaro,nombre_semaforo);
	sem_post(&semaforo_estado_blocked);
	t_queue* cola_ready = obtener_cola_ready_de(PID);
	queue_push(cola_ready,list_take(lista_bloqueados,1));

}





