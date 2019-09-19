/*
 * metricas.h
 *
 *  Created on: 18 sep. 2019
 *      Author: utnso
 */

#ifndef METRICAS_H_
#define METRICAS_H_

#include <commons/log.h>
#include "TADs.h"
#include <commons/string.h>



/*VARIABLES GLOBALES DE LAS METRICAS*/
t_log* log_metricas_sistema;
t_log* log_metricas_programa;
t_log* log_metricas_hilo;

				/*PROTOTIPOS*/
void incializar_log_sistema();
void metrica_por_sistema(t_list*);
void metrica_por_programa(programa_t*);
void metrica_por_cantidad_de_hilos(programa_t*);
void metrica_por_grado_actual_de_multiprogramacion(programa_t*);
void loguear_mensaje(t_log*,char*);
void destruir_log_sistema();
#endif /* METRICAS_H_ */
