#include "deserializar.h"

int determinar_protocolo(void* buffer){
	int codigo_de_operacion;
	int offset = 0;
	memcpy(&(codigo_de_operacion), (buffer+offset), sizeof(int));
	log_info(logger_de_deserializacion, "Obtuvimos el codigo de operacion \n");
	return codigo_de_operacion;
}

void identificar_paquete_y_ejecutar_comando(int cliente_socket, void* buffer){

	log_info(logger_de_deserializacion, "Estamos por deserializar el codigo de operacion\n");
	int codigo_de_operacion=determinar_protocolo(buffer);

	int TID;


	switch(codigo_de_operacion){

	case HILOLAY_INIT:
		log_info(logger_de_deserializacion, "Es el codigo de 'suse_create', comenzando la deserializacion de parametros\n");

		armar_paquete((void*)_hilolay_init(cliente_socket),INT,cliente_socket);
		break;


	case SUSE_CREATE:
		log_info(logger_de_deserializacion, "Es el codigo de 'suse_create', comenzando la deserializacion de parametros\n");
		int tid=descifrar_suse_create(buffer);
		armar_paquete((void*)suse_create(tid,cliente_socket),INT,cliente_socket);
		break;

	case SUSE_SCHEDULER_NEXT:
		log_info(logger_de_deserializacion, "Es el codigo de 'suse_scheduler_next', comenzando la deserializacion de parametros\n");
		int hilo_siguiente=suse_schedule_next(cliente_socket);
		armar_paquete((void*)hilo_siguiente,INT,cliente_socket);
		break;

	case SUSE_WAIT:
		log_info(logger_de_deserializacion, "Es el codigo de 'suse_wait', comenzando la deserializacion de parametros\n");
		semaforo_descifrado_t* semaforo_wait=descifrar_suse_wait(buffer);
		int paquete=suse_wait(semaforo_wait->nombre_del_semaforo, semaforo_wait->tid,cliente_socket);
		armar_paquete((void*)paquete,INT ,cliente_socket);
		free(semaforo_wait);
		break;

	case SUSE_SIGNAL:
		log_info(logger_de_deserializacion, "Es el codigo de 'suse_signal', comenzando la deserializacion de parametros\n");
		semaforo_descifrado_t* semaforo_signal=descifrar_suse_signal(buffer);
		armar_paquete((void*)suse_signal(semaforo_signal->nombre_del_semaforo,cliente_socket),INT,cliente_socket);
		free(semaforo_signal);
		break;

	case SUSE_JOIN:
		log_info(logger_de_deserializacion, "Es el codigo de 'suse_join', comenzando la deserializacion de parametros\n");
		TID=descifrar_suse_join(buffer);
		armar_paquete((void*)suse_join(TID,cliente_socket),INT,cliente_socket);

		break;

	case SUSE_CLOSE:
		log_info(logger_de_deserializacion, "Es el codigo de 'suse_close', comenzando la deserializacion de parametros\n");
		TID = descifrar_suse_close(buffer);
		armar_paquete(suse_close(TID,cliente_socket),INT,cliente_socket);

		break;

	default:
		send(cliente_socket, "Codigo Invalido", 16, 0);
		log_info(logger_de_deserializacion, "Nos llego un codigo invalido\n");
	}

}


/*
void* serializar_bool(bool dato){

	int peso_dato=sizeof(bool)+sizeof(int);
	void* paquete= malloc(peso_dato +sizeof(int));
	memcpy(paquete,&peso_dato ,sizeof(int));
	memcpy(paquete, &dato, sizeof(int));//NO SE SI VA EL & TODO, SI ROMPE MIREN ESTO
	return paquete;
}*/

int descifrar_suse_create(void*param){
	int offset=(int)sizeof(int);
	int TID;
	memcpy(&TID, param+offset,sizeof(int));
	return TID;

}
// El suse_create() esta en colas.c



semaforo_descifrado_t* descifrar_suse_wait(void*param){
	semaforo_descifrado_t* semaforo=(semaforo_descifrado_t*)malloc(sizeof(semaforo_descifrado_t));
	int offset=(int)sizeof(int);
	int longitud_nombre=0;
	memcpy(&longitud_nombre, param+offset,sizeof(int));
	semaforo->nombre_del_semaforo= (char*) malloc(longitud_nombre);
	offset+= sizeof(int);
	memcpy(semaforo->nombre_del_semaforo, param+offset, longitud_nombre);
	offset+=longitud_nombre;
	memcpy(&semaforo->tid, param+offset, sizeof(int));
	return semaforo;
}

char* descifrar_suse_signal(void* param){
	semaforo_descifrado_t* semaforo=(semaforo_descifrado_t*)malloc(sizeof(semaforo_descifrado_t));
	int offset=(int)sizeof(int);
	int longitud_nombre=0;
	memcpy(&longitud_nombre, param+offset,sizeof(int));
	semaforo->nombre_del_semaforo= (char*) malloc(longitud_nombre);
	offset+= sizeof(int);
	memcpy(semaforo->nombre_del_semaforo, param+offset, longitud_nombre);
	offset+=longitud_nombre;
	memcpy(&semaforo->tid, param+offset, sizeof(int));
	return semaforo;
}

int descifrar_suse_join(void* param){
	int offset=(int)sizeof(int);
	int TID;
	memcpy(&TID, param+offset,sizeof(int));
	return TID;
}

int descifrar_suse_close(void* buffer){
	int offset=(int)sizeof(int);
	int TID;
	memcpy(&TID, buffer+offset,sizeof(int));
	return TID;
}

void armar_paquete(void* dato, int tipo_de_dato, int cliente){

	void* paquete;
	int size;
	switch(tipo_de_dato){
	case BOOLEAN:
		size=(sizeof(bool));
		paquete=malloc(sizeof(bool)+sizeof(int));
		memcpy(paquete,&size,sizeof(int));
		memcpy(paquete+ sizeof(bool),&dato,sizeof(bool));
		break;
	case INT:
		size=(sizeof(int));
		paquete=(void*)malloc(sizeof(int)+sizeof(int));
		memcpy(paquete,&size,sizeof(int));
		memcpy(paquete+ sizeof(int),&dato,sizeof(int));
		break;
	case CHAR:
		size=(sizeof(char));
		paquete=malloc(sizeof(char)+sizeof(int));
		memcpy(paquete,&size,sizeof(int));
		memcpy(paquete+ sizeof(char),&dato,sizeof(char));
		break;
	case LONG:
		size=(sizeof(long));
		paquete=malloc(sizeof(long)+sizeof(int));
		memcpy(paquete,&size,sizeof(int));
		memcpy(paquete+ sizeof(long),&dato,sizeof(long));
		break;
	}


	int cosa=send(cliente,paquete,size+sizeof(int),0);

	free(paquete);

}
