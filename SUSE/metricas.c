/*
 * metricas.c
 *
 *  Created on: 18 sep. 2019
 *      Author: utnso
 */

#include "metricas.h"
#include <commons/string.h>
#include <commons/log.h>
#include <string.h>



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

void metricas_por_sistema(){//Son mas de 1 semaforo TODO

	int grado_multiprogramacion;
	sem_getvalue(&grado_de_multiprogramacion_contador,&grado_multiprogramacion);
	log_info(log_metricas_sistema,"grado de programacion actual: %d \n",grado_multiprogramacion);

	void loguear_semaforo(char* nombre_del_semaforo, valores_semaforo_t* valor){
		log_info(log_metricas_sistema,"el semaforo %s",nombre_del_semaforo);
		log_info(log_metricas_sistema," tiene un valor de %d \n",valor->valor_actual);
	}

	dictionary_iterator(diccionario_de_valor_por_semaforo,loguear_semaforo);
}

void metricas_por_programa(){

	void loguear_metricas_de_proceso(char* PID_string, proceso_t* un_proceso){
		metricas_de_hilos_por_estado(PID_string,un_proceso);
	}
	dictionary_iterator(diccionario_de_procesos,loguear_metricas_de_proceso);
}


void metricas_de_hilos_por_estado(char* PID,proceso_t* un_proceso){
	int acumulador_hilos;
	log_info(log_metricas_programa,"El programa con PID %s tiene los siguientes hilos en estos estados: \n",PID);
	acumulador_hilos= hilos_en_ready(un_proceso->hilos_del_programa,NEW);
	log_info(log_metricas_programa,"NEW---> %d hilos\n",acumulador_hilos);
	acumulador_hilos= hilos_en_ready(un_proceso->hilos_del_programa,READY);
	log_info(log_metricas_programa,"READY---> %d hilos\n",acumulador_hilos);
	acumulador_hilos= hilos_en_ready(un_proceso->hilos_del_programa,EXECUTE);
	log_info(log_metricas_programa,"EXECUTE---> %d hilos\n",acumulador_hilos);
	acumulador_hilos= hilos_en_ready(un_proceso->hilos_del_programa,BLOCKED);
	log_info(log_metricas_programa,"BLOCKED---> %d hilos\n",acumulador_hilos);

}

int hilos_en_ready(t_list* hilos, int estado){

	bool hilos_en_ready(hilo_t* un_hilo){
		return un_hilo->estado_del_hilo==estado;
	}
	return list_count_satisfying(hilos,hilos_en_ready);
}

void metricas_por_hilo(){

	void por_cada_proceso(char* PID_string, proceso_t* un_proceso){
		int tiempo_total_de_ejecucion= 0;

		void calcular_tiempo(hilo_t* un_hilo){
		tiempo_total_de_ejecucion+=	un_hilo->metricas->tiempo_de_ejecucion;

		}
		list_iterate(un_proceso->hilos_del_programa,calcular_tiempo);
		void loguear_hilo(hilo_t* un_hilo){
			un_hilo->metricas->porcentaje_total_tiempo_de_ejecucion_de_hilos=un_hilo->metricas->tiempo_de_uso_del_cpu/tiempo_total_de_ejecucion;
			log_info(log_metricas_hilo,"Al hilo %d le corresponden las siguientes metricas: \n",un_hilo->hilo_informacion->tid);
			log_info(log_metricas_hilo,"TIEMPO DE EJECUCION--> %d ms\n",un_hilo->metricas->tiempo_de_ejecucion);
			log_info(log_metricas_hilo,"TIEMPO DE ESPERA--> %d ms\n",un_hilo->metricas->tiempo_de_espera);
			log_info(log_metricas_hilo,"TIEMPO DE USO DE CPU--> %d ms\n",un_hilo->metricas->tiempo_de_uso_del_cpu);
			log_info(log_metricas_hilo,"PORCENTAJE DE TIEMPO DE EJECUCION--> %d ms\n",un_hilo->metricas->porcentaje_total_tiempo_de_ejecucion_de_hilos);

		}
		list_iterate(un_proceso->hilos_del_programa,loguear_hilo);
	}
	dictionary_iterator(diccionario_de_procesos,por_cada_proceso);
}



