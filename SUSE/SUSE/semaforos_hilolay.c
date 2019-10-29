/*
 * semaforos_hilolay.c
 *
 *  Created on: 28 oct. 2019
 *      Author: utnso
 */

#include "semaforos_hilolay.h"
#include "TADs.h"


int wait(char*nombre_semaforo,int TID){

	int value=dictionary_get(diccionario_de_valor_por_semaforo,nombre_semaforo);
	if(value){
	dictionary_put(diccionario_de_valor_por_semaforo,nombre_semaforo,value-1);
	}else{

		hilo_t* hilo=buscar_hilo_por_TID(TID);
		queue_push(cola_blocked,hilo);
	}

	return (bool)value;
}


int signal(char*nombre_semaforo, int TID){

	int value=dictionary_get(diccionario_de_valor_por_semaforo,nombre_semaforo);
	dictionary_put(diccionario_de_valor_por_semaforo,nombre_semaforo,value-1);
	blocked_to_ready();



	return (bool)value;
}






