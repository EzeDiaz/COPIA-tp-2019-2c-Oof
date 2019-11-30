/*
 * semaforos_hilolay.h
 *
 *  Created on: 28 oct. 2019
 *      Author: utnso
 */

#ifndef SEMAFOROS_HILOLAY_H_
#define SEMAFOROS_HILOLAY_H_
#include "globales.h"
#include "TADs.h"
#include "funciones_aux.h"
#include <commons/collections/dictionary.h>
#include <commons/collections/list.h>

//DICCIONARIOS
t_dictionary* diccionario_de_valor_por_semaforo;

//PROTOTIPOS
int wait(char*,int,int);
int signal(char*,int);
void bloquear_hilo(t_list*, hilo_t*);
int desbloquear_hilo(t_list* , int );
#endif /* SEMAFOROS_HILOLAY_H_ */
