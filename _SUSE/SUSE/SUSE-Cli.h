/*
 * SUSE-Cli.h
 *
 *  Created on: 7 nov. 2019
 *      Author: utnso
 */

#ifndef SUSE_CLI_H_
#define SUSE_CLI_H_
#include "serializar.h"
#include <hilolay/hilolay.h>
#include <hilolay/alumnos.h>


int suse_close(int);
int suse_join(int);
int suse_signal(int,char*);
int suse_schedule_next();
int suse_wait(int,char*);
int suse_create(int);
void _hilolay_init();

void start_up();
void conectar_con_servidor(int argc, char* argv);

void* enviar_paquete(void*);
void* recibir_resultado(int* );




#endif /* SUSE_CLI_H_ */
