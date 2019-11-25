/*
 * serializar.h
 *
 *  Created on: 7 nov. 2019
 *      Author: utnso
 */

#ifndef SERIALIZAR_H_
#define SERIALIZAR_H_

#include "globales.h"
#include <commons/string.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdint.h>
#include <hilolay/hilolay.h>
#include <readline/readline.h>


enum codigo_de_operacion{
	HILOLAY_INIT = 0,
	SUSE_CREATE = 1,
	SUSE_SCHEDULER_NEXT = 2,
	SUSE_WAIT = 3,
	SUSE_SIGNAL = 4,
	SUSE_JOIN = 5,
	SUSE_CLOSE = 6,
};

void* serializar_suse_close(int);
void* serializar_suse_join(int);
void* serializar_suse_signal(char*);
void* serializar_suse_scheduler_next();
void* serializar_suse_wait(char*);
void* serializar_suse_create(int);
void* serializar_hilolay_init();



#endif /* SERIALIZAR_H_ */
