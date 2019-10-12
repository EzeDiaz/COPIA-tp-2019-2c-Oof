/*
 * colas.h
 *
 *  Created on: 1 oct. 2019
 *      Author: utnso
 */

#ifndef COLAS_H_
#define COLAS_H_

#include <commons/temporal.h>
#include "funciones_aux.h"
#include "TADs.h"
//hilolay falta


//Prototipos
void newToReady();

//Flags
bool finDePlanificacion;//inicia en false en el main

//Lists
t_list* hilos_finalizados;//inicializar en el main
#endif /* COLAS_H_ */
