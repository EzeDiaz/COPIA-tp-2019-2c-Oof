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
	direccionServidor.sin_addr.s_addr= inet_addr("127.0.0.1"); //INADDR_ANY; //127.0.0.1 //get_local_IP()
	direccionServidor.sin_port=htons(config_get_int_value(config,"LISTEN_PORT"));


	int servidor = socket(AF_INET, SOCK_STREAM , 0);
	sem_wait(&logger_semaphore);
	log_info(logger, "Levantamos el servidor con la ip %s", get_local_IP());
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
	sem_wait(&logger_semaphore);
	log_info(logger,"Recv al recibirBuffer dio %d", recv_result);
	sem_post(&logger_semaphore);


	if(recv_result > 0){
		buffer = malloc(*alocador);
		recv(cliente, buffer,*alocador, MSG_WAITALL);
		return buffer;
	} else if(recv_result == 0){ //Tanto en seg fault como otro error igual devuelve 0
		*alocador=0;
		client* cliente_found = FIND_CLIENT_BY_SOCKET(cliente);
		if(cliente_found != NULL) {
			sem_wait(&mp_semaphore);
			sem_wait(&mapped_files_semaphore);
			sem_wait(&logger_semaphore);
			LOG_METRICS(cliente);
			sem_post(&logger_semaphore);
			sem_post(&mp_semaphore);
			sem_post(&mapped_files_semaphore);
			sem_wait(&logger_semaphore);
			log_info(logger,"El cliente %d dejo el sistema pero no hizo muse_close, lo saco de prepo", cliente);
			sem_post(&logger_semaphore);
			CLIENT_LEFT_THE_SYSTEM(cliente);
		}
		return buffer;
	}

	//Si da menor a 0 es porque el cliente se desconecto mal (seg_fault o algo asi).
	//Entonces, lo saco de prepo del sistema. Deberia chequear si, en una de esas, no salio antes?
	//Will it ever come here?
	if(recv_result < 0) {
		CLIENT_LEFT_THE_SYSTEM(cliente);
		sem_wait(&mp_semaphore);
		sem_wait(&mapped_files_semaphore);
		sem_wait(&logger_semaphore);
		LOG_METRICS(cliente);
		sem_post(&logger_semaphore);
		sem_post(&mp_semaphore);
		sem_post(&mapped_files_semaphore);
		return buffer; //Creo que tambien hay que retornarlo aca
	}

	return NULL;
}

