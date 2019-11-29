/*
 * SAC-Cli.h
 *
 *  Created on: 30 oct. 2019
 *      Author: utnso
 */

#ifndef SAC_CLI_H_
#define SAC_CLI_H_



#include <fuse.h>
#include <errno.h>
#include <fcntl.h>
#include "serializacion.h"


void leer_config();
void conectar_SAC_SERVER(int , char*);
int run_sac(int,char**);



char* IP;
int puerto;


#endif /* SAC_CLI_H_ */
