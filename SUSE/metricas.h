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
#include "semaforos_hilolay.h"

/*PROTOTIPOS*/
void metrica_por_sistema();
void metrica_por_programa(proceso_t*);
void metrica_por_cantidad_de_hilos(proceso_t*);
void metrica_por_grado_actual_de_multiprogramacion(proceso_t*);
void loguear_mensaje(t_log*,char*);

#endif /* METRICAS_H_ */