void realizarRequest(void *buffer_recibido, int cliente){

	bool falseStatus = 0;
	bool trueStatus = 1;

	int cod_op;
	memcpy(&cod_op, buffer_recibido, sizeof(int));

	//Variables que uso en todos los cases
	int offset= sizeof(int);
	int longitudDelSiguiente=0;
	void* buffer;
	size_t n;
	client* client = FIND_CLIENT_BY_SOCKET(cliente);

	switch(cod_op){

	//init
	case 100:
		;
		char* IP_ID;

		memcpy(&longitudDelSiguiente, (buffer_recibido + offset), sizeof(int));
		IP_ID=(char*)malloc(longitudDelSiguiente);
		offset= offset+sizeof(int);
		memcpy(IP_ID, (buffer_recibido + offset), longitudDelSiguiente);

		//MUSE YO TE INVOCO
		ADD_CLIENT_TO_LIST(IP_ID, cliente);

		int resultado = CREATE_ADDRESS_SPACE(IP_ID);

		//Semaforo del adress space del cliente
		client = FIND_CLIENT_BY_SOCKET(cliente);
		sem_init(&client->client_sempahore,0,1);

		buffer=(void*)malloc(sizeof(int));
		memcpy(buffer,&resultado,sizeof(int));

		send(cliente, buffer, sizeof(buffer),0);

		sem_wait(&logger_semaphore);
		log_info(logger,"Se inicializo el cliente %d (muse_init)", cliente);
		sem_post(&logger_semaphore);

		free(buffer);
		free(IP_ID);
		break;

		//close
	case 101:
		sem_wait(&mp_semaphore);
		sem_wait(&logger_semaphore);
		LOG_METRICS(cliente);
		CLIENT_LEFT_THE_SYSTEM(cliente);
		log_info(logger,"Cliente %d dejo el sistema (muse_close)", cliente);
		sem_post(&logger_semaphore);
		sem_post(&mp_semaphore);

		// Agrego al struct _client_ 2 int que sirven para las metricas

		break;

		//alloc
	case 102:
		;
		uint32_t bytes_a_reservar;

		memcpy(&longitudDelSiguiente, (buffer_recibido + offset), sizeof(int));
		offset= offset+sizeof(int);
		memcpy(&bytes_a_reservar, (buffer_recibido + offset), longitudDelSiguiente);

		int debe_crearse_segmento_flag = 0;
		int se_pudo_reservar_flag = 0;
		void* pointer;
		uint32_t segment_base;

		//MUSE YO TE INVOCO
		sem_wait(&addresses_space_semaphore);
		addressSpace* client_address_space = GET_ADDRESS_SPACE(cliente);
		sem_post(&addresses_space_semaphore);

		sem_wait(&logger_semaphore);
		log_info(logger,"El cliente %d pidio memoria, laburamos... (muse_alloc)", cliente);
		sem_post(&logger_semaphore);
		sem_wait(&client->client_sempahore);

		uint32_t virtual_direction = ALLOC(client_address_space, bytes_a_reservar);

		sem_post(&client->client_sempahore);
		//////////// END MUSE

		buffer=(void*)malloc(sizeof(uint32_t));
		memcpy(buffer, &virtual_direction, sizeof(uint32_t));

		send(cliente, buffer, sizeof(buffer),0);

		client->total_memory_requested += bytes_a_reservar;

		sem_wait(&mp_semaphore);
		sem_wait(&mapped_files_semaphore);
		sem_wait(&logger_semaphore);
		LOG_METRICS(cliente);
		sem_post(&logger_semaphore);
		sem_post(&mp_semaphore);
		sem_post(&mapped_files_semaphore);

		free(buffer);
		break;

		//free
	case 103:
		;
		uint32_t dir;

		memcpy(&longitudDelSiguiente, (buffer_recibido + offset), sizeof(int));
		offset= offset+sizeof(int);
		memcpy(&dir, (buffer_recibido + offset), longitudDelSiguiente);

		//MUSE YO TE INVOCO
		sem_wait(&addresses_space_semaphore);
		addressSpace* address_space = GET_ADDRESS_SPACE(cliente);
		sem_post(&addresses_space_semaphore);
		sem_wait(&mp_semaphore);
		sem_wait(&client->client_sempahore);
		int bytes_freed = FREE_USED_FRAME(dir , address_space);
		sem_post(&client->client_sempahore);
		sem_post(&mp_semaphore);
		if(bytes_freed >= 0)
			client->total_memory_freed += bytes_freed;


		uint32_t respuesta = (uint32_t) bytes_freed; //FREE_USED_FRAME puede devolver -1

		buffer=(void*)malloc(sizeof(uint32_t));
		memcpy(buffer, &respuesta, sizeof(uint32_t));

		send(cliente, buffer, sizeof(buffer),0);

		free(buffer);

		sem_wait(&logger_semaphore);
		log_info(logger,"Free realizado con retorno %d", respuesta);
		sem_post(&logger_semaphore);

		break;

		//get
	case 104:
		;
		//void* dst;
		uint32_t src;

		/* NO MANDO EL dst
		memcpy(&longitudDelSiguiente, (buffer_recibido + offset), sizeof(int));
		offset= offset+sizeof(int);
		dst = (void*)malloc(longitudDelSiguiente);
		memcpy(dst, (buffer_recibido + offset), longitudDelSiguiente);
		offset= offset+longitudDelSiguiente;
		 */

		memcpy(&longitudDelSiguiente, (buffer_recibido + offset), sizeof(int));
		offset= offset+sizeof(int);
		memcpy(&src, (buffer_recibido + offset), longitudDelSiguiente);
		offset= offset+longitudDelSiguiente;

		memcpy(&longitudDelSiguiente, (buffer_recibido + offset), sizeof(int));
		offset= offset+sizeof(int);
		memcpy(&n, (buffer_recibido + offset), longitudDelSiguiente);
		offset= offset+longitudDelSiguiente;

		sem_wait(&logger_semaphore);
		log_info(logger,"El cliente %d leyo %d bytes (muse_get)", cliente, n);
		sem_post(&logger_semaphore);

		//MUSE YO TE INVOCO
		sem_wait(&addresses_space_semaphore);
		addressSpace* addr_sp = GET_ADDRESS_SPACE(cliente);
		sem_post(&addresses_space_semaphore);
		sem_wait(&mp_semaphore);
		sem_wait(&mapped_files_semaphore); //Siempre agarrar los semaforos asi para evitar potenciales deadlocks. Si llega a haber, revisar aca
		sem_wait(&client->client_sempahore);
		void* data = GET_N_BYTES_DATA_FROM_MUSE_V2(addr_sp , src, n);
		sem_post(&client->client_sempahore);
		sem_post(&mp_semaphore);
		sem_post(&mapped_files_semaphore);

		if(data == NULL) {
			//Si es NULL es porque no pudo leer, entonces seteo tutti el data en '\0'
			memset(data, '\0', n); //Pongo tutti el data en \0. Sera mala idea?
		}

		buffer=(void*)malloc(n);
		memcpy(buffer, data, n);

		send(cliente, buffer, n, 0);

		free(buffer);
		//free(dst);
		free(data); //Ojo con este

		sem_wait(&logger_semaphore);
		log_info(logger,"Get realizado");
		sem_post(&logger_semaphore);

		break;

		//cpy
	case 105:
		;
		uint32_t dest;
		void* source;

		memcpy(&longitudDelSiguiente, (buffer_recibido + offset), sizeof(int));
		offset= offset+sizeof(int);
		memcpy(&dest, (buffer_recibido + offset), longitudDelSiguiente);
		offset= offset+sizeof(uint32_t);

		memcpy(&longitudDelSiguiente, (buffer_recibido + offset), sizeof(int));
		offset= offset+sizeof(int);
		source = (void*)malloc(longitudDelSiguiente);
		memcpy(source, (buffer_recibido + offset), longitudDelSiguiente);
		offset= offset+longitudDelSiguiente;

		n = longitudDelSiguiente;

		/* POR COMO ARMO EL PAQUETE, ACA YA ESTOY METIENDO MANO EN LUGARES RAROS
		memcpy(&longitudDelSiguiente, (buffer_recibido + offset), sizeof(int));
		offset= offset+sizeof(int);
		memcpy(&n, (buffer_recibido + offset), longitudDelSiguiente);
		offset= offset+longitudDelSiguiente;

		n = longitudDelSiguiente;
		 */
		sem_wait(&logger_semaphore);
		log_info(logger,"El cliente %d escribio %d bytes", cliente, n);
		sem_post(&logger_semaphore);

		//MUSE YO TE INVOCO
		sem_wait(&addresses_space_semaphore);
		addressSpace* addrs_spc = GET_ADDRESS_SPACE(cliente);
		sem_post(&addresses_space_semaphore);
		sem_wait(&mp_semaphore);
		sem_wait(&mapped_files_semaphore);
		sem_wait(&client->client_sempahore);
		int retorno = WRITE_N_BYTES_DATA_TO_MUSE_V2(dest, addrs_spc, n, source);
		sem_post(&client->client_sempahore);
		sem_post(&mp_semaphore);
		sem_post(&mapped_files_semaphore);

		buffer=(void*)malloc(sizeof(int));
		memcpy(buffer, &retorno, sizeof(int));

		send(cliente, buffer, sizeof(buffer),0);

		free(buffer);
		free(source);

		sem_wait(&logger_semaphore);
		log_info(logger,"Cpy realizado");
		sem_post(&logger_semaphore);

		break;

		//map
	case 106:
		;
		char* path;
		size_t length;
		int flag;

		memcpy(&longitudDelSiguiente, (buffer_recibido + offset), sizeof(int));
		offset= offset+sizeof(int);
		path = (char*)malloc(longitudDelSiguiente);
		memcpy(path, (buffer_recibido + offset), longitudDelSiguiente);
		offset= offset+longitudDelSiguiente;

		memcpy(&longitudDelSiguiente, (buffer_recibido + offset), sizeof(int));
		offset= offset+sizeof(int);
		memcpy(&length, (buffer_recibido + offset), longitudDelSiguiente);
		offset= offset+longitudDelSiguiente;

		memcpy(&longitudDelSiguiente, (buffer_recibido + offset), sizeof(int));
		offset= offset+sizeof(int);
		memcpy(&flag, (buffer_recibido + offset), longitudDelSiguiente);
		offset= offset+longitudDelSiguiente;

		sem_wait(&logger_semaphore);
		log_info(logger,"El cliente %d mapeo el archivo %s a memoria (muse_map)", cliente, path);
		sem_post(&logger_semaphore);

		//MUSE YO TE INVOCO
		sem_wait(&addresses_space_semaphore);
		addressSpace* cli_address_space = GET_ADDRESS_SPACE(cliente);
		sem_post(&addresses_space_semaphore);
		sem_wait(&mapped_files_semaphore);
		sem_wait(&client->client_sempahore);
		if(FILE_ALREADY_MAPPED(path)) {
			mappedFile* mapped_file = GET_MAPPED_FILE(path);
			mapped_file->references++;
		} else {
			//Paso el length a un tamaño de pagina
			size_t length_true;
			if(length % page_size) {
				length_true = ((length / page_size) + 1)*page_size;
			} else {
				length_true = length;
			}

			mappedFile* new_map = (mappedFile*)malloc(sizeof(mappedFile)); //Struct a agregar a la lista
			client = FIND_CLIENT_BY_SOCKET(cliente); //Para sacar el id
			char* mapped_file = (char*) malloc(length_true); //Lo que tendra el return de mmap
			new_map->path=(char*)malloc(strlen(path)+1);
			memcpy(new_map->path, path, strlen(path)+1);
			new_map->owner=(char*)malloc(strlen(client->clientProcessId)+1);
			memcpy(new_map->owner, client->clientProcessId, strlen(client->clientProcessId)+1);
			new_map->flag=flag;
			new_map->length=length_true;
			new_map->references = 1;

			//Mapeo posta posta el archivo. Deberia chequear si existe?
			int file_desc = open(path, O_RDWR, S_IRWXU | S_IWOTH | S_IROTH); //Los ultimos dos flags son para 'others'
			//Como no se si el archivo es mas chico o mas grande de lo que me pidio
			//lo trunco y chau. Si era mas chico bien, y si no bueno, perdi datos
			ftruncate(file_desc, length_true);
			new_map->file_desc = file_desc;
			mapped_file = mmap(NULL, length_true, PROT_READ | PROT_WRITE, flag, file_desc, 0);
			new_map->pointer = mapped_file;

			list_add(mapped_files, new_map); //Agrego el mapeo a la lista global
		}
		sem_post(&mapped_files_semaphore);

		//Siguientes pasos:
		//1. Rellenar el espacio que sobre del archivo con \0 (Esto no lo hace de por si mmap?) --> "https://stackoverflow.com/questions/47604431/why-we-can-mmap-to-a-file-but-exceed-the-file-size"
		//2. Crear el segmento... (FIRST_FIT)
		segment* new_segment = (segment*)malloc(sizeof(segment));
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
			//int frame_number = CLOCK(); //Ver issue #28
			sem_post(&mp_semaphore);
			pageFrame* new_page = (pageFrame*)malloc(sizeof(pageFrame));
			new_page->modifiedBit = 0; //Casi seguro que es 0
			new_page->useBit = 0;
			new_page->presenceBit = 0;
			new_page->frame_number = i * page_size; //Lo seteo porque si no queda cualquier verdura y me rompe el BRING_FROM_SWAP
			list_add(new_segment->pageFrameTable, new_page);
			//dictionary_put(clock_table, string_itoa(frame_number), new_page);
			//Juli dijo que asignaramos frames de entrada, pero nos ha entrao la duda...
		}

		addressSpace* cli_add_sp = GET_ADDRESS_SPACE(cliente);

		new_segment->path = (char*)malloc(strlen(path)+1);
		memcpy(new_segment->path, path, strlen(path)+1);
		new_segment->size = new_segment->pageFrameTable->elements_count * page_size;
		new_segment->base = FIRST_FIT(cli_address_space->segment_table, 0, new_segment->size);
		list_add(cli_add_sp->segment_table, new_segment);
		sem_post(&client->client_sempahore);

		//////////////////
		buffer=(void*)malloc(sizeof(uint32_t));
		memcpy(buffer, &new_segment->base, sizeof(uint32_t));

		send(cliente, buffer, sizeof(buffer),0);

		free(buffer);
		free(path);
		break;

		//sync
	case 107:
		;
		uint32_t addr;
		size_t len;

		memcpy(&longitudDelSiguiente, (buffer_recibido + offset), sizeof(int));
		offset= offset+sizeof(int);
		memcpy(&addr, (buffer_recibido + offset), longitudDelSiguiente);
		offset= offset+longitudDelSiguiente;

		memcpy(&longitudDelSiguiente, (buffer_recibido + offset), sizeof(int));
		offset= offset+sizeof(int);
		memcpy(&len, (buffer_recibido + offset), longitudDelSiguiente);
		offset= offset+longitudDelSiguiente;

		sem_wait(&logger_semaphore);
		log_info(logger,"El cliente %d pidio actualizar %d bytes de su direccion virtual mapeada %d (muse_sync)", cliente, len, addr);
		sem_post(&logger_semaphore);

		//MUSE YO TE INVOCO
		sem_wait(&addresses_space_semaphore);
		addressSpace* address_space_client = GET_ADDRESS_SPACE(cliente);
		sem_post(&addresses_space_semaphore);
		sem_wait(&client->client_sempahore);
		segment* segment_requested = GET_SEGMENT_FROM_ADDRESS(addr, address_space_client);
		int writen_pages = 0;

		sem_wait(&mp_semaphore);
		sem_wait(&mapped_files_semaphore);
		if(segment_requested != NULL) {
			if(segment_requested->isHeap) {
				resultado = -1;
			} else {
				if(addr + len > segment_requested->base + segment_requested->size) {
					//Quiere escribir mas bytes del tamanio del archivo/segmento
					//escribo tutti lo que puedo y luego rompo? No escribo nada y rompo?
					//escribo lo que pueda y no chillo?
					//En caso de romper, seguramente tire SEG FAULT (porque me zarpe del limite)
					resultado = -1;
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
					int page_number = GET_PAGE_NUMBER_FROM_ADDRESS(addr, segment_requested);
					mappedFile* mapped_file = GET_MAPPED_FILE(segment_requested->path);
					int bytes_traveled = addr - segment_requested->base;
					if(bytes_traveled % page_size > 0) //Si arranco corrido en la pagina
						pages_to_write++;
					int offset = bytes_traveled;
					//Escribo de a paginas, no de a bytes
					while(writen_pages < pages_to_write) {
						pageFrame* current_page = list_get(segment_requested->pageFrameTable, page_number);
						if(current_page->presenceBit && current_page->modifiedBit) {
							int current_frame = current_page->frame_number;
							int mapped_file_offset = writen_pages * page_size;
							void* pointer = GET_FRAME_POINTER(current_frame);
							memcpy(mapped_file->pointer + mapped_file_offset, pointer, page_size);
							//TODO: deberia tambien liberar la pagina o se libera sola despues?
							FREE_MEMORY_FRAME_BITMAP(current_page->frame_number);
							/* DEPRECATED: lo dejo porque compilaba pero el comportamiento no era muy adecuado
							for(int i = (offset % page_size); i < page_size ; i++) {
								mapped_file->pointer[bytes_traveled] = (char*)(pointer + i);
								//Este casteo magico esta bien o hay que hacer memcpy al archivo mappeado?
								bytes_traveled++;
							}
							 */
						}
						offset = 0;
						writen_pages++;
						page_number++;
						current_page->modifiedBit = 0;
						current_page->useBit = 1;
					}
					resultado = 0;
				}
			}
		} else {
			//seg_fault porque la direccion pedida no es valida
			resultado = -1;
		}
		sem_post(&mp_semaphore);
		sem_post(&mapped_files_semaphore);
		sem_post(&client->client_sempahore);

		////////////////////////////////////

		buffer=(void*)malloc(sizeof(uint32_t));
		memcpy(buffer, &resultado, sizeof(uint32_t));

		send(cliente, buffer, sizeof(buffer),0);

		free(buffer);
		break;

		//unmap
	case 108:
		;
		uint32_t direc;

		memcpy(&longitudDelSiguiente, (buffer_recibido + offset), sizeof(int));
		offset= offset+sizeof(int);
		memcpy(&direc, (buffer_recibido + offset), longitudDelSiguiente);
		offset= offset+longitudDelSiguiente;
		int res;

		sem_wait(&logger_semaphore);
		log_info(logger,"El cliente %d unmapeo el archivo que estaba en la direccion virtual %d (muse_unmap)", cliente, direc);
		sem_post(&logger_semaphore);

		//MUSE YO TE INVOCO
		sem_wait(&addresses_space_semaphore);
		addressSpace* addr_space = GET_ADDRESS_SPACE(cliente);
		sem_post(&addresses_space_semaphore);

		sem_wait(&client->client_sempahore);
		segment* a_segm = GET_SEGMENT_FROM_BASE(direc,addr_space);
		//Controlar que la direccion que me pasan este OK
		if(a_segm != NULL) {
			if(a_segm->isHeap) {
				res = -1;
			} else {
				//OK
				sem_wait(&mapped_files_semaphore);
				mappedFile* mapped_file = GET_MAPPED_FILE(a_segm->path);

				//Hago igual que en sync, actualizo el archivo (ctrl+c - ctrl+v)
				for(int i = 0; a_segm->pageFrameTable->elements_count > i; i++) {
					pageFrame* current_page = list_get(a_segm->pageFrameTable, i);
					if(current_page->presenceBit && current_page->modifiedBit) {
						int current_frame = current_page->frame_number;
						int mapped_file_offset = i * page_size;
						void* pointer = GET_FRAME_POINTER(current_frame);
						memcpy(mapped_file->pointer + mapped_file_offset, pointer, page_size);
						//TODO: deberia tambien liberar la pagina o se libera sola despues?
						FREE_MEMORY_FRAME_BITMAP(current_page->frame_number);
					}
					current_page->useBit = 1; //Por que le pongo uso en 1? //TODO: Revisar bit
					current_page->modifiedBit = 0;
				}

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
				//Inspirado en CLIENT_LEFT_THE_SYSTEM
				void liberar_frames_de_memoria(pageFrame* a_page) {
					if(a_page->presenceBit) {
						FREE_MEMORY_FRAME_BITMAP(a_page->frame_number);
					}
				}
				list_iterate(a_segm->pageFrameTable, liberar_frames_de_memoria);
				sem_post(&mp_semaphore);

				//Borro el segmento
				int index = GET_SEGMENT_INDEX(addr_space->segment_table, a_segm->base);
				list_remove_and_destroy_element(addr_space->segment_table, index, DESTROY_SEGMENT);
				res = 0;
			}
		} else {
			//ERROR
			res = -1;
		}

		sem_post(&client->client_sempahore);
		////////////////////////////

		buffer=(void*)malloc(sizeof(uint32_t));
		memcpy(buffer, &res, sizeof(uint32_t));

		send(cliente, buffer, sizeof(buffer),0);

		free(buffer);
		break;

	default:
		break;
	}

	sem_wait(&logger_semaphore);
	log_info(logger,"Terminamos el request del cliente %d", cliente);
	sem_post(&logger_semaphore);
}

