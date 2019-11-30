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


enum {
	HILOLAY_INIT = 100,
	SUSE_CREATE = 101,
	SUSE_SCHEDULER_NEXT = 102,
	SUSE_WAIT = 103,
	SUSE_SIGNAL = 104,
	SUSE_JOIN = 105,
	SUSE_CLOSE = 106,
};

void* serializar_suse_close(int);
void* serializar_suse_join(int);
void* serializar_suse_signal(char*);
void* serializar_suse_scheduler_next();
void* serializar_suse_wait(char*);
void* serializar_suse_create(int);
void* serializar_hilolay_init();



#endif /* SERIALIZAR_H_ */
