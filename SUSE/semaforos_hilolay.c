#include "semaforos_hilolay.h"

void bloquear_hilo( hilo_t* hilo){

	sem_wait(&mutex_lista_bloqueados);
	list_add(bloqueados,hilo);
	hilo->estado_del_hilo = BLOCKED;
	sem_post(&mutex_lista_bloqueados);

	long milisegundos_blocked= time(NULL);
	sem_wait(&mutex_cronometro);
	hilo->tiempos->sumatoria_tiempos_en_EXEC += (milisegundos_blocked - hilo->tiempos->tiempo_llegada_a_EXEC);
	hilo->tiempos->tiempo_en_ejecucion_real = milisegundos_blocked - hilo->tiempos->tiempo_llegada_a_EXEC;
	sem_post(&mutex_cronometro);
}

int wait(char*nombre_semaforo,int TID,int PID){
	bool flag=dictionary_has_key(diccionario_de_valor_por_semaforo,nombre_semaforo);

	sem_wait(&semaforo_diccionario_por_semaforo);
	valores_semaforo_t* value=dictionary_get(diccionario_de_valor_por_semaforo,nombre_semaforo);
	sem_post(&semaforo_diccionario_por_semaforo);

	if(flag){
		if(value->valor_actual>0){
			value->valor_actual-=1;
			sem_wait(&semaforo_diccionario_por_semaforo);
			dictionary_put(diccionario_de_valor_por_semaforo,nombre_semaforo,value);
			sem_post(&semaforo_diccionario_por_semaforo);

		}else{
			proceso_t*un_proceso=obtener_proceso(PID);
			char* pid=string_itoa(PID);
			t_queue* cola_exec=obtener_cola_exec_de(pid);

			bool igual_tid(hilo_t* un_hilo){
				return un_hilo->hilo_informacion->tid==TID;
			}

			sem_wait(&un_proceso->mutex_cola_exec);
			hilo_t* hilo=list_remove_by_condition(cola_exec->elements,igual_tid);
			sem_post(&un_proceso->mutex_cola_exec);

			sem_post(&un_proceso->proceso_en_exec);

			sem_wait(&semaforo_diccionario_procesos_x_semaforo);
			t_list* lista_bloqueados = dictionary_get(diccionario_bloqueados_por_semafaro,nombre_semaforo);
			sem_post(&semaforo_diccionario_procesos_x_semaforo);

			sem_wait(&mutex_bloquear_hilo);
			list_add(lista_bloqueados,hilo->hilo_informacion->tid);
			bloquear_hilo(hilo);
			sem_post(&mutex_bloquear_hilo);


		}
	}
	return (int)flag;
}


int signal(char*nombre_semaforo, int PID){
	int retorno=-1;
	bool flag=dictionary_has_key(diccionario_de_valor_por_semaforo,nombre_semaforo);

	sem_wait(&semaforo_diccionario_por_semaforo);
	valores_semaforo_t *value=dictionary_get(diccionario_de_valor_por_semaforo,nombre_semaforo);
	sem_post(&semaforo_diccionario_por_semaforo);
	if(flag){

		sem_wait(&semaforo_diccionario_procesos_x_semaforo);
		t_list* lista_bloqueados =dictionary_get(diccionario_bloqueados_por_semafaro,nombre_semaforo);
		sem_post(&semaforo_diccionario_procesos_x_semaforo);
		if(value->valor_actual==0 && !list_is_empty(lista_bloqueados)){
			int tid=desbloquear_hilo(lista_bloqueados);

		}else{

			retorno= value->valor_actual;

			if(value->valor_actual<value->valor_max){
				value->valor_actual++;
				sem_wait(&semaforo_diccionario_por_semaforo);
				dictionary_put(diccionario_de_valor_por_semaforo,nombre_semaforo,value);
				sem_post(&semaforo_diccionario_por_semaforo);
			}

		}sem_post(&semaforo_estado_blocked);

	}



	return retorno;
}
int desbloquear_hilo(t_list* lista_de_tids){



	int tid= list_remove(lista_de_tids,0);

	bool remover_tid(hilo_t* hilo){

		return hilo->hilo_informacion->tid==tid;
	}
	sem_wait(&mutex_lista_bloqueados);
	hilo_t* un_hilo=list_remove_by_condition(bloqueados,remover_tid);
	sem_post(&mutex_lista_bloqueados);


	int retorno=-1;
	char*pid=string_itoa(un_hilo->PID);
	proceso_t* un_proceso=obtener_proceso(un_hilo->PID);

	if(un_hilo!=NULL){
		t_queue* cola_ready = obtener_cola_ready_de(pid);
		sem_wait(&un_proceso->mutex_cola_ready);
		queue_push(cola_ready,un_hilo);
		un_hilo->estado_del_hilo=READY;
		sem_post(&un_proceso->mutex_cola_ready);
		sem_post(&(un_proceso->procesos_en_ready));
		retorno=un_hilo->hilo_informacion->tid;
	}
	free(pid);
	return retorno;
}





