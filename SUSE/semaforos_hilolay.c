/*
 * semaforos_hilolay.c
 *
 *  Created on: 28 oct. 2019
 *      Author: utnso
 */

#include "semaforos_hilolay.h"
#include "TADs.h"

void bloquear_hilo(t_list* lista_bloqueados, hilo_t* hilo){

	/*char*pid= string_itoa(PID);
	 * char* nombre_semaforo
	t_queue* cola_exec = obtener_cola_exec_de(pid);
	hilo_t* hilo=queue_pop(cola_exec);*/
	list_add(lista_bloqueados,hilo);
	hilo->estado_del_hilo = BLOCKED;

}

bool wait(char*nombre_semaforo,int PID){
	bool flag=dictionary_has_key(diccionario_de_valor_por_semaforo,nombre_semaforo);

	if(flag){
		sem_wait(&semaforo_diccionario_por_semaforo);
		valores_semaforo_t* value=dictionary_get(diccionario_de_valor_por_semaforo,nombre_semaforo);
		sem_post(&semaforo_diccionario_por_semaforo);
		if(value->valor_actual>0){
			value->valor_actual-=1;
			sem_wait(&semaforo_diccionario_por_semaforo);
			dictionary_put(diccionario_de_valor_por_semaforo,nombre_semaforo,value);
			sem_post(&semaforo_diccionario_por_semaforo);
		}else{
			hilo_t* hilo=queue_pop(obtener_cola_exec_de(PID));
			sem_wait(&semaforo_estado_blocked);
			t_list* lista_bloqueados = dictionary_get(diccionario_bloqueados_por_semafaro,nombre_semaforo);
			bloquear_hilo(lista_bloqueados,hilo);
			sem_post(&semaforo_estado_blocked);

		}
	}
	return flag;
}


int signal(char*nombre_semaforo, int PID){

	bool flag=dictionary_has_key(diccionario_de_valor_por_semaforo,nombre_semaforo);
	if(flag){
		sem_wait(&semaforo_diccionario_por_semaforo);
		valores_semaforo_t *value=dictionary_get(diccionario_de_valor_por_semaforo,nombre_semaforo);
		if(value->valor_actual<value->valor_max)
			value->valor_actual++;

		dictionary_put(diccionario_de_valor_por_semaforo,nombre_semaforo,value);
		sem_post(&semaforo_diccionario_por_semaforo);
		if(value==0){
			sem_wait(&semaforo_estado_blocked);
			t_list* lista_bloqueados =dictionary_get(diccionario_bloqueados_por_semafaro,nombre_semaforo);
			sem_post(&semaforo_estado_blocked);

			desbloquear_hilo(lista_bloqueados, PID);
		}
		return (bool)value;
	}else{
		return -1;
	}
}

void desbloquear_hilo(t_list* lista_bloqueados, int PID){

	char*pid=string_itoa(PID);
	proceso_t* un_proceso=obtener_proceso(PID);
	hilo_t* un_hilo= list_remove(lista_bloqueados,0);
	t_queue* cola_ready = obtener_cola_ready_de(PID);
	queue_push(cola_ready,un_hilo);
	un_hilo->estado_del_hilo=READY;
	sem_post(un_proceso->procesos_en_ready);

}





