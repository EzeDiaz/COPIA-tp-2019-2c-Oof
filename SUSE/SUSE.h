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
void suse_init();
void* suse_close(int);
void* suse_join(int);
void* suse_signal(char* semaforo, int PID);
hilo_t* suse_schedule_next(int);
bool suse_wait(char*, int );
int suse_create(int, int);
proceso_t* obtener_proceso(int);
proceso_t* inicializar_proceso(int);
void agregar_al_diccionario(int , sem_t*);
void leer_config();
void crear_hilo(hilo_t*, void*);
void liberar_recursos();
int _hilolay_init(int);

typedef struct {
	char* IP;
	char * LISTEN_PORT;
	int METRICS_TIMER;
	int MAX_MULTIPROG;
	char** SEM_IDS;
	char** SEM_INIT;
	char** SEM_MAX;
	float ALPHA_SJF;

}structConfig;

void leer_config2();


#endif /* SUSE_H_ */
