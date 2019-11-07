/*
 * SUSE-Cli.h
 *
 *  Created on: 7 nov. 2019
 *      Author: utnso
 */

#ifndef SUSE_CLI_H_
#define SUSE_CLI_H_
#include "serializar.h"


void suse_close(int);
void suse_join(int);
void suse_signal(char*);
void suse_scheduler_next();
void suse_wait(char*);
void suse_create(hilolay_t *, const hilolay_attr_t *,void *(*start_routine)(void *), void *);

void start_up();
void conectar_con_servidor(int, char**);


#endif /* SUSE_CLI_H_ */
