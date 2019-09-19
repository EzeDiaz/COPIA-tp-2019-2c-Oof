/*
 * SUSE.c
 *
 *  Created on: 18 sep. 2019
 *      Author: utnso
 */

#include "metricas.h"

int main(){

	start_up();

	/*DESARROLLO*/

	terminate_SUSE();
	return 0;
}

void start_up(){
	/*INICIALIZO RECURSOS*/

	incializar_log_sistema();

}

void terminate_SUSE(){


	/*LIBERO RECURSOS GLOBALES*/
	destruir_log_sistema();


}

