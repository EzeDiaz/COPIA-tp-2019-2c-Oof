/*
 * SUSE.h
 *
 *  Created on: 6 nov. 2019
 *      Author: utnso
 */

#ifndef SUSE_H_
#define SUSE_H_

//prototipos

/*
 * SUSE.c
 *
 *  Created on: 18 sep. 2019
 *      Author: utnso
 */

#include "funciones_aux.h"
#include "metricas.h"
#include "colas.h"
#include "globales.h"
#include "funciones_aux.h"
#include <unistd.h>
#include <pthread.h>
#include <hilolay/hilolay.h>
#include <semaphore.h>
#include <stdint.h>
#include <sys/types.h>
#include <commons/collections/dictionary.h>
#include <commons/collections/list.h>
#include <commons/string.h>

#define ATTR_C11_THREAD ((void*)(uintptr_t)-1)
void* suse_close(int);
void* suse_join(int);
void* suse_signal(char* semaforo, int PID);
hilo_t* suse_scheduler_next(int);
bool suse_wait(char*, int );
int suse_create(hilolay_t *, const hilolay_attr_t *,void *(*start_routine)(void *), void *, int);
proceso_t* obtener_proceso(int);
proceso_t* inicializar_proceso(int);
void agregar_al_diccionario(int , sem_t*);


#endif /* SUSE_H_ */
