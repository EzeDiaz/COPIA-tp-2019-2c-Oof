//Bibliotecas propias
#include <libMUSE.h>
#include "serverMUSE.h"
#include "globales.h"
#include "estructuras_MUSE.h"

//Commons
#include <commons/string.h>
#include <commons/config.h>
#include <commons/log.h>
#include <commons/collections/list.h>

//Standards
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include <signal.h>

//Para poder pedir el pid
#include <unistd.h>

//Para usar sockets
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <pthread.h>

int main(){
	remove("MUSE_logger.log");
	logger = log_create("MUSE_logger.log","main_server", 0, LOG_LEVEL_DEBUG);	//hacemo' el log

	CHECK_LOGGER();

	config = config_create("MUSE_config.config"); //abrimo' el archivo config

	CHECK_CONFIG();
	CHECK_FIELDS();
	GET_CONFIG_VALUES();

	SET_BITMAP_MEMORY();
	SET_BITMAP_SWAP();

	SWAP_INIT();

	INITIALIZE_SEMAPHORES();

	mp_pointer=malloc(memory_size);
	CHECK_MEMORY();	//checkeo si
	bzero(mp_pointer,memory_size);	//limpiamos la memoria

	all_address_spaces = list_create(); //Incializo el espacio de direcciones del sistema
	mapped_files = list_create(); //Para controlar los archivos mapeados

	//Inicializo las cuestiones del clock
	clock_pointer = 0;
	//No se que tan necesario es esto
	for(int i=0;i<memory_size/page_size;i++){
		clock_table[i]=  (pageFrame*) malloc(sizeof(pageFrame));
	}

	/*
		WRITE_HEAPMETADATA_IN_MEMORY(mp_pointer,memory_left,0);
		READ_HEAPMETADATA_IN_MEMORY(mp_pointer);
		printf("hola maude! \n");

		CREATE_NEW_EMPTY_SEGMENT("jorge");
		segment* new =list_get(segmentation_table,0);
		printf("Nombre del primer segmento : %s",new->owner);

		free(mp_pointer);
	 */

	iniciarServidor();

	DESTROY_SEMAPHORES();
	return 0;
}
