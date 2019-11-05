/*
 * deserializar.h
 *
 *  Created on: 2 nov. 2019
 *      Author: utnso
 */

#ifndef DESERIALIZAR_H_
#define DESERIALIZAR_H_
#include <commons/log.h>

t_log* logger_de_deserializacion;

enum {
	HILOLAY_INIT = 0,
	SUSE_CREATE = 1,
	SUSE_SCHEDULER_NEXT = 2,
	SUSE_WAIT = 3,
	SUSE_SIGNAL = 4,
	SUSE_JOIN = 5,
	SUSE_CLOSE = 6,
};

typedef struct{
	char* nombre_del_semaforo;
	int tid;
}semaforo_descifrado_t;

void serializar_y_enviar_resultado(void*,int);

void* descifrar_hilolay_init(void*);
//void* hilolay_init(void*);

void* descifrar_suse_create(void*);
//void* suse_create(hilolay_t*, hilolay_attr_t*, (void*), void* );

void* descifrar_suse_scheduler_next(void*);
void* suse_scheduler_next(void*);

void* descifrar_suse_wait(void*);
void* suse_wait(semaforo_descifrado_t*);

void* descifrar_suse_signal(void*);
void* suse_signal(semaforo_descifrado_t*);

void* descifrar_suse_join(void*);
void* suse_join(void*);

int descifrar_suse_close(void*);
void* suse_close(int);

#endif /* DESERIALIZAR_H_ */

