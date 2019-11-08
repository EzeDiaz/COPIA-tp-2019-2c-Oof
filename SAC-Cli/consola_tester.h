/*
 * consola_tester.h
 *
 *  Created on: 2 nov. 2019
 *      Author: utnso
 */

#ifndef CONSOLA_TESTER_H_
#define CONSOLA_TESTER_H_

#include "SAC-Cli.h"
#include <readline/readline.h>
#include <stddef.h>
#include <stdlib.h>
#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>

int consola();
int leer_parametros_mkdir(char*,mode_t*,char*);
int leer_parametros_readdir(char*, void * ,fuse_fill_dir_t ,unsigned int * ,struct fuse_file_info*,char* );


#endif /* CONSOLA_TESTER_H_ */
