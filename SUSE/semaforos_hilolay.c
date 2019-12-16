#include "semaforos_hilolay.h"
//VERSION 2


void meter_en_block( hilo_t* hilo){

	proceso_t* un_proceso = obtener_proceso(hilo->PID);
	sem_wait(&hilo->mutex_estado_del_hilo);
	hilo->estado_del_hilo = BLOCKED;
	sem_post(&hilo->mutex_estado_del_hilo);

	sem_wait(&mutex_lista_bloqueados);
	list_add(bloqueados,hilo);
	sem_post(&mutex_lista_bloqueados);

	sem_wait(&hilo->mutex_tiempo_exec);
	sem_wait(&hilo->mutex_sumatoria_exec);
	long milisegundos_blocked= time(NULL);
	hilo->tiempos->sumatoria_tiempos_en_EXEC += (milisegundos_blocked - hilo->tiempos->tiempo_llegada_a_EXEC);
	hilo->tiempos->tiempo_en_ejecucion_real = milisegundos_blocked - hilo->tiempos->tiempo_llegada_a_EXEC;
	sem_post(&hilo->mutex_sumatoria_exec);
	sem_post(&hilo->mutex_tiempo_exec);

}

int wait(char*nombre_semaforo,int TID_a_bloquear,int PID){
	sem_wait(&semaforo_diccionario_por_semaforo);
	bool existe_la_key_en_el_diccionario=dictionary_has_key(diccionario_de_valor_por_semaforo,nombre_semaforo);
	sem_post(&semaforo_diccionario_por_semaforo);
	if(existe_la_key_en_el_diccionario){
		sem_wait(&semaforo_diccionario_por_semaforo);
		valores_semaforo_t* value=dictionary_get(diccionario_de_valor_por_semaforo,nombre_semaforo);
		bool el_valor_es_mayor_a_cero=value->valor_actual>0;
		sem_post(&semaforo_diccionario_por_semaforo);

		if(el_valor_es_mayor_a_cero){
			decrementar_semaforo(value,nombre_semaforo);
		}else{
			bloquear(TID_a_bloquear,PID,nombre_semaforo);
		}
	}
	return (int)existe_la_key_en_el_diccionario;
}


int signal(char*nombre_semaforo, int PID){
	int retorno=-1;

	sem_wait(&semaforo_diccionario_por_semaforo);
	bool existe_la_key_en_el_diccionario=dictionary_has_key(diccionario_de_valor_por_semaforo,nombre_semaforo);
	sem_post(&semaforo_diccionario_por_semaforo);

	if(existe_la_key_en_el_diccionario){

		sem_wait(&semaforo_diccionario_por_semaforo);
		valores_semaforo_t *value=dictionary_get(diccionario_de_valor_por_semaforo,nombre_semaforo);
		bool maximo_valor_semaforo_flag=value->valor_actual<value->valor_max;
		bool valor_igual_cero=value->valor_actual == 0 ;
		sem_post(&semaforo_diccionario_por_semaforo);

		sem_wait(&semaforo_diccionario_procesos_x_semaforo);
		t_list* lista_bloqueados =dictionary_get(diccionario_bloqueados_por_semafaro,nombre_semaforo);
		sem_post(&semaforo_diccionario_procesos_x_semaforo);

		if(valor_igual_cero && !list_is_empty(lista_bloqueados)){

			hilo_bloqueado_t* un_bloqueado= list_remove(lista_bloqueados,0);
			sem_wait(&mutex_desbloquear_hilo);
			desbloquear_hilo(un_bloqueado->TID,un_bloqueado->PID);
			sem_post(&mutex_desbloquear_hilo);

			free(un_bloqueado);
		}else{
			if(maximo_valor_semaforo_flag){

				sem_wait(&semaforo_diccionario_por_semaforo);
				value->valor_actual++;
				memcpy(&retorno, &value->valor_actual,sizeof(int));
				dictionary_put(diccionario_de_valor_por_semaforo,nombre_semaforo,value);
				sem_post(&semaforo_diccionario_por_semaforo);
			}
		}

	}

	return retorno;
}

