#include "metricas.h"

/*
		• Por programa: grado actual de multiprogramación y cantidad de hilos en cada estado
		• Por sistema: cada semáforo con su valor actual
		• Por hilo:
				- Tiempo de ejecución: intervalo en [ms] desde creación hasta que se toma la métrica. (El hilo
				va a tener TS de inicio y genero un TS para la métrica para luego restarlos)

				- Tiempo de espera: es la suma de todas las veces que estás en READY (Genero un TS cada vez
				que entras y salis de READY y lo vas acumulando)

				- Tiempo de uso de CPU: Idem anterior pero con EXEC

				- Porcentaje del tiempo de ejecución: tiempo de uso de CPU del hilo. (Es la sumatoria de
				todos los tiempos de uso de CPU de todos los hilos del mismo proceso)*/



void mostrar_metricas_cada_delta_t(){
	while(1){
		sleep(METRICS_TIMER);
		mostrar_metricas();
	}

}

void mostrar_metricas(){

	metricas_por_sistema();
	metricas_por_programa();
	metricas_por_hilo();

}

int cantidad_hilos(proceso_t* un_proceso){
	int cantidad_hilos = 0;
	cantidad_hilos += cantidad_hilos_en_el_estado(un_proceso,NEW);
	cantidad_hilos += cantidad_hilos_en_el_estado(un_proceso,READY);
	cantidad_hilos += cantidad_hilos_en_el_estado(un_proceso,EXECUTE);
	cantidad_hilos += cantidad_hilos_en_el_estado(un_proceso,BLOCKED);
	return cantidad_hilos;
}

void metricas_por_sistema(){//Son mas de 1 semaforo TODO
	int hilos_totales;
	int contar_hilos(char* PID_string, proceso_t* un_proceso){
		return hilos_totales = cantidad_hilos(un_proceso);
	}
	sem_wait(&mutex_diccionario);
	dictionary_iterator(diccionario_de_procesos,contar_hilos);
	sem_post(&mutex_diccionario);

	/*int grado_multiprogramacion;
	sem_getvalue(&grado_de_multiprogramacion_contador,&grado_multiprogramacion);
	*/sem_wait(&mutex_log_metricas);
	int printeo=MAX_MULTIPROG-hilos_totales;
	log_info(log_metricas_sistema,"grado de programacion actual: %d \n",printeo);
	sem_post(&mutex_log_metricas);

	void loguear_semaforo(char* nombre_del_semaforo, valores_semaforo_t* valor){
		sem_wait(&mutex_log_metricas);
		log_info(log_metricas_sistema,"el semaforo %s",nombre_del_semaforo);
		log_info(log_metricas_sistema," tiene un valor de %d \n",valor->valor_actual);
		sem_post(&mutex_log_metricas);
	}
	sem_wait(&semaforo_diccionario_por_semaforo);
	dictionary_iterator(diccionario_de_valor_por_semaforo,loguear_semaforo);
	sem_post(&semaforo_diccionario_por_semaforo);
}

void metricas_por_programa(){

	void loguear_metricas_de_proceso(char* PID_string, proceso_t* un_proceso){
		metricas_de_hilos_por_estado(PID_string,un_proceso);
	}
	sem_wait(&mutex_diccionario);
	dictionary_iterator(diccionario_de_procesos,loguear_metricas_de_proceso);
	sem_post(&mutex_diccionario);
}


void metricas_de_hilos_por_estado(char* PID,proceso_t* un_proceso){
	int acumulador_hilos;

	sem_wait(&mutex_log_metricas);
	log_info(log_metricas_programa,"El programa con PID %s tiene los siguientes hilos en estos estados: \n",PID);
	sem_post(&mutex_log_metricas);

	acumulador_hilos= cantidad_hilos_en_el_estado(un_proceso,NEW);
	sem_wait(&mutex_log_metricas);
	log_info(log_metricas_programa,"NEW---> %d hilos\n",acumulador_hilos);
	sem_post(&mutex_log_metricas);

	acumulador_hilos= cantidad_hilos_en_el_estado(un_proceso,READY);
	sem_wait(&mutex_log_metricas);
	log_info(log_metricas_programa,"READY---> %d hilos\n",acumulador_hilos);
	sem_post(&mutex_log_metricas);

	acumulador_hilos= cantidad_hilos_en_el_estado(un_proceso,EXECUTE);
	sem_wait(&mutex_log_metricas);
	log_info(log_metricas_programa,"EXECUTE---> %d hilos\n",acumulador_hilos);
	sem_post(&mutex_log_metricas);

	acumulador_hilos= cantidad_hilos_en_el_estado(un_proceso,BLOCKED);
	sem_wait(&mutex_log_metricas);
	log_info(log_metricas_programa,"BLOCKED---> %d hilos\n",acumulador_hilos);
	sem_post(&mutex_log_metricas);

}

