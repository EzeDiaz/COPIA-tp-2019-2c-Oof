/*
	En este .c esta toddo lo relacionado al servidor.
		1. Escucha conexiones
		2. Recibe un paquete
		3. Hace lo que corresponde segun el codigo de operacion
		4. Envia por socket una respuesta
 */

//Bibliotecas propias
#include <libMUSE.h>
#include "globales.h"
#include "estructuras_MUSE.h"

//Commons
#include <commons/string.h>

//Standards
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

//Para poder pedir el pid
#include <unistd.h>

//Para usar sockets
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>


void iniciarServidor(){

	client_list = list_create();

	struct sockaddr_in direccionServidor;
	direccionServidor.sin_family= AF_INET;
	direccionServidor.sin_addr.s_addr= inet_addr(config_get_string_value(config,"IP")); //INADDR_ANY;
	direccionServidor.sin_port=htons(config_get_int_value(config,"PUERTO"));


	int servidor = socket(AF_INET, SOCK_STREAM , 0);
	sem_wait(&semaforoLogger);
	log_info(logger, "Levantamos el servidor\n");
	sem_post(&semaforoLogger);

	int activado = 1;
	setsockopt(servidor, SOL_SOCKET, SO_REUSEADDR, &activado, sizeof(activado));

	if(bind(servidor, (void*) &direccionServidor, sizeof(direccionServidor)) != 0){
		sem_wait(&semaforoLogger);
		log_info(logger, "Fallo el bind");
		sem_post(&semaforoLogger);
		perror("Fallo el bind");
		exit(1);
	}

	int cliente;

	sem_wait(&semaforoLogger);
	log_info(logger, "Servidor listo para recibir un cliente\n");
	sem_post(&semaforoLogger);

	listen(servidor, 100);
	pthread_t *hilo;
	struct sockaddr_in direccionCliente;
	unsigned tamanioDireccion= sizeof(struct sockaddr_in);
	cliente = accept(servidor, (void*) &direccionCliente, &tamanioDireccion);

	while(1){
		sem_wait(&semaforoLogger);
		log_info(logger, "Recibimos un cliente\n");
		sem_post(&semaforoLogger);
		pthread_create(&hilo, NULL,  (void*)atenderCliente, cliente);
		pthread_detach(hilo);
		cliente = accept(servidor, (void*) &direccionCliente, &tamanioDireccion);
	}
}

void atenderCliente(int cliente){

	void* buffer;
	int alocador;

	sem_wait(&semaforoLogger);
	log_info(logger, "Recibimos conexion \n");
	sem_post(&semaforoLogger);

	buffer=recibirBuffer(&alocador,cliente);

	while(0<alocador){
		realizarRequest(buffer, cliente);
		free(buffer);
		buffer=recibirBuffer(&alocador,cliente);
	}

	sem_wait(&semaforoLogger);
	log_info(logger, "Se desconecto el cliente\n");
	sem_post(&semaforoLogger);
	close(cliente);
}

void* recibirBuffer(int* alocador, int cliente){

	void* buffer;

	if(recv(cliente, alocador, 4, MSG_WAITALL)!=0){
		buffer = malloc(*alocador);
		recv(cliente, buffer,*alocador, MSG_WAITALL);
		return buffer;
	}else{
		*alocador=0;
		return buffer;
	}
}

