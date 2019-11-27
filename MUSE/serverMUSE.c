/*
	En este .c esta toddo lo relacionado al servidor.
		1. Escucha conexiones
		2. Recibe un paquete
		3. Hace lo que corresponde segun el codigo de operacion
		4. Envia por socket una respuesta
 */

//Bibliotecas propias
#include <libMUSE.h>
#include <get_local_IP.h>
#include "globales.h"
#include "estructuras_MUSE.h"
#include "serverMUSE.h"

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

//More stuff
#include <sys/mman.h> //Para manejo de memoria
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>

void atenderCliente(int cliente){

	void* buffer;
	int alocador;

	sem_wait(&logger_semaphore);
	log_info(logger, "Recibimos conexion");
	sem_post(&logger_semaphore);

	buffer=recibirBuffer(&alocador,cliente);

	while(0<alocador){
		realizarRequest(buffer, cliente);
		free(buffer);
		buffer=recibirBuffer(&alocador,cliente);
	}

	sem_wait(&logger_semaphore);
	log_info(logger, "Se desconecto el cliente");
	sem_post(&logger_semaphore);
	close(cliente);
}

void iniciarServidor(){

	client_list = list_create();

	struct sockaddr_in direccionServidor;
	direccionServidor.sin_family= AF_INET;
	direccionServidor.sin_addr.s_addr= inet_addr(get_local_IP()); //INADDR_ANY;
	direccionServidor.sin_port=htons(config_get_int_value(config,"LISTEN_PORT"));


	int servidor = socket(AF_INET, SOCK_STREAM , 0);
	sem_wait(&logger_semaphore);
	log_info(logger, "Levantamos el servidor");
	sem_post(&logger_semaphore);

	int activado = 1;
	setsockopt(servidor, SOL_SOCKET, SO_REUSEADDR, &activado, sizeof(activado));

	if(bind(servidor, (void*) &direccionServidor, sizeof(direccionServidor)) != 0){
		sem_wait(&logger_semaphore);
		log_info(logger, "Fallo el bind");
		sem_post(&logger_semaphore);
		perror("Fallo el bind");
		exit(1);
	}

	int cliente;

	sem_wait(&logger_semaphore);
	log_info(logger, "Servidor listo para recibir un cliente");
	sem_post(&logger_semaphore);

	listen(servidor, 100);
	pthread_t *hilo;
	struct sockaddr_in direccionCliente;
	unsigned tamanioDireccion= sizeof(struct sockaddr_in);
	cliente = accept(servidor, (void*) &direccionCliente, &tamanioDireccion);

	while(1){
		sem_wait(&logger_semaphore);
		log_info(logger, "Recibimos un cliente");
		sem_post(&logger_semaphore);
		pthread_create(&hilo, NULL,  (void*)atenderCliente, cliente);
		pthread_detach(hilo);
		cliente = accept(servidor, (void*) &direccionCliente, &tamanioDireccion);
	}
}

void* recibirBuffer(int* alocador, int cliente){

	void* buffer;

	int recv_result = recv(cliente, alocador, 4, MSG_WAITALL);

	if(recv_result > 0){
		buffer = malloc(*alocador);
		recv(cliente, buffer,*alocador, MSG_WAITALL);
		return buffer;
	} else if(recv_result == 0){
		*alocador=0;
		return buffer;
	}

	//Si da menor a 0 es porque el cliente se desconecto mal (seg_fault o algo asi).
	//Entonces, lo saco de prepo del sistema. Deberia chequear si, en una de esas, no salio antes?
	if(recv_result < 0) {
		CLIENT_LEFT_THE_SYSTEM(cliente);
		LOG_METRICS(cliente);
		return buffer; //Creo que tambien hay que retornarlo aca
	}
}

