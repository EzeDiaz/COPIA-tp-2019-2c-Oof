#ifndef SUSE_H_
#define SUSE_H_

#include "funciones_aux.h"
#include "metricas.h"
#include "colas.h"
#include "globales.h"
#include "funciones_aux.h"
#include <unistd.h>
#include <pthread.h>
#include <hilolay/hilolay.h>
#include <hilolay/alumnos.h>
#include <semaphore.h>
#include <stdint.h>
#include <sys/types.h>
#include <commons/collections/dictionary.h>
#include <commons/collections/list.h>
#include <commons/string.h>
#include <commons/config.h>
#include <readline/readline.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include "metricas.h"

#define ATTR_C11_THREAD ((void*)(uintptr_t)-1)

void suse_init();
void* suse_close(int,int);
int suse_join(int,int);
int suse_signal(char* , int );
int suse_schedule_next(int);
int suse_wait(char*, int ,int );
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
