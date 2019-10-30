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
	t_list* lista_bloqueados = diccionary_get(diccionario_bloqueados_por_semafaro,nombre_semaforo);
	list_add(lista_bloqueados,hilo);

}

int wait(char*nombre_semaforo,int TID){

	int value=dictionary_get(diccionario_de_valor_por_semaforo,nombre_semaforo);
	if(value){
		dictionary_put(diccionario_de_valor_por_semaforo,nombre_semaforo,value-1);
	}else{
		hilo_t* hilo=buscar_hilo_por_TID(TID);
		bloquear_hilo(nombre_semaforo,hilo);
	}

	return (bool)value;
}


int signal(char*nombre_semaforo, int TID){

	int value=dictionary_get(diccionario_de_valor_por_semaforo,nombre_semaforo);
	dictionary_put(diccionario_de_valor_por_semaforo,nombre_semaforo,value-1);
	desbloquear_hilo(nombre_semaforo, TID);



	return (bool)value;
}

void desbloquear_hilo(char* nombre_semaforo, int TID){

	t_list* lista_bloqueados =diccionary_get(diccionario_hilos_bloqueados_por_semaforo,nombre_semaforo);
	hilo_t* un_hilo = buscar_hilo_por_TID(TID);
	t_queue* cola_ready = obtener_cola_ready_de(un_hilo->PID);
	queue_push(cola_ready,list_take(lista_bloqueados,1));
	// TODO revisar si el get y el take me genera efecto de lado

}