void realizarRequest(void *buffer, int cliente){

	int cod_op;
	memcpy(&cod_op, buffer, sizeof(int));

	switch(cod_op){

	//init
	case 100:
		int offset= sizeof(int);
		int longitudDelSiguiente=0;
		char* IP_ID;

		memcpy(&longitudDelSiguiente, (buffer + offset), sizeof(int));
		IP_ID=(char*)malloc(longitudDelSiguiente);
		offset= offset+sizeof(int);
		memcpy(IP_ID, (buffer + offset), longitudDelSiguiente);

		ADD_CLIENT_TO_LIST(IP_ID, cliente);

		int resultado = CREATE_ADDRESS_SPACE(IP_ID);

		void* buffer;
		buffer=(void*)malloc(sizeof(int));
		memcpy(buffer,&resultado,sizeof(int));

		send(socket_MUSE, buffer, sizeof(buffer),0);

		free(buffer);
		break;

		//close
	case 101:
		//Habria que:
		//	1)Liberar los frames ocupados por el proceso (bitmap)
		//	2)Limpiar las estructuras administrativas asociadas al proceso
		//		a)Limpio la segment_table
		//		b)free del owner
		//		c)Lo saco de all_address_spaces
		//		d)Lo saco de la lista de clientes
		break;

		//alloc
	case 102:
		int offset= sizeof(int);
		int longitudDelSiguiente=0;
		uint32_t bytes_a_reservar;

		memcpy(&longitudDelSiguiente, (buffer + offset), sizeof(int));
		offset= offset+sizeof(int);
		memcpy(&bytes_a_reservar, (buffer + offset), longitudDelSiguiente);

		int debe_crearse_segmento_flag = 0;
		int se_pudo_reservar_flag = 0;
		void* pointer;

		addressSpace* client_address_space = GET_ADDRESS_SPACE(cliente);

		if(memory_left >= bytes_a_reservar) {
			if(THERE_ARE_EXISTING_HEAP_SEGMENTS(client_address_space)) {
				//El proceso que pide ya tiene segmentos de heap
				void usar_segmento_si_tiene_espacio(segment* un_segmento) {
					if(!se_pudo_reservar_flag) {
						pointer = SEGMENT_IS_BIG_ENOUGH(un_segmento, bytes_a_reservar + 5); //Porque quiero guardar la ultima metadata
						if(pointer != NULL) {
							//El segmento tiene lugar
							sem_wait(&mp_semaphore);
							uint32_t bytes_que_habia;
							uint32_t bytes_sobrantes;
							memcpy(&bytes_que_habia, pointer - 5, sizeof(uint32_t));
							//Sobreescribo la metadata
							memcpy(pointer - 5, &bytes_a_reservar, sizeof(uint32_t));
							memcpy(pointer - 1, &false, sizeof(bool));
							//Escribo la nueva metadata
							bytes_sobrantes = bytes_que_habia - bytes_a_reservar - 5;
							memcpy(pointer + bytes_a_reservar, &bytes_sobrantes, sizeof(uint32_t));
							memcpy(pointer + bytes_a_reservar + sizeof(uint32_t), &true, sizeof(uint32_t));
							sem_post(&mp_semaphore);
							se_pudo_reservar_flag = 1;
						}
					}
				}

				void usar_segmento_si_se_puede_agrandar(segment* un_segmento) {
					if(!se_pudo_reservar_flag) {
						if(SEGMENT_CAN_BE_EXTENDED(un_segmento, client_address_space, bytes_a_reservar + 5)) {
							//Puedo agrandar el segmento
							void* last_metadata = GET_LAST_METADATA(un_segmento);
							uint32_t internal_fragmentation;
							memcpy(&internal_fragmentation, last_metadata, sizeof(uint32_t));
							int frames_to_require;
							if((bytes_a_reservar + 5 - internal_fragmentation) % page_size > 0) {
								frames_to_require = ((bytes_a_reservar + 5 - internal_fragmentation) / page_size) + 1;
							} else {
								frames_to_require = (bytes_a_reservar + 5 - internal_fragmentation) / page_size;
							}

							sem_wait(&mp_semaphore);
							uint32_t bytes_que_habia;
							uint32_t bytes_sobrantes;
							uint32_t bytes_que_quedan;
							memcpy(&bytes_que_habia, last_metadata, sizeof(uint32_t));
							//Sobreescribo la metadata
							memcpy(last_metadata, &bytes_a_reservar, sizeof(uint32_t));
							memcpy(last_metadata + sizeof(uint32_t), &false, sizeof(bool));

							bytes_que_quedan = bytes_a_reservar - internal_fragmentation;

							pageFrame* last_page;

							for(int i=0; i < frames_to_require; i++) {
								int frame_number = ASSIGN_FIRST_FREE_FRAME();
								pageFrame* new_page = (pageFrame*)malloc(sizeof(pageFrame));
								new_page->modifiedBit = 1; //Not sure
								new_page->presenceBit = 1;
								list_add(client_address_space->segment_table, new_page);
								if(bytes_que_quedan - page_size > 0)
									bytes_que_quedan = bytes_que_quedan - page_size;
								last_page = new_page;
							}

							//Escribo la nueva metadata
							bytes_sobrantes = page_size - bytes_que_quedan - 5;
							void* pointer = GET_FRAME_POINTER(last_page->frame_number);
							memcpy(pointer + bytes_que_quedan, &bytes_sobrantes, sizeof(uint32_t));
							memcpy(pointer + bytes_que_quedan + sizeof(uint32_t), &true, sizeof(uint32_t));
							sem_post(&mp_semaphore);

							se_pudo_reservar_flag = 1;
						}
					}
				}

				t_list* heap_segments_list = GET_HEAP_SEGMENTS(client_address_space);
				list_iterate(heap_segments_list, usar_segmento_si_tiene_espacio);

				if(!se_pudo_reservar_flag) //Ningun segmento tenia espacio, trato de agrandar
					list_iterate(heap_segments_list, usar_segmento_si_se_puede_agrandar);

				debe_crearse_segmento_flag = !se_pudo_reservar_flag;
				//borrar la heap_segments_list (porque es resultado de un filter)
			} else {
				//El proceso que pide no tiene segmento
				debe_crearse_segmento_flag = 1;
			}
			if(debe_crearse_segmento_flag) {
				//Crear nuevo segmento
			}
		} else {
			//No pude escribir porque no hay memoria
			//Existe esta condicion? O agarro y mando un frame a swap y chau?
		}

		/* Armamos el paquetito de respuesta
		void* buffer;
		int peso=0;
		offset=0;
		peso+=strlen(resultado)+1;
		buffer=(void*)malloc(peso+sizeof(int));
		memcpy(buffer,&peso,sizeof(int));
		offset=sizeof(int);
		memcpy(buffer+offset,resultado,peso);
		 */
		//send

		free(buffer);
		break;

		//free
	case 103:
		int offset= sizeof(int);
		int longitudDelSiguiente=0;
		uint32_t dir;

		memcpy(&longitudDelSiguiente, (buffer + offset), sizeof(int));
		offset= offset+sizeof(int);
		memcpy(&dir, (buffer + offset), longitudDelSiguiente);

		//MUSE YO TE INVOCO

		/* Armamos el paquetito de respuesta
		void* buffer;
		int peso=0;
		offset=0;
		peso+=strlen(resultado)+1;
		buffer=(void*)malloc(peso+sizeof(int));
		memcpy(buffer,&peso,sizeof(int));
		offset=sizeof(int);
		memcpy(buffer+offset,resultado,peso);
		 */
		//send

		free(buffer);
		break;

		//get
	case 104:
		int offset= sizeof(int);
		int longitudDelSiguiente=0;
		void* dst;
		uint32_t src;
		size_t n;

		memcpy(&longitudDelSiguiente, (buffer + offset), sizeof(int));
		offset= offset+sizeof(int);
		dst = (void*)malloc(longitudDelSiguiente);
		memcpy(dst, (buffer + offset), longitudDelSiguiente);
		offset= offset+longitudDelSiguiente;

		memcpy(&longitudDelSiguiente, (buffer + offset), sizeof(int));
		offset= offset+sizeof(int);
		memcpy(&src, (buffer + offset), longitudDelSiguiente);
		offset= offset+longitudDelSiguiente;

		memcpy(&longitudDelSiguiente, (buffer + offset), sizeof(int));
		offset= offset+sizeof(int);
		memcpy(&n, (buffer + offset), longitudDelSiguiente);
		offset= offset+longitudDelSiguiente;

		//MUSE YO TE INVOCO

		/* Armamos el paquetito de respuesta
		void* buffer;
		int peso=0;
		offset=0;
		peso+=strlen(resultado)+1;
		buffer=(void*)malloc(peso+sizeof(int));
		memcpy(buffer,&peso,sizeof(int));
		offset=sizeof(int);
		memcpy(buffer+offset,resultado,peso);
		 */
		//send

		free(buffer);
		free(dst);
		break;

		//cpy
	case 105:
		int offset= sizeof(int);
		int longitudDelSiguiente=0;
		uint32_t dest;
		void* source;
		size_t n;

		memcpy(&longitudDelSiguiente, (buffer + offset), sizeof(int));
		offset= offset+sizeof(int);
		memcpy(&dest, (buffer + offset), longitudDelSiguiente);
		offset= offset+longitudDelSiguiente;

		memcpy(&longitudDelSiguiente, (buffer + offset), sizeof(int));
		offset= offset+sizeof(int);
		source = (void*)malloc(longitudDelSiguiente);
		memcpy(source, (buffer + offset), longitudDelSiguiente);
		offset= offset+longitudDelSiguiente;

		memcpy(&longitudDelSiguiente, (buffer + offset), sizeof(int));
		offset= offset+sizeof(int);
		memcpy(&n, (buffer + offset), longitudDelSiguiente);
		offset= offset+longitudDelSiguiente;

		//MUSE YO TE INVOCO

		/* Armamos el paquetito de respuesta
		void* buffer;
		int peso=0;
		offset=0;
		peso+=strlen(resultado)+1;
		buffer=(void*)malloc(peso+sizeof(int));
		memcpy(buffer,&peso,sizeof(int));
		offset=sizeof(int);
		memcpy(buffer+offset,resultado,peso);
		 */
		//send

		free(buffer);
		free(source);
		break;

		//map
	case 106:
		int offset= sizeof(int);
		int longitudDelSiguiente=0;
		char* path;
		size_t length;
		int flags;

		memcpy(&longitudDelSiguiente, (buffer + offset), sizeof(int));
		offset= offset+sizeof(int);
		path = (char*)malloc(longitudDelSiguiente);
		memcpy(path, (buffer + offset), longitudDelSiguiente);
		offset= offset+longitudDelSiguiente;

		memcpy(&longitudDelSiguiente, (buffer + offset), sizeof(int));
		offset= offset+sizeof(int);
		memcpy(&length, (buffer + offset), longitudDelSiguiente);
		offset= offset+longitudDelSiguiente;

		memcpy(&longitudDelSiguiente, (buffer + offset), sizeof(int));
		offset= offset+sizeof(int);
		memcpy(&flags, (buffer + offset), longitudDelSiguiente);
		offset= offset+longitudDelSiguiente;

		//MUSE YO TE INVOCO

		/* Armamos el paquetito de respuesta
		void* buffer;
		int peso=0;
		offset=0;
		peso+=strlen(resultado)+1;
		buffer=(void*)malloc(peso+sizeof(int));
		memcpy(buffer,&peso,sizeof(int));
		offset=sizeof(int);
		memcpy(buffer+offset,resultado,peso);
		 */
		//send

		free(buffer);
		free(path);
		break;

		//sync
	case 107:
		int offset= sizeof(int);
		int longitudDelSiguiente=0;
		uint32_t addr;
		size_t len;

		memcpy(&longitudDelSiguiente, (buffer + offset), sizeof(int));
		offset= offset+sizeof(int);
		memcpy(&addr, (buffer + offset), longitudDelSiguiente);
		offset= offset+longitudDelSiguiente;

		memcpy(&longitudDelSiguiente, (buffer + offset), sizeof(int));
		offset= offset+sizeof(int);
		memcpy(&len, (buffer + offset), longitudDelSiguiente);
		offset= offset+longitudDelSiguiente;

		//MUSE YO TE INVOCO

		/* Armamos el paquetito de respuesta
		void* buffer;
		int peso=0;
		offset=0;
		peso+=strlen(resultado)+1;
		buffer=(void*)malloc(peso+sizeof(int));
		memcpy(buffer,&peso,sizeof(int));
		offset=sizeof(int);
		memcpy(buffer+offset,resultado,peso);
		 */
		//send

		free(buffer);
		break;

		//unmap
	case 108:
		int offset= sizeof(int);
		int longitudDelSiguiente=0;
		uint32_t direc;

		memcpy(&longitudDelSiguiente, (buffer + offset), sizeof(int));
		offset= offset+sizeof(int);
		memcpy(&direc, (buffer + offset), longitudDelSiguiente);
		offset= offset+longitudDelSiguiente;

		//MUSE YO TE INVOCO

		/* Armamos el paquetito de respuesta
		void* buffer;
		int peso=0;
		offset=0;
		peso+=strlen(resultado)+1;
		buffer=(void*)malloc(peso+sizeof(int));
		memcpy(buffer,&peso,sizeof(int));
		offset=sizeof(int);
		memcpy(buffer+offset,resultado,peso);
		 */
		//send

		free(buffer);
		break;

	default:
		break;
	}

	sem_wait(&semaforoLogger);
	log_info(logger,"Terminamos el request\n");
	sem_post(&semaforoLogger);
}

