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

void metrica_por_sistema(t_list* varios_semaforos){//Son mas de 1 semaforo TODO

	void loguear_valores(char* un_semaforo){

		int valor_string_id=string_length(un_semaforo);

		int valor_actual=dictionary_get(diccionario_de_valor_por_semaforo,un_semaforo);
		int valor_string_semaforo=string_length(string_itoa(valor_actual));
		char* msj=(char*)malloc(32+valor_string_id+valor_string_semaforo);
		msj=strcat("el semaforo ",string_itoa(un_semaforo));
		msj=strcat(" tiene un valor de ",string_itoa(valor_actual));
		loguear_mensaje(log_metricas_sistema,msj);
		free(msj);
	}
	list_iterate(varios_semaforos,loguear_valores);
	//hay que agarrar todos los semaforos porque se pregunta cada cierto tiempo el valor de todos?


}
void metrica_por_programa(proceso_t *un_programa){

	metrica_por_cantidad_de_hilos(un_programa);
	metrica_por_grado_actual_de_multiprogramacion(un_programa);
}

void metrica_por_cantidad_de_hilos(proceso_t* un_proceso){




	char* msj=(char*)malloc(43+string_length(string_itoa(un_proceso->hilos_del_programa->elements_count)));
	msj=strcat("cantidad de hilos del programa",string_itoa(un_proceso->hilos_del_programa->elements_count));

	loguear_mensaje(log_metricas_programa,msj);
	free(msj);

	//TODO no se sabe hasta que este hilolay


}
/*
void destruir_log_sistema(){

	log_destroy(log_metricas_sistema);//destruyo el log y borro el archivo
	remove("log_sistema.log");//		no va a pasar como el tp anterior que tenias un log de 15 MB

}
void incializar_log_sistema(){

	// hacer un remove("log_sistema.log"); TODO

	log_metricas_sistema=log_create("log_sistema.log","log_sistema",0,LOG_LEVEL_INFO);

}
*/
void metrica_por_grado_actual_de_multiprogramacion(proceso_t* un_proceso){

	char* msj=(char*)malloc(43+string_length(string_itoa(un_proceso->hilos_del_programa->elements_count)));
	msj=strcat("se cambio el grado de multiprogramacion a ",string_itoa(un_proceso->hilos_del_programa->elements_count));//potencial SEGFAULT
	loguear_mensaje(log_metricas_programa,msj);
	free(msj);//TODO hacerlo sin sarna

}

void loguear_mensaje(t_log* un_log, char* msj){

	log_info(un_log,msj);


}
