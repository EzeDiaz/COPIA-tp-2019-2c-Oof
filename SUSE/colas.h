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
void * estadoReady(int );
void newToReady();
void * estadoNew();
void exit_thread(hilo_t*);
bool esta_vacia(t_queue* );
void encolar_en_new(hilo_t*);
//Flags
bool finDePlanificacion;//inicia en false en el main

//Lists
t_list* hilos_finalizados;//inicializar en el main
#endif /* COLAS_H_ */
