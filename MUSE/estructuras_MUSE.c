/*
 * estructuras_MUSE.c
 *
 *  Created on: 25 oct. 2019
 *      Author: utnso
 */
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include <commons/config.h>
#include <commons/log.h>
#include <commons/collections/list.h>
#include <commons/collections/dictionary.h>
#include <string.h>
#include <semaphore.h>
#include "globales.h"
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>

//ESTRUCTURAS DE DATOS
// --> Estan en el .h

int CREATE_ADDRESS_SPACE(char* IP_ID) {
	addressSpace* new_address_space = (addressSpace*)malloc(sizeof(addressSpace));
	new_address_space->owner = (char*)malloc(sizeof(IP_ID));

	memcpy(new_address_space->owner, IP_ID, sizeof(IP_ID));
	new_address_space->segment_table = list_create();

	list_add(all_address_space, new_address_space);

	return 0; //En que caso podria retornar -1?
}

void SET_BITMAP(){
	FILE* archivo= fopen("bitmap.bin","w");

	int number_of_frames = memory_size / page_size;
	//Lo "paso" a bits
	if((number_of_frames % 8) == 0) {
		number_of_frames = number_of_frames / 8;
	} else {
		number_of_frames = (number_of_frames / 8) + 1;
	}

	char* frames_vector=(char*)malloc(number_of_frames);
	frames_vector=string_repeat('\0',number_of_frames);
	fwrite(frames_vector,number_of_frames,1,archivo);
	fclose(archivo);

	int file_desc_bitmap = open("bitmap.bin", O_RDWR, S_IRUSR | S_IWUSR);

	char* bit_array = mmap(NULL, number_of_frames, PROT_READ | PROT_WRITE, MAP_SHARED, file_desc_bitmap, 0);

	bitmap = bitarray_create_with_mode(bit_array,number_of_frames,LSB_FIRST);

	free(frames_vector);
}

client* FIND_CLIENT_BY_SOCKET(int a_client_socket) {
	bool es_el_cli_de_ese_socket(void *a_client) {
		return ((client*)a_client)->clientSocket == a_client_socket;
	}

	return list_find(client_list, es_el_cli_de_ese_socket); //Consigo el id segun el socket
}

int ADD_CLIENT_TO_LIST(char* client_ID, int client_socket){
	client* new_client = (client*)malloc(sizeof(client));
	new_client->clientProcessId = (char*)malloc(sizeof(client_ID));

	memcpy(new_client->clientProcessId, client_ID, sizeof(client_ID));
	memcpy(&new_client->clientSocket, &client_socket, sizeof(int));

	list_add(client_list, new_client);

	return 0; //"Si no lo pude inicializar retorna -1. Podria pasar o es solo del lado del cliente?
}

void CHECK_LOGGER(){
	if(logger!=NULL)
		log_info(logger,"Logger creado");
	else{
		printf("No se pudo crear el logger, cierro el administrador\n");
		exit(0);	//si no puedo crear el log, cierro el programa
	}
}

void CHECK_CONFIG(){
	if(config!=NULL)
		log_info(logger,"Archivo config levantado");
	else{
		log_error(logger,"No se pudo levantar el archivo de configuracion base, cierro el administrador\n");
		exit(0);	//si no encuentro el config inicial, me muero
	}
}

void CHECK_FIELDS(){
	if(config_has_property(config,"LISTEN_PORT") && config_has_property(config,"MEMORY_SIZE")
			&&config_has_property(config,"PAGE_SIZE") && config_has_property(config,"SWAP_SIZE"))
		log_info(logger,"El config tiene los campos necesarios para operar");
	else{
		log_error(logger,"El config no tiene los campos necesarios para operar");
		exit(0);
	}
}

void GET_CONFIG_VALUES(){
	listen_port = config_get_int_value(config,"LISTEN_PORT");
	memory_size = config_get_int_value(config,"MEMORY_SIZE");
	page_size = config_get_int_value(config,"PAGE_SIZE");
	swap_size = config_get_int_value(config,"SWAP_SIZE");

	memory_left = memory_size;

	log_info(logger,"Puerto de escucha: %d",listen_port);
	log_info(logger,"Tamanio de memoria (Bytes) %d",memory_size);
	log_info(logger,"Tamanio de pagina  (Bytes) %d",page_size);
	log_info(logger,"Tamanio de swap  (Bytes) %d",swap_size);
}

void INITIALIZE_SEMAPHORES(){
	sem_init(&mp_semaphore,0,1);
	sem_init(&logger_semaphore,0,1);
	sem_init(&segmentation_table_semaphore,0,1);
	sem_init(&memory_controller_semaphore,0,1);
}

void DESTROY_SEMAPHORES(){
	sem_destroy(&mp_semaphore);
	sem_destroy(&logger_semaphore);
	sem_destroy(&segmentation_table_semaphore);
	sem_destroy(&memory_controller_semaphore);
}

void CHECK_MEMORY(){
	if(mp_pointer!=NULL)
		log_info(logger,"Memoria creada");
	else{
		log_error(logger,"No se pudo crear la memoria principal para administrar, salgo a SO");
		exit(0);
	}
}

void CREATE_TABLES(){
	segmentation_table = list_create();
}

void DESTROY_TABLES(){
	/*
	DESTROY_ELEMENTS(pageFrame* frame){
		frame->
	}

	list_destroy_and_destroy_elements(segmentation_table,);*/
}

void SUBSTRACT_MEMORY_LEFT(int size){
	for(int i=0; i<size; i++){
		memory_left--;
	}
}

