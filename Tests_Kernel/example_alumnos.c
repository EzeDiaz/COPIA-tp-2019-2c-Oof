/*
 * example_alumnos.c
 *
 *  Created on: 26 nov. 2019
 *      Author: utnso
 */

#include <hilolay/alumnos.h>

#include <hilolay/hilolay.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <SUSE/SUSE-Cli.h>
/* Lib implementation: It'll only schedule the last thread that was created */
//int max_tid = 0;





static struct hilolay_operations hiloops = {
		.suse_create = &suse_create,
		//.suse_schedule_next = &suse_schedule_next,
		.suse_join = &suse_join,
		.suse_close = &suse_close,
		.suse_wait = &suse_wait,
		.suse_signal = &suse_signal
};

/*
void hilolay_init(void){
	init_internal(&hiloops);
	_hilolay_init();
}
*/
