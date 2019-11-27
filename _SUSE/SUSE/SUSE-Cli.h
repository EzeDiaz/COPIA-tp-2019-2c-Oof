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
void suse_schedule_next();
void suse_wait(char*);
void suse_create(int);
void _hilolay_init();

void start_up();
void conectar_con_servidor(int argc, char* argv);




#endif /* SUSE_CLI_H_ */
