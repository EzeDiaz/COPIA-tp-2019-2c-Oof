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
	void* resultado;
	void* paquete_descifrado;

	switch(codigo_de_operacion){

	case HILOLAY_INIT:
		log_info(logger_de_deserializacion, "Es el codigo de 'hilolay init', comenzando la deserializacion de parametros\n");
		paquete_descifrado=descifrar_hilolay_init(buffer);
		resultado=hilolay_init(paquete_descifrado);
		serializar_y_enviar_resultado(resultado,cliente_socket);
		break;

	case SUSE_CREATE:
		log_info(logger_de_deserializacion, "Es el codigo de 'suse_create', comenzando la deserializacion de parametros\n");
		paquete_descifrado=descifrar_suse_create(buffer);
		resultado=suse_create(paquete_descifrado);
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
		paquete_descifrado=descifrar_suse_wait(buffer);
		resultado=suse_wait(paquete_descifrado);
		serializar_y_enviar_resultado(resultado,cliente_socket);
		break;

	case SUSE_SIGNAL:
		log_info(logger_de_deserializacion, "Es el codigo de 'suse_signal', comenzando la deserializacion de parametros\n");
		paquete_descifrado=descifrar_suse_signal(buffer);
		resultado=suse_signal(paquete_descifrado);
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
		paquete_descifrado=descifrar_suse_close(buffer);
		resultado=suse_close(paquete_descifrado);
		serializar_y_enviar_resultado(resultado,cliente_socket);
		break;

	default:
		send(cliente_socket, "Codigo Invalido", 16, 0);
		log_info(logger_de_deserializacion, "Nos llego un codigo invalido\n");
	}
	free(resultado);
	free(paquete_descifrado);
}
