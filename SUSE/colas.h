#ifndef COLAS_H_
#define COLAS_H_

#include <commons/temporal.h>
#include "funciones_aux.h"
#include "TADs.h"
//hilolay falta


//Prototipos

//TRANSICIONES
void * estadoReady(int );
void readyToExec(int);
void newToReady();
void estadoNew();
void exit_thread(hilo_t*);


//auxiliares
bool esta_vacia(t_queue* );
void encolar_en_new(hilo_t*);


//Flags
bool finDePlanificacion;//inicia en false en el main

//Lists
#endif /* COLAS_H_ */