/* GOOD OL ALLOC

		sem_wait(&addresses_space_semaphore);
		addressSpace* client_address_space = GET_ADDRESS_SPACE(cliente);
		sem_post(&addresses_space_semaphore);

		sem_wait(&logger_semaphore);
		log_info(logger,"El cliente %d pidio memoria, laburamos... (muse_alloc)", cliente);
		sem_post(&logger_semaphore);
		sem_wait(&client->client_sempahore);
		if(memory_left >= bytes_a_reservar) {
			if(THERE_ARE_EXISTING_HEAP_SEGMENTS(client_address_space)) {
				//El proceso que pide ya tiene segmentos de heap
				void usar_segmento_si_tiene_espacio(segment* un_segmento) {
					if(!se_pudo_reservar_flag) {
						sem_wait(&mp_semaphore);
						pointer = SEGMENT_IS_BIG_ENOUGH(un_segmento, bytes_a_reservar + 5); //Porque quiero guardar la ultima metadata
						if(pointer != NULL) {
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
							sem_wait(&logger_semaphore);
							log_info(logger,"El segmento puede extenderse");
							sem_post(&logger_semaphore);
							//Puedo agrandar el segmento
							int page = 0;
							int bytes_current_frame = 0;
							int bytes_next_frame = 0;
							uint32_t internal_fragmentation;
							void* last_metadata = GET_LAST_METADATA(un_segmento, &page, &internal_fragmentation, &bytes_next_frame, &bytes_current_frame);
							pointer = last_metadata; // Ya recibo el pointer apuntando a donde escribir (salteando metadata)

							//Esto va a funcionar como mi flag para saber si mi metadata esta spliteada
							if(bytes_current_frame > 0 && bytes_next_frame > 0){
								//Sobreescribo metadata spliteada
								pageFrame* last_page = list_get(un_segmento->pageFrameTable, page);
								pageFrame* previous_to_last_page = list_get(un_segmento->pageFrameTable, page - 1);

								void* ptr_last_page = GET_FRAME_POINTER(last_page->frame_number);
								void* ptr_previous_to_last_page = GET_FRAME_POINTER(previous_to_last_page->frame_number);
								ptr_previous_to_last_page += page_size - bytes_current_frame; //bytes current frame son los bytes de la pag anterior

								void* buffer = malloc(5);
								memcpy(buffer, &bytes_a_reservar, sizeof(int));
								memcpy(buffer+bytes_a_reservar, &falseStatus, sizeof(bool));

								memcpy(ptr_previous_to_last_page, buffer, bytes_current_frame);
								memcpy(ptr_last_page, buffer+bytes_current_frame, bytes_next_frame);
							} else {
								//Sobreescribo metadata normal
								memcpy(last_metadata - 5, &bytes_a_reservar, sizeof(uint32_t));
								memcpy((last_metadata - 5) + sizeof(uint32_t), &falseStatus, sizeof(bool));
							}

							int frames_to_require;
							if((bytes_a_reservar + 5 - internal_fragmentation) % page_size > 0) {
								frames_to_require = ((bytes_a_reservar + 5 - internal_fragmentation) / page_size) + 1;
							} else {
								frames_to_require = (bytes_a_reservar + 5 - internal_fragmentation) / page_size;
							}

							/*
							uint32_t bytes_que_habia = 0;
							uint32_t bytes_sobrantes = 0;
							uint32_t bytes_que_quedan = 0;
							memcpy(&bytes_que_habia, last_metadata, sizeof(uint32_t));
							bytes_que_quedan = bytes_a_reservar + 5 - internal_fragmentation;

							pageFrame* last_page;

							sem_wait(&logger_semaphore);
							log_info(logger,"Solicito los frames que hagan falta");
							sem_post(&logger_semaphore);

							for(int i=0; i < frames_to_require; i++) {
								int frame_number = CLOCK();
								pageFrame* new_page = (pageFrame*)malloc(sizeof(pageFrame));
								new_page->modifiedBit = 0; //Not sure
								new_page->presenceBit = 1;
								new_page->frame_number = frame_number;
								list_add(un_segmento->pageFrameTable, new_page);
								/*
								int dif = (bytes_que_quedan - page_size);
								if(dif > 0) {
									bytes_que_quedan = bytes_que_quedan - page_size;
								}


								sem_wait(&logger_semaphore);
								log_info(logger,"Le doy frames a la nueva pagina");
								sem_post(&logger_semaphore);
								last_page = new_page;
								dictionary_put(clock_table, string_itoa(frame_number), new_page);
								sem_wait(&logger_semaphore);
								log_info(logger,"Agregue la pagina al clock");
								sem_post(&logger_semaphore);
							}

							uint32_t frag_int = page_size - 5 - (bytes_a_reservar - internal_fragmentation - (frames_to_require - 1) * page_size);

							if(frag_int > page_size - 5){
								//Escribo metadata spliteada
								pageFrame* final_page;
								pageFrame* previous_to_final_page;
								int final_page_metadata_bytes = bytes_a_reservar + 10 - page_size;
								int previous_to_final_page_metadata_bytes = 5 - final_page_metadata_bytes;
								uint32_t free_size = page_size - final_page_metadata_bytes;

								void* buffer = malloc(5);
								memcpy(buffer, &free_size, sizeof(uint32_t));
								memcpy(buffer+sizeof(uint32_t), &trueStatus, sizeof(bool));

								final_page = list_get(un_segmento->pageFrameTable,un_segmento->pageFrameTable->elements_count - 1);
								previous_to_final_page = list_get(un_segmento->pageFrameTable,un_segmento->pageFrameTable->elements_count - 2);

								void* ptr_to_final_page = GET_FRAME_POINTER(final_page->frame_number);
								void* ptr_to_previous_to_final_page = GET_FRAME_POINTER(previous_to_final_page->frame_number);
								ptr_to_previous_to_final_page += page_size - previous_to_final_page_metadata_bytes;

								memcpy(ptr_to_previous_to_final_page, buffer, previous_to_final_page_metadata_bytes);
								memcpy(ptr_to_final_page, buffer+previous_to_final_page_metadata_bytes, final_page_metadata_bytes);

								free(buffer);
							} else{
								//Escribo metadata normal
								memcpy(last_metadata - 5, &frag_int, sizeof(uint32_t));
								memcpy(last_metadata - 5 + sizeof(uint32_t), &trueStatus, sizeof(bool));
							}
							sem_wait(&logger_semaphore);
							log_info(logger,"Se escribe la nueva metadata");
							sem_post(&logger_semaphore);

							se_pudo_reservar_flag = 1;
							segment_base = un_segmento->base;
						}
						sem_post(&mp_semaphore);
					}
				}

				t_list* heap_segments_list = GET_HEAP_SEGMENTS(client_address_space);
				list_iterate(heap_segments_list, usar_segmento_si_tiene_espacio);
				sem_wait(&logger_semaphore);
				log_info(logger,"Se intento usar espacio de algun segmento (cliente %d)", cliente);
				sem_post(&logger_semaphore);

				if(!se_pudo_reservar_flag) { //Ningun segmento tenia espacio, trato de agrandar
					list_iterate(heap_segments_list, usar_segmento_si_se_puede_agrandar);
					sem_wait(&logger_semaphore);
					log_info(logger,"No habia espacio, se intento extender algun segmento (cliente %d)", cliente);
					sem_post(&logger_semaphore);
				}

				debe_crearse_segmento_flag = !se_pudo_reservar_flag;
				//borrar la heap_segments_list (porque es resultado de un filter)
			} else {
				//El proceso que pide no tiene segmento
				debe_crearse_segmento_flag = 1;
			}
			if(debe_crearse_segmento_flag) {
				sem_wait(&logger_semaphore);
				log_info(logger,"Debe crearse un nuevo segmento (cliente %d)", cliente);
				sem_post(&logger_semaphore);
				sem_wait(&mp_semaphore);
				//Crear nuevo segmento
				if(client_address_space->segment_table->elements_count) {
					sem_wait(&logger_semaphore);
					log_info(logger,"Ya habia segmentos, creo uno nuevo (cliente %d)", cliente);
					sem_post(&logger_semaphore);
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

					sem_wait(&logger_semaphore);
					log_info(logger,"Se reservaron %d frames (cliente %d)", frames_to_require, cliente);
					sem_post(&logger_semaphore);

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
						new_page->modifiedBit = 0; //Not sure
						new_page->presenceBit = 1;
						new_page->frame_number = frame_number;
						list_add(new_segment->pageFrameTable, new_page);
						if(bytes_que_quedan - page_size > 0)
							bytes_que_quedan = bytes_que_quedan - page_size;
						last_page = new_page;
						dictionary_put(clock_table, string_itoa(frame_number), new_page);
					}
					//TODO: Fixear porque si el bytes a reservar es mas grande no entras
					//Escribo la nueva metadata
					//if(bytes_a_reservar + 5 < page_size && bytes_a_reservar + 10 > page_size){ //bytes_a_reservar contempla la primer metadata???
					if(bytes_sobrantes > page_size - 5){
						sem_wait(&logger_semaphore);
						log_info(logger,"QUILOMBOOOOO METADATA SPLITEADA ndeah");
						sem_post(&logger_semaphore);

						pageFrame* final_page;
						pageFrame* previous_to_final_page;
						int final_page_metadata_bytes = bytes_a_reservar + 10 - page_size;
						int previous_to_final_page_metadata_bytes = 5 - final_page_metadata_bytes;
						uint32_t free_size = page_size - final_page_metadata_bytes;

						void* buffer = malloc(5);
						memcpy(buffer, &free_size, sizeof(uint32_t));
						memcpy(buffer+sizeof(uint32_t), &trueStatus, sizeof(bool));

						final_page = list_get(new_segment->pageFrameTable,new_segment->pageFrameTable->elements_count - 1);
						previous_to_final_page = list_get(new_segment->pageFrameTable,new_segment->pageFrameTable->elements_count - 2);

						void* ptr_to_final_page = GET_FRAME_POINTER(final_page->frame_number);
						void* ptr_to_previous_to_final_page = GET_FRAME_POINTER(previous_to_final_page->frame_number);
						ptr_to_previous_to_final_page += page_size - previous_to_final_page_metadata_bytes;

						memcpy(ptr_to_previous_to_final_page, buffer, previous_to_final_page_metadata_bytes);
						memcpy(ptr_to_final_page, buffer+previous_to_final_page_metadata_bytes, final_page_metadata_bytes);

						free(buffer);

					} else{
						bytes_sobrantes = page_size - bytes_que_quedan - 5 - 5;
						void* last_page_pointer = GET_FRAME_POINTER(last_page->frame_number);
						memcpy(last_page_pointer + bytes_que_quedan + 5, &bytes_sobrantes, sizeof(uint32_t));
						memcpy(last_page_pointer + bytes_que_quedan + 5 + sizeof(uint32_t), &trueStatus, sizeof(bool));

					}

					new_segment->size = new_segment->pageFrameTable->elements_count * page_size;
					new_segment->base = FIRST_FIT(client_address_space->segment_table, 0, new_segment->size);

					list_add(client_address_space->segment_table, new_segment);
					segment_base = new_segment->base;
				} else {
					sem_wait(&logger_semaphore);
					log_info(logger,"No habia segmentos. Creo el primero (cliente %d)", cliente);
					sem_post(&logger_semaphore);

					//Es el primer segmento
					segment* new_segment = (segment*)malloc(sizeof(segment));
					new_segment->base = 0;
					new_segment->isHeap = true;
					new_segment->pageFrameTable = list_create();
					int frames_to_require;
					if((bytes_a_reservar + 10) % page_size > 0) {
						frames_to_require = ((bytes_a_reservar + 10) / page_size) + 1;
					} else {
						frames_to_require = (bytes_a_reservar + 10) / page_size;
					}

					sem_wait(&logger_semaphore);
					log_info(logger,"Se reservaron %d frames (cliente %d)", frames_to_require, cliente);
					sem_post(&logger_semaphore);

					pageFrame* last_page;
					int bytes_que_quedan = bytes_a_reservar;
					uint32_t bytes_sobrantes = 0;

					for(int i=0; i < frames_to_require; i++) {
						int frame_number = CLOCK();
						if(i == 0) {
							//Escribo la primer metadata
							if(5 > page_size){
								log_info(logger,"La primer metadata va a quedar spliteadovich");
								int bytes_metadata_current_frame = 5 - page_size;
								int bytes_metadata_next_frame = 5 - bytes_metadata_current_frame;
								pointer = GET_FRAME_POINTER(frame_number);
								memcpy(pointer, &bytes_a_reservar, bytes_metadata_current_frame);



							} else{
								log_info(logger,"La primer metadata no va a quedar spliteada de rutaaa");
								pointer = GET_FRAME_POINTER(frame_number);
								memcpy(pointer, &bytes_a_reservar, sizeof(uint32_t));
								memcpy(pointer + sizeof(uint32_t), &falseStatus, sizeof(bool));
								pointer = pointer + 5;
							}

						}
						pageFrame* new_page = (pageFrame*)malloc(sizeof(pageFrame));
						new_page->modifiedBit = 1; //Not sure
						new_page->presenceBit = 1;
						new_page->frame_number = frame_number;
						list_add(new_segment->pageFrameTable, new_page);
						if(bytes_que_quedan - page_size > 0)
							bytes_que_quedan = bytes_que_quedan - page_size;
						last_page = new_page;
						dictionary_put(clock_table, string_itoa(frame_number), new_page);
					}

					//Escribo la nueva metadata
					if(bytes_a_reservar + 5 < page_size && bytes_a_reservar + 10 > page_size){ //bytes_a_reservar contempla la primer metadata???
						sem_wait(&logger_semaphore);
						log_info(logger,"QUILOMBOOOOO METADATA SPLITEADA ndeah");
						sem_post(&logger_semaphore);

						pageFrame* final_page;
						pageFrame* previous_to_final_page;
						int final_page_metadata_bytes = bytes_a_reservar + 10 - page_size;
						int previous_to_final_page_metadata_bytes = 5 - final_page_metadata_bytes;
						uint32_t free_size = page_size - final_page_metadata_bytes;

						void* buffer = malloc(5);
						memcpy(buffer, &free_size, sizeof(uint32_t));
						memcpy(buffer+sizeof(uint32_t), &trueStatus, sizeof(bool));

						final_page = list_get(new_segment->pageFrameTable,new_segment->pageFrameTable->elements_count - 1);
						previous_to_final_page = list_get(new_segment->pageFrameTable,new_segment->pageFrameTable->elements_count - 2);

						void* ptr_to_final_page = GET_FRAME_POINTER(final_page->frame_number);
						void* ptr_to_previous_to_final_page = GET_FRAME_POINTER(previous_to_final_page->frame_number);
						ptr_to_previous_to_final_page += page_size - previous_to_final_page_metadata_bytes;

						memcpy(ptr_to_previous_to_final_page, buffer, previous_to_final_page_metadata_bytes);
						memcpy(ptr_to_final_page, buffer+previous_to_final_page_metadata_bytes, final_page_metadata_bytes);

						free(buffer);

					} else{
						bytes_sobrantes = page_size - bytes_que_quedan - 5 - 5;
						void* last_page_pointer = GET_FRAME_POINTER(last_page->frame_number);
						memcpy(last_page_pointer + bytes_que_quedan + 5, &bytes_sobrantes, sizeof(uint32_t));
						memcpy(last_page_pointer + bytes_que_quedan + 5 + sizeof(uint32_t), &trueStatus, sizeof(bool));

					}
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
			//Ojo si hay que swapear aca. Me podrian sacar el frame antes de llegar aca?
			pageFrame* a_page = list_get(a_segment->pageFrameTable, iterator);
			if(a_page->frame_number == frame_number)
				break;
			iterator++;
			displacement_until_page = displacement_until_page + page_size;
		}
		//De aca me fui con la page. Quizas me sirve para el tema del clock
		virtual_direction = a_segment->base + displacement_until_page + page_offset;

		sem_wait(&logger_semaphore);
		log_info(logger,"Se otorgo la direccion virtual %d (cliente %d)", virtual_direction, cliente);
		sem_post(&logger_semaphore);

		sem_post(&client->client_sempahore);

*/
