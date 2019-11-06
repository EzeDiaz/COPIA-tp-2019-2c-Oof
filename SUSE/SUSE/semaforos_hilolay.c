/*
 * semaforos_hilolay.c
 *
 *  Created on: 28 oct. 2019
 *      Author: utnso
 */

#include "semaforos_hilolay.h"
#include "TADs.h"

void bloquear_hilo(char* nombre_semaforo, hilo_t* hilo){

	t_queue* cola_exec = obtener_cola_exec_de(hilo->PID);
	queue_pop(cola_exec);
	sem_wait(semaforo_estado_blocked);
	t_list* lista_bloqueados = dictionary_get(diccionario_bloqueados_por_semafaro,nombre_semaforo);
	sem_post(semaforo_estado_blocked);
	list_add(lista_bloqueados,hilo);

}

bool wait(char*nombre_semaforo,int TID){

	sem_wait(semaforo_diccionario_por_semaforo);
	int value=dictionary_get(diccionario_de_valor_por_semaforo,nombre_semaforo);
	sem_post(semaforo_diccionario_por_semaforo);
	if(value){
		sem_wait(semaforo_diccionario_por_semaforo);
		dictionary_put(diccionario_de_valor_por_semaforo,nombre_semaforo,value-1);
		sem_post(semaforo_diccionario_por_semaforo);
	}else{
		hilo_t* hilo=buscar_hilo_por_TID(TID);
		bloquear_hilo(nombre_semaforo,hilo);
	}

	return (bool)value;
}


int signal(char*nombre_semaforo, int TID){

	sem_wait(semaforo_diccionario_por_semaforo);
	int value=dictionary_get(diccionario_de_valor_por_semaforo,nombre_semaforo);
	dictionary_put(diccionario_de_valor_por_semaforo,nombre_semaforo,value-1);
	sem_post(semaforo_diccionario_por_semaforo);
	desbloquear_hilo(nombre_semaforo, TID);



	return (bool)value;
}

void desbloquear_hilo(char* nombre_semaforo, int TID){

	sem_wait(semaforo_estado_blocked);
	t_list* lista_bloqueados =dictionary_get(diccionario_bloqueados_por_semafaro,nombre_semaforo);
	sem_post(semaforo_estado_blocked);
	hilo_t* un_hilo = buscar_hilo_por_TID(TID);
	t_queue* cola_ready = obtener_cola_ready_de(un_hilo->PID);
	queue_push(cola_ready,list_take(lista_bloqueados,1));
	// TODO revisar si el get y el take me genera efecto de lado

}