//usemos 1 como usado y 0 como libre en el bool (LIBRE 1 - USADO 0)
void WRITE_HEAPMETADATA_IN_MEMORY(void* pointer, uint32_t size, bool status){
	if(memory_left>=5){
		sem_wait(&mp_semaphore);
		int aux = size-5;
		memcpy(pointer,&aux,sizeof(uint32_t));
		memcpy(pointer+sizeof(uint32_t),&status,sizeof(bool));
		SUBSTRACT_MEMORY_LEFT(5);
		sem_post(&mp_semaphore);
	}else{
		//aca hay que ver que hacemos si no hay espacio en memoria
		//para escribir un metadata
	}
}

heapMetadata* READ_HEAPMETADATA_IN_MEMORY(void* pointer){
	heapMetadata* new_metadata = (heapMetadata*)malloc(sizeof(heapMetadata));

	sem_wait(&mp_semaphore);
	memcpy(&new_metadata->size, pointer,sizeof(uint32_t));
	memcpy(&new_metadata->isFree, pointer+sizeof(uint32_t),sizeof(bool));
	sem_post(&mp_semaphore);

	return new_metadata;
}

/* POTENTIALLY DEPRECATED FUNCTIONS

void* SEGMENT_IS_BIG_ENOUGH(segment* a_segment, uint32_t intended_size) {

	int segment_size = a_segment->pageFrameTable->elements_count * page_size;
	int move_counter = 0;
	void* pointer = a_segment->segmentPointer;

	while(move_counter < segment_size) { //mientras este en mi segmento
		heapMetadata* new_metadata = READ_HEAPMETADATA_IN_MEMORY(pointer - 5);
		if(new_metadata->isFree)
			if(new_metadata->size >= intended_size)
				return pointer;
		pointer = pointer + new_metadata->size + 5;
		move_counter+=new_metadata->size;
		free(new_metadata);
	}

	return NULL;
}

t_list* GET_CLIENT_SEGMENTS(int a_client_socket) {
	client* the_client = FIND_CLIENT_BY_SOCKET(a_client_socket); //Consigo el id segun el socket

	bool segmento_es_del_cliente(void *a_segment) {
		return strcmp(((segment*)a_segment)->owner, the_client->clientProcessId) == 0;
	}
	return list_filter(segmentation_table, segmento_es_del_cliente);
}

bool CLIENT_HAS_SEGMENT(int a_client_socket) {
	client* the_client = FIND_CLIENT_BY_SOCKET(a_client_socket); //Consigo el id segun el socket

	bool segmento_es_del_cliente(void *a_segment) {
		return strcmp(((segment*)a_segment)->owner, the_client->clientProcessId) == 0;
	}

	return list_any_satisfy(segmentation_table, segmento_es_del_cliente);
}

void CREATE_NEW_SEGMENT_IN_MEMORY(void* pointer, void* info, uint32_t size,char * name){
	if(memory_left==memory_size && size+10<=memory_left){	//1° segmento en memoria
		WRITE_HEAPMETADATA_IN_MEMORY(pointer,size,1);	//1°metadata
		memcpy(pointer+5,info,size);
		WRITE_ADDRESSES_IN_SEGMENT(pointer+5,size,CREATE_NEW_EMPTY_SEGMENT(name));
		WRITE_HEAPMETADATA_IN_MEMORY(pointer+5+size,memory_left,0);//el puntero inicial+5+size del bloque
	}else if(size+10<=memory_left){	//es decir, hay otro segmento antes
		//podriamos modificar los valores de la metadata anterior o reescribirla
		//me decanto por volver a escribir encima
		//primero tendria que encontrar la direccion de memoria donde tengo el ultimo metadata
		//lo saco haciendo memoria total - memoria restante-5 porque quiero pegarle
		//tambien a la ultima metadata escrita
		WRITE_HEAPMETADATA_IN_MEMORY(pointer+(memory_size-memory_left-5),size,1);
		memcpy(pointer+(memory_size-memory_left),info,size);
		WRITE_ADDRESSES_IN_SEGMENT(pointer+(memory_size-memory_left),size,CREATE_NEW_EMPTY_SEGMENT(name));
		WRITE_HEAPMETADATA_IN_MEMORY(pointer+(memory_size-memory_left)+size,memory_left,0);
	}else{
		//Que hacer si no queda espacio?
	}
}

void WRITE_ADDRESSES_IN_SEGMENT(void* pointer, uint32_t size, segment* segment){
	for(int i=0; i<size/page_size; i++){	//tamanio total/tamanio de pagina = cantidad de paginas y direcciones
		pageFrame* newPageFrame = (pageFrame*)malloc(sizeof(pageFrame));
		newPageFrame->modifiedBit=0;
		newPageFrame->presenceBit=0;	//OJO CON ESTO, REVISA LA TEORIA
		newPageFrame->pagePointer=pointer+(i*page_size);//1=0*32,2=1*32,3=2*32 OJO
		list_add(segment->pageFrameTable,newPageFrame);
	}
}

segment* CREATE_NEW_EMPTY_SEGMENT(char* name){
	segment* newSegment = (segment*)malloc(sizeof(segment));
	newSegment->owner=(char*)malloc(strlen(name)+1);	//strlen no incluye el \0
	memcpy(newSegment->owner,name,strlen(name)+1);
	newSegment->pageFrameTable=list_create();
	list_add(segmentation_table,newSegment);

	return newSegment;
}

}
 */
