//Bibliotecas propias
#include <libMUSE.h>
#include <serverMUSE.h>
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

//Para poder pedir el pid
#include <unistd.h>

//Para usar sockets
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>

int main(){
	remove("MUSE_logger.log");
		logger = log_create("MUSE_logger.log","main_server", 0, LOG_LEVEL_DEBUG);	//hacemo' el log

		CHECK_LOGGER();

		config = config_create("MUSE_config.config"); //abrimo' el archivo config

		CHECK_CONFIG();
		CHECK_FIELDS();
		GET_CONFIG_VALUES();

		SET_BITMAP();

		INITIALIZE_SEMAPHORES();

		mp_pointer=malloc(memory_size);
		CHECK_MEMORY();	//checkeo si
		bzero(mp_pointer,memory_size);	//limpiamos la memoria

		segmentation_table=list_create();

		WRITE_HEAPMETADATA_IN_MEMORY(mp_pointer,memory_left,0);
		READ_HEAPMETADATA_IN_MEMORY(mp_pointer);
		printf("hola maude! \n");

		CREATE_NEW_EMPTY_SEGMENT("jorge");
		segment* new =list_get(segmentation_table,0);
		printf("Nombre del primer segmento : %s",new->owner);

		free(mp_pointer);
		DESTROY_SEMAPHORES();
		return 0;

	iniciar_servidor();

	return 0;
}
