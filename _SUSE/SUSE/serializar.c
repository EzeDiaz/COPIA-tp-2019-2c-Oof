/*
 * serializar.c
 *
 *  Created on: 7 nov. 2019
 *      Author: utnso
 */

#include "serializar.h"

void* serializar_hilolay_init(){
	int peso_total_paquete = sizeof(int);
	int codigo_de_operacion = HILOLAY_INIT;
	int offset = 0;
	void* paquete = malloc(peso_total_paquete+sizeof(int));

	memcpy(paquete + offset,&peso_total_paquete,sizeof(int));
	offset += sizeof(int);
	memcpy(paquete + offset,&codigo_de_operacion,sizeof(int));
	offset += sizeof(int);
	return paquete;

}

void* serializar_suse_close(int tid){

	int peso_total_paquete = sizeof(int)*3;
	int codigo_de_operacion = SUSE_CLOSE;
	int offset = 0;
	void* paquete = malloc(peso_total_paquete);

	memcpy(paquete + offset,&peso_total_paquete,sizeof(int));
	offset += sizeof(int);
	memcpy(paquete + offset,&codigo_de_operacion,sizeof(int));
	offset += sizeof(int);
	memcpy(paquete + offset,&tid,sizeof(int));

	return paquete;
}

void* serializar_suse_join(int tid){

	int peso_total_paquete = sizeof(int)*2;
	int codigo_de_operacion = SUSE_JOIN;
	int offset = 0;
	void* paquete = malloc(peso_total_paquete+sizeof(int));

	memcpy(paquete + offset,&peso_total_paquete,sizeof(int));
	offset += sizeof(int);
	memcpy(paquete + offset,&codigo_de_operacion,sizeof(int));
	offset += sizeof(int);
	memcpy(paquete + offset,&tid,sizeof(int));

	return paquete;

}

void* serializar_suse_signal(int tid,char* semaforo){

	int longitud = string_length(semaforo) + 1;
	int peso_total_paquete = longitud + sizeof(int)*4;
	int codigo_de_operacion = SUSE_SIGNAL;
	int offset = 0;

	void* paquete = malloc(peso_total_paquete);

	memcpy(paquete + offset,&peso_total_paquete,sizeof(int));
	offset += sizeof(int);
	memcpy(paquete + offset,&codigo_de_operacion,sizeof(int));
	offset += sizeof(int);
	memcpy(paquete + offset,&longitud,sizeof(int));
	offset += sizeof(int);
	memcpy(paquete + offset,semaforo,longitud);
	offset += longitud;
	memcpy(paquete + offset,&tid,sizeof(int));


	return paquete;

}

void* serializar_suse_scheduler_next(){

	int peso_total_paquete = sizeof(int)*2;
	int codigo_de_operacion = SUSE_SCHEDULER_NEXT;
	int offset = 0;
	void* paquete = malloc(peso_total_paquete);

	offset += sizeof(int);
	memcpy(paquete,&offset,sizeof(int));

	memcpy(paquete + offset,&codigo_de_operacion,sizeof(int));

	return paquete;
}

void* serializar_suse_wait(int tid,char*semaforo){

	int longitud = string_length(semaforo) + 1;
	int peso_total_paquete = longitud + sizeof(int)*4;
	int codigo_de_operacion = SUSE_WAIT;
	int offset = 0;

	void* paquete = malloc(peso_total_paquete);

	memcpy(paquete + offset,&peso_total_paquete,sizeof(int));
	offset += sizeof(int);
	memcpy(paquete + offset,&codigo_de_operacion,sizeof(int));
	offset += sizeof(int);
	memcpy(paquete + offset,&longitud,sizeof(int));
	offset += sizeof(int);
	memcpy(paquete + offset,semaforo,longitud);
	offset += longitud;
	memcpy(paquete + offset,&tid,sizeof(int));


	return paquete;
}


void* serializar_suse_create(int tid){
	int peso_total_paquete = sizeof(int)*3;
	int codigo_de_operacion = SUSE_CREATE;
	int offset = 0;
	void* paquete = malloc(peso_total_paquete);

	memcpy(paquete + offset,&peso_total_paquete,sizeof(int));
	offset += sizeof(int);
	memcpy(paquete + offset,&codigo_de_operacion,sizeof(int));
	offset += sizeof(int);
	memcpy(paquete + offset,&tid,sizeof(int));

	return paquete;
}