int cantidad_hilos_en_el_estado(proceso_t* un_proceso, int estado){

	int cantidad_elementos;

	bool esta_en_dicho_estado(hilo_t* un_hilo){

		sem_wait(&un_hilo->mutex_estado_del_hilo);
		bool retorno = un_hilo->estado_del_hilo==estado;
		sem_post(&un_hilo->mutex_estado_del_hilo);

		return retorno;
	}

	sem_wait(&un_proceso->mutex_lista_hilos);
	cantidad_elementos = list_count_satisfying(un_proceso->hilos_del_programa,esta_en_dicho_estado);
	sem_post(&un_proceso->mutex_lista_hilos);

	return cantidad_elementos;
}

void metricas_por_hilo(){

	long milisegundos_metricas= time(NULL);

	void loguear_metricas(char*PID, proceso_t* un_proceso){





		void obtener_tiempo_ejecucion(hilo_t* un_hilo){
			un_hilo->metricas->tiempo_de_ejecucion = milisegundos_metricas - un_hilo->tiempos->tiempo_llegada_a_NEW;
			sem_wait(&un_hilo->mutex_sumatoria_ready);
			un_hilo->metricas->tiempo_de_espera = un_hilo->tiempos->sumatoria_tiempos_en_READY;
			sem_post(&un_hilo->mutex_sumatoria_ready);

			sem_wait(&un_hilo->mutex_sumatoria_exec);
			un_hilo->metricas->tiempo_de_uso_del_cpu = un_hilo->tiempos->sumatoria_tiempos_en_EXEC;
			sem_post(&un_hilo->mutex_sumatoria_exec);

			int tiempo_total=obtener_tiempo_total_de(un_proceso);
			int tiempo_ejecucion_del_hilo = un_hilo->metricas->tiempo_de_ejecucion;
			if(tiempo_total!=0){
				un_hilo->metricas->porcentaje_total_tiempo_de_ejecucion_de_hilos = ((tiempo_ejecucion_del_hilo *100) / tiempo_total);
			}else{
				un_hilo->metricas->porcentaje_total_tiempo_de_ejecucion_de_hilos = 57;
			}

			sem_wait(&mutex_log_metricas);
			log_info(log_metricas_hilo,"Al hilo %d del proceso %d le corresponden las siguientes metricas: \n",un_hilo->hilo_informacion->tid,un_hilo->PID);
			log_info(log_metricas_hilo,"TIEMPO DE EJECUCION--> %d ms\n",un_hilo->metricas->tiempo_de_ejecucion);
			log_info(log_metricas_hilo,"TIEMPO DE ESPERA--> %d ms\n",un_hilo->metricas->tiempo_de_espera);
			log_info(log_metricas_hilo,"TIEMPO DE USO DE CPU--> %d ms\n",un_hilo->metricas->tiempo_de_uso_del_cpu);
			log_info(log_metricas_hilo,"PORCENTAJE DE TIEMPO DE EJECUCION--> %f %\n",un_hilo->metricas->porcentaje_total_tiempo_de_ejecucion_de_hilos);
			sem_post(&mutex_log_metricas);

		}
		//sem_wait(&un_proceso->mutex_lista_hilos);
		list_iterate(un_proceso->hilos_del_programa, obtener_tiempo_ejecucion);
		//sem_post(&un_proceso->mutex_lista_hilos);

	}
	sem_wait(&mutex_diccionario);
	dictionary_iterator(diccionario_de_procesos,loguear_metricas);
	sem_post(&mutex_diccionario);
}

int obtener_tiempo_total_de(proceso_t* un_proceso){

	int tiempo_total=0;
	sem_wait(&un_proceso->mutex_lista_hilos);
	int cantidad_de_hilos = un_proceso->hilos_del_programa->elements_count;
	sem_post(&un_proceso->mutex_lista_hilos);
	hilo_t* un_hilo;


	for(int i=0;i<cantidad_de_hilos;i++){

		sem_wait(&mutex_obtener_tiempo_total);
		sem_wait(&un_proceso->mutex_lista_hilos);
		un_hilo = list_get(un_proceso->hilos_del_programa,i);
		sem_post(&un_proceso->mutex_lista_hilos);
		tiempo_total+=un_hilo->metricas->tiempo_de_ejecucion;
		sem_post(&mutex_obtener_tiempo_total);

	}
	return tiempo_total;
}
