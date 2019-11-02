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
		//Come on do stuff
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

		if(memory_left >= bytes_a_reservar) {
			if(CLIENT_HAS_SEGMENT(cliente)) {
				//El proceso que pide ya tiene segmento
				void intentar_usar_segmento(segment* un_segmento) {
					if(!se_pudo_reservar_flag) {
						void* pointer = SEGMENT_IS_BIG_ENOUGH(un_segmento, bytes_a_reservar + 5); //Porque quiero guardar la ultima metadata
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
							memcpy(pointer + bytes_a_reservar, &bytes_sobrantes, sizeof(uint32_t));
							memcpy(pointer + bytes_a_reservar + sizeof(uint32_t), &true, sizeof(uint32_t));
							sem_post(&mp_semaphore);
							se_pudo_reservar_flag = 1;
						} else {
							//El segmento no tiene lugar
							if() {
								//El segmento puede agrandarse
								se_pudo_reservar_flag = 1;
							} else {
								//El segmento no puede agrandarse
							}
						}
					}
				}
				t_list* segments_list = GET_CLIENT_SEGMENTS(cliente);
				list_iterate(segments_list, intentar_usar_segmento);
				debe_crearse_segmento_flag = !se_pudo_reservar_flag;
				//borrar la segments_list
			} else {
				//El proceso que pide no tiene segmento
				debe_crearse_segmento_flag = 1;
			}

			if(debe_crearse_segmento_flag) {
				//Crear nuevo segmento
			}
		} else {
			//No pude escribir porque no hay memoria
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

