#include "deserializar.h"
#include "SUSE.c"
#include "TADs.h"
#include "colas.c"

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
	void* resultado;
	void* paquete_descifrado;
	int TID;
	semaforo_descifrado_t* semaforo;

	switch(codigo_de_operacion){

	case HILOLAY_INIT:
		log_info(logger_de_deserializacion, "Es el codigo de 'hilolay init', comenzando la deserializacion de parametros\n");
		paquete_descifrado=descifrar_hilolay_init(buffer);
		//resultado=hilolay_init(paquete_descifrado);
		serializar_y_enviar_resultado(resultado,cliente_socket);
		break;

	case SUSE_CREATE:
		log_info(logger_de_deserializacion, "Es el codigo de 'suse_create', comenzando la deserializacion de parametros\n");
		paquete_descifrado=descifrar_suse_create(buffer);
		//resultado=suse_create(paquete_descifrado);
		serializar_y_enviar_resultado(resultado,cliente_socket);
		break;

	case SUSE_SCHEDULER_NEXT:
		log_info(logger_de_deserializacion, "Es el codigo de 'suse_scheduler_next', comenzando la deserializacion de parametros\n");
		paquete_descifrado=descifrar_suse_scheduler_next(buffer);
		resultado=suse_scheduler_next(paquete_descifrado);
		serializar_y_enviar_resultado(resultado,cliente_socket);
		break;

	case SUSE_WAIT:
		log_info(logger_de_deserializacion, "Es el codigo de 'suse_wait', comenzando la deserializacion de parametros\n");
		semaforo=descifrar_suse_wait(buffer);
		resultado=suse_wait(semaforo);
		serializar_y_enviar_resultado(resultado,cliente_socket);
		break;

	case SUSE_SIGNAL:
		log_info(logger_de_deserializacion, "Es el codigo de 'suse_signal', comenzando la deserializacion de parametros\n");
		semaforo=descifrar_suse_signal(buffer);
		resultado=suse_signal(semaforo);
		serializar_y_enviar_resultado(resultado,cliente_socket);
		break;

	case SUSE_JOIN:
		log_info(logger_de_deserializacion, "Es el codigo de 'suse_join', comenzando la deserializacion de parametros\n");
		paquete_descifrado=descifrar_suse_join(buffer);
		resultado=suse_join(paquete_descifrado);
		serializar_y_enviar_resultado(resultado,cliente_socket);
		break;

	case SUSE_CLOSE:
		log_info(logger_de_deserializacion, "Es el codigo de 'suse_close', comenzando la deserializacion de parametros\n");
		TID = descifrar_suse_close(buffer);
		resultado = suse_close(TID);
		serializar_y_enviar_resultado(resultado,cliente_socket);
		break;

	default:
		send(cliente_socket, "Codigo Invalido", 16, 0);
		log_info(logger_de_deserializacion, "Nos llego un codigo invalido\n");
	}
	free(resultado);
	free(paquete_descifrado);
}

void serializar_y_enviar_resultado(void* param1,int param2){
	return NULL;
}

void* descifrar_hilolay_init(void*param){
	return NULL;
}/*
void* hilolay_init(void*param){
	return NULL;
}*/

void* descifrar_suse_create(void*param){
	return NULL;
}
// El suse_create() esta en colas.c

void* descifrar_suse_scheduler_next(void*param){
	return NULL;
}
void* suse_scheduler_next(void*param){
	return NULL;
}

void* descifrar_suse_wait(void*param){
	return NULL;
}
void* suse_wait(semaforo_descifrado_t* semaforo){

	//Genero una operacion wait sobre el semaforo dado
	wait(semaforo->nombre_del_semaforo,semaforo->tid);

	//return el_hilo_pudo_obtener_semaforo(); //TODO
	return NULL;
}

void* descifrar_suse_signal(void* param){
	return NULL;
}
void* suse_signal(semaforo_descifrado_t* semaforo){

	//Genero una operacion signal sobre el semaforo dado
	signal(semaforo->nombre_del_semaforo,semaforo->tid);

	return NULL;
}

void* descifrar_suse_join(void* param){
	return NULL;
}
void* suse_join(void* param){
	return NULL;
}

int descifrar_suse_close(void* buffer){
	int offset = sizeof(int);
	int longitud_del_siguiente = 0;

	memcpy(&longitud_del_siguiente, (buffer + offset), sizeof(int));
	offset += sizeof(int);

	return NULL;
}
void* suse_close(int TID){
	// Con el TID que me pasan yo tengo que identificar al hilo en cuestion
	// para poder mandarlo a EXIT
	hilo_t* un_hilo;
	TID = un_hilo->hilo_informacion->tid; //Esto esta matado
	exit_thread(un_hilo);
	return NULL;
}
