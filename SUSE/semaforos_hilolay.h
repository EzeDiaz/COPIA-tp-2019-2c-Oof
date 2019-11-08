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

//DICCIONARIOS
t_dictionary* diccionario_de_valor_por_semaforo;

//PROTOTIPOS
bool wait(char*,int);
int signal(char*,int);
void bloquear_hilo(char*, int);
void desbloquear_hilo(char* , int );
#endif /* SEMAFOROS_HILOLAY_H_ */
