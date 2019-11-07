/*
 * deserializar.h
 *
 *  Created on: 2 nov. 2019
 *      Author: utnso
 */

#ifndef DESERIALIZAR_H_
#define DESERIALIZAR_H_
#include <commons/log.h>
#include <stdbool.h>
#include <string.h>
#include "SUSE.h"
#include "colas.h"
#include <sys/socket.h>
#include <sys/types.h>
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


void enviar_resultado(void*,int);
void* serializar_bool(bool);
void* descifrar_hilolay_init(void*);
//void* hilolay_init(void*);

void* descifrar_suse_create(void*);
//void* suse_create(hilolay_t*, hilolay_attr_t*, (void*), void* );


char* descifrar_suse_wait(void*);

char* descifrar_suse_signal(void*);

int descifrar_suse_join(void*);


int descifrar_suse_close(void*);


#endif /* DESERIALIZAR_H_ */