void realizarRequest(void *buffer, int cliente){

	bool falseStatus = 0;
	bool trueStatus = 1;

	int cod_op;
	memcpy(&cod_op, buffer, sizeof(int));

	switch(cod_op){

	//Variables que uso en todos los cases
	int offset= sizeof(int);
	int longitudDelSiguiente=0;
	void* buffer;
	size_t n;
	client* client = FIND_CLIENT_BY_SOCKET(cliente);

	//init
	case 100:
		;
		char* IP_ID;

		memcpy(&longitudDelSiguiente, (buffer + offset), sizeof(int));
		IP_ID=(char*)malloc(longitudDelSiguiente);
		offset= offset+sizeof(int);
		memcpy(IP_ID, (buffer + offset), longitudDelSiguiente);

		//MUSE YO TE INVOCO
		ADD_CLIENT_TO_LIST(IP_ID, cliente);

		int resultado = CREATE_ADDRESS_SPACE(IP_ID);

		//TODO: crearle los semaforos particulares de este proceso

		buffer=(void*)malloc(sizeof(int));
		memcpy(buffer,&resultado,sizeof(int));

		send(cliente, buffer, sizeof(buffer),0);

		free(buffer);
		break;

		//close
	case 101:
		CLIENT_LEFT_THE_SYSTEM(cliente);
		LOG_METRICS(cliente);
		// Agrego al struct _client_ 2 int que sirven para las metricas

		break;

		//alloc
	case 102:
		;
		uint32_t bytes_a_reservar;

		memcpy(&longitudDelSiguiente, (buffer + offset), sizeof(int));
		offset= offset+sizeof(int);
		memcpy(&bytes_a_reservar, (buffer + offset), longitudDelSiguiente);

		int debe_crearse_segmento_flag = 0;
		int se_pudo_reservar_flag = 0;
		void* pointer;
		uint32_t segment_base;

		//MUSE YO TE INVOCO
		addressSpace* client_address_space = GET_ADDRESS_SPACE(cliente);

		if(memory_left >= bytes_a_reservar) {
			if(THERE_ARE_EXISTING_HEAP_SEGMENTS(client_address_space)) {
				//El proceso que pide ya tiene segmentos de heap
				void usar_segmento_si_tiene_espacio(segment* un_segmento) {
					if(!se_pudo_reservar_flag) {
						sem_wait(&mp_semaphore);
						pointer = SEGMENT_IS_BIG_ENOUGH(un_segmento, bytes_a_reservar + 5); //Porque quiero guardar la ultima metadata
						if(pointer != NULL) {
							//El segmento tiene lugar
							uint32_t bytes_que_habia;
							uint32_t bytes_sobrantes;
							memcpy(&bytes_que_habia, pointer - 5, sizeof(uint32_t));
							//Sobreescribo la metadata
							memcpy(pointer - 5, &bytes_a_reservar, sizeof(uint32_t));
							memcpy(pointer - 1, &falseStatus, sizeof(bool));
							//Escribo la nueva metadata
							bytes_sobrantes = bytes_que_habia - bytes_a_reservar - 5;
							memcpy(pointer + bytes_a_reservar, &bytes_sobrantes, sizeof(uint32_t));
							memcpy(pointer + bytes_a_reservar + sizeof(uint32_t), &trueStatus, sizeof(bool));
							se_pudo_reservar_flag = 1;
							segment_base = un_segmento->base;
						}
						sem_post(&mp_semaphore);
					}
				}

				void usar_segmento_si_se_puede_agrandar(segment* un_segmento) {
					if(!se_pudo_reservar_flag) {
						sem_wait(&mp_semaphore);
						if(SEGMENT_CAN_BE_EXTENDED(un_segmento, client_address_space, bytes_a_reservar + 5)) {
							//Puedo agrandar el segmento
							void* last_metadata = GET_LAST_METADATA(un_segmento);
							pointer = last_metadata + 5;
							uint32_t internal_fragmentation;
							memcpy(&internal_fragmentation, last_metadata, sizeof(uint32_t));
							int frames_to_require;
							if((bytes_a_reservar + 5 - internal_fragmentation) % page_size > 0) {
								frames_to_require = ((bytes_a_reservar + 5 - internal_fragmentation) / page_size) + 1;
							} else {
								frames_to_require = (bytes_a_reservar + 5 - internal_fragmentation) / page_size;
							}

							uint32_t bytes_que_habia = 0;
							uint32_t bytes_sobrantes = 0;
							uint32_t bytes_que_quedan = 0;
							memcpy(&bytes_que_habia, last_metadata, sizeof(uint32_t));
							//Sobreescribo la metadata
							memcpy(last_metadata, &bytes_a_reservar, sizeof(uint32_t));
							memcpy(last_metadata + sizeof(uint32_t), &falseStatus, sizeof(bool));

							bytes_que_quedan = bytes_a_reservar - internal_fragmentation;

							pageFrame* last_page;

							for(int i=0; i < frames_to_require; i++) {
								int frame_number = CLOCK();
								pageFrame* new_page = (pageFrame*)malloc(sizeof(pageFrame));
								new_page->modifiedBit = 1; //Not sure
								new_page->presenceBit = 1;
								list_add(un_segmento->pageFrameTable, new_page);
								if(bytes_que_quedan - page_size > 0)
									bytes_que_quedan = bytes_que_quedan - page_size;
								last_page = new_page;
							}

							//Escribo la nueva metadata
							bytes_sobrantes = page_size - bytes_que_quedan - 5;
							void* pointer = GET_FRAME_POINTER(last_page->frame_number);
							memcpy(pointer + bytes_que_quedan, &bytes_sobrantes, sizeof(uint32_t));
							memcpy(pointer + bytes_que_quedan + sizeof(uint32_t), &trueStatus, sizeof(bool));

							se_pudo_reservar_flag = 1;
							segment_base = un_segmento->base;
						}
						sem_post(&mp_semaphore);
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
				sem_wait(&mp_semaphore);
				//Crear nuevo segmento
				if(client_address_space->segment_table->elements_count) {
					//Ya hay segmentos
					segment* new_segment = (segment*)malloc(sizeof(segment*));
					new_segment->isHeap = true;
					new_segment->pageFrameTable = list_create();
					int frames_to_require;
					if((bytes_a_reservar + 10) % page_size > 0) {
						frames_to_require = ((bytes_a_reservar + 10) / page_size) + 1;
					} else {
						frames_to_require = (bytes_a_reservar + 10) / page_size;
					}

					pageFrame* last_page;
					uint32_t bytes_que_quedan = bytes_a_reservar;
					uint32_t bytes_sobrantes = 0;

					for(int i=0; i < frames_to_require; i++) {
						int frame_number = CLOCK();
						if(i == 0) {
							//Escribo la primer metadata
							pointer = GET_FRAME_POINTER(frame_number);
							memcpy(pointer, &bytes_a_reservar, sizeof(uint32_t));
							memcpy(pointer + sizeof(uint32_t), &falseStatus, sizeof(bool));
							pointer = pointer + 5;
						}
						pageFrame* new_page = (pageFrame*)malloc(sizeof(pageFrame));
						new_page->modifiedBit = 1; //Not sure
						new_page->presenceBit = 1;
						list_add(new_segment->pageFrameTable, new_page);
						if(bytes_que_quedan - page_size > 0)
							bytes_que_quedan = bytes_que_quedan - page_size;
						last_page = new_page;
					}

					//Escribo la nueva metadata
					bytes_sobrantes = page_size - bytes_que_quedan - 5;
					void* pointer = GET_FRAME_POINTER(last_page->frame_number);
					memcpy(pointer + bytes_que_quedan, &bytes_sobrantes, sizeof(uint32_t));
					memcpy(pointer + bytes_que_quedan + sizeof(uint32_t), &trueStatus, sizeof(bool));

					new_segment->size = new_segment->pageFrameTable->elements_count * page_size;
					new_segment->base = FIRST_FIT(client_address_space->segment_table, 0, new_segment->size);

					list_add(client_address_space->segment_table, new_segment);
					segment_base = new_segment->base;
				} else {
					//Es el primer segmento
					segment* new_segment = (segment*)malloc(sizeof(segment*));
					new_segment->base = 0;
					new_segment->isHeap = true;
					new_segment->pageFrameTable = list_create();
					int frames_to_require;
					if((bytes_a_reservar + 10) % page_size > 0) {
						frames_to_require = ((bytes_a_reservar + 10) / page_size) + 1;
					} else {
						frames_to_require = (bytes_a_reservar + 10) / page_size;
					}

					pageFrame* last_page;
					uint32_t bytes_que_quedan = bytes_a_reservar;
					uint32_t bytes_sobrantes = 0;

					for(int i=0; i < frames_to_require; i++) {
						int frame_number = CLOCK();
						if(i == 0) {
							//Escribo la primer metadata
							void* pointer = GET_FRAME_POINTER(frame_number);
							memcpy(pointer, &bytes_a_reservar, sizeof(uint32_t));
							memcpy(pointer + sizeof(uint32_t), &falseStatus, sizeof(bool));
							pointer = pointer + 5;
						}
						pageFrame* new_page = (pageFrame*)malloc(sizeof(pageFrame));
						new_page->modifiedBit = 1; //Not sure
						new_page->presenceBit = 1;
						list_add(new_segment->pageFrameTable, new_page);
						if(bytes_que_quedan - page_size > 0)
							bytes_que_quedan = bytes_que_quedan - page_size;
						last_page = new_page;
					}

					//Escribo la nueva metadata
					bytes_sobrantes = page_size - bytes_que_quedan - 5;
					void* pointer = GET_FRAME_POINTER(last_page->frame_number);
					memcpy(pointer + bytes_que_quedan, &bytes_sobrantes, sizeof(uint32_t));
					memcpy(pointer + bytes_que_quedan + sizeof(uint32_t), &trueStatus, sizeof(bool));

					new_segment->size = new_segment->pageFrameTable->elements_count * page_size;

					list_add(client_address_space->segment_table, new_segment);
					segment_base = new_segment->base;
				}
				sem_post(&mp_semaphore);
				client->last_requested_segment_base = segment_base;
			}
		} else {
			//No pude escribir porque no hay memoria
			//Existe esta condicion? O agarro y mando un frame a swap y chau?
		}
		uint32_t displacement_until_page = 0;
		sem_wait(&mp_semaphore);
		uint32_t page_offset = GET_OFFSET_FROM_POINTER(pointer);
		int frame_number = GET_FRAME_NUMBER_FROM_POINTER(pointer);
		sem_post(&mp_semaphore);
		int iterator = 0;
		uint32_t virtual_direction;

		segment* a_segment = GET_SEGMENT_FROM_BASE(segment_base, client_address_space);
		while(a_segment->pageFrameTable->elements_count > iterator) {
			pageFrame* a_page = list_get(a_segment->pageFrameTable, iterator);
			if(a_page->frame_number == frame_number)
				break;
			iterator++;
			displacement_until_page = displacement_until_page + page_size;
		}
		virtual_direction = a_segment->base + displacement_until_page + page_offset;


		//////////// END MUSE

		buffer=(void*)malloc(sizeof(uint32_t));
		memcpy(buffer, &virtual_direction, sizeof(uint32_t));

		send(cliente, buffer, sizeof(buffer),0);

		client->total_memory_requested += bytes_a_reservar;

		LOG_METRICS(cliente);

		free(buffer);
		break;

		//free
	case 103:
		;
		uint32_t dir;

		memcpy(&longitudDelSiguiente, (buffer + offset), sizeof(int));
		offset= offset+sizeof(int);
		memcpy(&dir, (buffer + offset), longitudDelSiguiente);

		addressSpace* address_space = GET_ADDRESS_SPACE(cliente);
		sem_wait(&mp_semaphore);
		int bytes_freed = FREE_USED_FRAME(dir , address_space);
		sem_post(&mp_semaphore);
		client->total_memory_freed += bytes_freed;

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
		;
		void* dst;
		uint32_t src;

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
		addressSpace* addr_sp = GET_ADDRESS_SPACE(cliente);
		sem_wait(&mp_semaphore);
		sem_wait(&mapped_files_semaphore); //Siempre agarrar los semaforos asi para evitar potenciales deadlocks. Si llega a haber, revisar aca
		void* data = GET_N_BYTES_DATA_FROM_MUSE(addr_sp , src, n);
		sem_post(&mp_semaphore);
		sem_post(&mapped_files_semaphore);
		buffer=(void*)malloc(n);
		memcpy(buffer, data, n);

		send(cliente, buffer, sizeof(buffer),0);

		free(buffer);
		free(dst);
		break;

		//cpy
	case 105:
		;
		uint32_t dest;
		void* source;

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
		;
		char* path;
		size_t length;
		int flag;

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
		memcpy(&flag, (buffer + offset), longitudDelSiguiente);
		offset= offset+longitudDelSiguiente;

		//MUSE YO TE INVOCO
		addressSpace* cli_address_space = GET_ADDRESS_SPACE(cliente);
		sem_wait(&mapped_files_semaphore);
		if(FILE_ALREADY_MAPPED(path)) {
			mappedFile* mapped_file = GET_MAPPED_FILE(path);
			mapped_file->references++;
		} else {
			mappedFile* new_map = (mappedFile*)malloc(sizeof(mappedFile)); //Struct a agregar a la lista
			client = FIND_CLIENT_BY_SOCKET(cliente); //Para sacar el id
			char* mapped_file = malloc(length); //Lo que tendra el return de mmap
			new_map->path=(char*)malloc(sizeof(path));
			memcpy(new_map->path, path, sizeof(path));
			new_map->owner=(char*)malloc(sizeof(client->clientProcessId));
			memcpy(new_map->owner, client->clientProcessId, sizeof(client->clientProcessId));
			new_map->flag=flag;
			new_map->length=length;
			new_map->references = 1;

			//Mapeo posta posta el archivo. Deberia chequear si existe?
			int file_desc = open(path, O_RDWR, S_IRWXU | S_IWOTH | S_IROTH); //Los ultimos dos flags son para 'others'
			new_map->file_desc = file_desc;
			mapped_file = mmap(NULL, length, PROT_READ | PROT_WRITE, flag, file_desc, 0);
			new_map->pointer = mapped_file;

			list_add(mapped_files, new_map); //Agrego el mapeo a la lista global
		}
		sem_post(&mapped_files_semaphore);

		//Siguientes pasos:
		//1. Rellenar el espacio que sobre del archivo con \0 (Esto no lo hace de por si mmap?) --> "https://stackoverflow.com/questions/47604431/why-we-can-mmap-to-a-file-but-exceed-the-file-size"
		//2. Crear el segmento... (FIRST_FIT)
		segment* new_segment = (segment*)malloc(sizeof(segment*));
		new_segment->isHeap = false;
		new_segment->pageFrameTable = list_create();
		//a. Tabla de paginas con todas en presence = 0
		int frames_to_require;
		if(length % page_size > 0) {
			frames_to_require = (length / page_size) + 1;
		} else {
			frames_to_require = length / page_size;
		}

		for(int i=0; i < frames_to_require; i++) {
			sem_wait(&mp_semaphore);
			int frame_number = CLOCK(); //Ver issue #28
			sem_post(&mp_semaphore);
			pageFrame* new_page = (pageFrame*)malloc(sizeof(pageFrame));
			new_page->modifiedBit = 0; //Casi seguro que es 0
			new_page->useBit = 0;
			new_page->presenceBit = 0;
			list_add(new_segment->pageFrameTable, new_page);
		}

		new_segment->path = (char*)malloc(sizeof(path));
		memcpy(new_segment->path, path, sizeof(path));
		new_segment->size = new_segment->pageFrameTable->elements_count * page_size;
		new_segment->base = FIRST_FIT(cli_address_space->segment_table, 0, new_segment->size);
		list_add(client_address_space->segment_table, new_segment);

		buffer=(void*)malloc(sizeof(uint32_t));
		memcpy(buffer, &new_segment->base, sizeof(uint32_t));

		send(cliente, buffer, sizeof(buffer),0);

		free(buffer);
		break;

		//sync
	case 107:
		;
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
		addressSpace* address_space_client = GET_ADDRESS_SPACE(cliente);
		segment* segment_requested = GET_SEGMENT_FROM_ADDRESS(addr, address_space_client);
		int writen_pages = 0;

		sem_wait(&mp_semaphore);
		sem_wait(&mapped_files_semaphore);
		if(segment_requested != NULL) {
			if(segment_requested->isHeap) {
				//ERROR: no es un segmento de map
			} else {
				if(addr + len > segment_requested->base + segment_requested->size) {
					//Quiere escribir mas bytes del tamanio del archivo/segmento
					//escribo tutti lo que puedo y luego rompo? No escribo nada y rompo?
					//escribo lo que pueda y no chillo?
					//En caso de romper, seguramente tire SEG FAULT (porque me zarpe del limite)
				} else {
					//Do your job buddy...
					int pages_to_write;
					if(len % page_size > 0) {
						pages_to_write = (len / page_size) + 1;
					} else {
						pages_to_write = (len / page_size);
					}
					//Las paginas que no estan presentes, las salteo
					//A las que copio al archivo, les pongo el bit de modificado en 0
					//Bit de uso en 1?
					mappedFile* mapped_file = GET_MAPPED_FILE(segment_requested->path);
					int bytes_traveled = addr - segment_requested->base;
					if(bytes_traveled % page_size > 0) //Si arranco corrido en la pagina
						pages_to_write++;
					int offset = bytes_traveled;
					while(writen_pages < pages_to_write) {
						int current_frame = GET_FRAME_FROM_ADDRESS(bytes_traveled, segment_requested);
						void* pointer = GET_FRAME_POINTER(current_frame);
						for(int i = (offset % page_size); i < page_size ; i++) {
							mapped_file->pointer[bytes_traveled] = (char*)(pointer + i);
							//Este casteo magico esta bien o hay que hacer memcpy al archivo mappeado?
							bytes_traveled++;
						}
						offset = 0;
						writen_pages++;
					}
				}
			}
		} else {
			//seg_fault porque la direccion pedida no es valida
		}
		sem_post(&mp_semaphore);
		sem_post(&mapped_files_semaphore);
		resultado = 0;//(en los casos feos lo pongo en -1... y el seg fault no entonces?)
		buffer=(void*)malloc(sizeof(uint32_t));
		memcpy(buffer, &resultado, sizeof(uint32_t));

		send(cliente, buffer, sizeof(buffer),0);

		free(buffer);
		break;

		//unmap
	case 108:
		;
		uint32_t direc;

		memcpy(&longitudDelSiguiente, (buffer + offset), sizeof(int));
		offset= offset+sizeof(int);
		memcpy(&direc, (buffer + offset), longitudDelSiguiente);
		offset= offset+longitudDelSiguiente;
		int res;

		//MUSE YO TE INVOCO
		addressSpace* addr_space = GET_ADDRESS_SPACE(cliente);
		segment* a_segm = GET_SEGMENT_FROM_BASE(direc,addr_space);
		//Controlar que la direccion que me pasan este OK
		sem_wait(&mapped_files_semaphore);
		mappedFile* mapped_file = GET_MAPPED_FILE(a_segm->path);
		mapped_file->references--;
		if(!mapped_file->references) {
			close(mapped_file->file_desc);
			munmap(mapped_file->pointer, mapped_file->length);
			//Borrar entrada de la lista mapped_files
			int index = GET_MAPPED_FILE_INDEX(mapped_file->path);
			list_remove_and_destroy_element(mapped_files, index, DESTROY_MAPPED_FILE);
		}
		sem_post(&mapped_files_semaphore);

		sem_wait(&mp_semaphore);
		//Libero los frames que tenga tomados en memoria
		//TODO - ver CLIENT_LEFT_THE_SYSTEM
		sem_post(&mp_semaphore);

		//Borro el segmento
		int index = GET_SEGMENT_INDEX(addr_space->segment_table, a_segm->base);
		list_remove_and_destroy_element(addr_space->segment_table, index, DESTROY_SEGMENT);

		buffer=(void*)malloc(sizeof(uint32_t));
		memcpy(buffer, &res, sizeof(uint32_t));

		send(cliente, buffer, sizeof(buffer),0);

		free(buffer);
		break;

	default:
		break;
	}

	sem_wait(&logger_semaphore);
	log_info(logger,"Terminamos el request\n");
	sem_post(&logger_semaphore);
}

