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
#include "globales.h"


enum {
	HILOLAY_INIT = 0,
	SUSE_CREATE = 1,
	SUSE_SCHEDULER_NEXT = 2,
	SUSE_WAIT = 3,
	SUSE_SIGNAL = 4,
	SUSE_JOIN = 5,
	SUSE_CLOSE = 6,
};

enum{

	BOOLEAN=0,
	INT = 1,
	CHAR = 2,
	LONG = 3,
};

void enviar_resultado(void*,int);
void* serializar_bool(bool);
void* armar_paquete(void* , int );
void* descifrar_hilolay_init(void*);
//void* hilolay_init(void*);
int determinar_protocolo(void* );
void identificar_paquete_y_ejecutar_comando(int, void*);

int descifrar_suse_create(void*);
//void* suse_create(hilolay_t*, hilolay_attr_t*, (void*), void* );


char* descifrar_suse_wait(void*);

char* descifrar_suse_signal(void*);

int descifrar_suse_join(void*);


int descifrar_suse_close(void*);


#endif /* DESERIALIZAR_H_ */

