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
#include <commons/collections/dictionary.h>

//DICCIONARIOS
t_dictionary* diccionario_de_valor_por_semaforo;
t_dictionary* diccionario_hilos_bloqueados_por_semaforo;

//PROTOTIPOS
int wait(char*,int);
int signal(char*,int);
void bloquear_hilo(char*, hilo_t*);

#endif /* SEMAFOROS_HILOLAY_H_ */