int desbloquear_hilo(int tid,int PID){

	bool remover_tid(hilo_t* hilo_a_desbloquear){

		bool tid_iguales=hilo_a_desbloquear->hilo_informacion->tid==tid;
		bool pid_iguales=hilo_a_desbloquear->PID==PID;
		return (tid_iguales&&pid_iguales);
	}
	sem_wait(&mutex_lista_bloqueados);
	hilo_t* un_hilo=list_remove_by_condition(bloqueados,remover_tid);
	sem_post(&mutex_lista_bloqueados);


	int retorno=-1;
	char*pid=string_itoa(un_hilo->PID);
	proceso_t* un_proceso=obtener_proceso(un_hilo->PID);

	if(un_hilo!=NULL){
		t_queue* cola_ready = obtener_cola_ready_de(pid);

		sem_wait(&un_hilo->mutex_estado_del_hilo);
		un_hilo->estado_del_hilo=READY;
		sem_post(&un_hilo->mutex_estado_del_hilo);

		sem_wait(&un_proceso->mutex_cola_ready);
		long milisegundos_ready= time(NULL);
		un_hilo->tiempos->tiempo_llegada_a_READY = milisegundos_ready;

		queue_push(cola_ready,un_hilo);
		sem_post(&un_proceso->mutex_cola_ready);
		sem_post(&(un_proceso->procesos_en_ready));
		retorno=un_hilo->hilo_informacion->tid;
	}
	free(pid);
	return retorno;
}



void decrementar_semaforo(valores_semaforo_t*value,char* nombre_semaforo){
	sem_wait(&semaforo_diccionario_por_semaforo);
	value->valor_actual-=1;
	dictionary_put(diccionario_de_valor_por_semaforo,nombre_semaforo,value);
	sem_post(&semaforo_diccionario_por_semaforo);
}



void bloquear(int TID_a_bloquear,int PID,char* nombre_semaforo){
	proceso_t*un_proceso=obtener_proceso(PID);
	hilo_t* un_hilo=buscar_hilo_por_TID(TID_a_bloquear,PID);

	sem_wait(&un_hilo->mutex_estado_del_hilo);
	bool el_hilo_esta_en_exec= un_hilo->estado_del_hilo==EXECUTE;
	sem_post(&un_hilo->mutex_estado_del_hilo);

	if(el_hilo_esta_en_exec){
		char* pid=string_itoa(PID);
		hilo_t* hilo=obtener_hilo_a_bloquear(un_proceso,TID_a_bloquear,pid);
		bloquear_hilo_por_semaforo(hilo,nombre_semaforo);
		free(pid);
	}
}


hilo_t* obtener_hilo_a_bloquear(proceso_t* un_proceso,int TID_a_bloquear,char* pid){


	t_queue* cola_exec=obtener_cola_exec_de(pid);

	bool igual_tid(hilo_t* un_hilo){
		return un_hilo->hilo_informacion->tid==TID_a_bloquear;
	}
	sem_wait(&un_proceso->consumir_hilo_exec);
	sem_wait(&un_proceso->mutex_cola_exec);
	hilo_t* hilo=list_remove_by_condition(cola_exec->elements,igual_tid);
	sem_post(&un_proceso->mutex_cola_exec);
	sem_post(&un_proceso->proceso_en_exec);

	return hilo;

}



void bloquear_hilo_por_semaforo(hilo_t* hilo,char* nombre_semaforo){

	sem_wait(&semaforo_diccionario_procesos_x_semaforo);
	t_list* lista_bloqueados = dictionary_get(diccionario_bloqueados_por_semafaro,nombre_semaforo);
	sem_post(&semaforo_diccionario_procesos_x_semaforo);

	hilo_bloqueado_t* un_bloqueado=(hilo_bloqueado_t*)malloc(sizeof(hilo_bloqueado_t));
	un_bloqueado->PID=hilo->PID;
	un_bloqueado->TID=hilo->hilo_informacion->tid;
	sem_wait(&mutex_bloquear_hilo);
	meter_en_block(hilo);
	sem_post(&mutex_bloquear_hilo);

	sem_wait(&semaforo_diccionario_procesos_x_semaforo);
	list_add(lista_bloqueados,un_bloqueado);
	dictionary_put(diccionario_bloqueados_por_semafaro,nombre_semaforo,lista_bloqueados);
	sem_post(&semaforo_diccionario_procesos_x_semaforo);


}
