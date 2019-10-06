/*
 * funciones_aux.h
 *
 *  Created on: 6 oct. 2019
 *      Author: utnso
 */

#ifndef FUNCIONES_AUX_H_
#define FUNCIONES_AUX_H_

#include "TADs.h"
#include <commons/collections/queue.h>

//Prototipos
t_queue* obtener_cola_ready_de(int );
t_queue* obtener_cola_exec_de(int);
t_queue* obtener_cola_de(int, int);
float calcular_sjf(hilo_t*);
void destruir_logs_sistema();
void incializar_logs_sistema();
void leer_config_y_setear();





#endif /* FUNCIONES_AUX_H_ */
