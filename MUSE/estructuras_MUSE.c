/*
 * estructuras_MUSE.c
 *
 *  Created on: 25 oct. 2019
 *      Author: utnso
 */
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <inttypes.h>
#include <commons/config.h>
#include <commons/log.h>
#include <commons/collections/list.h>
#include <commons/collections/dictionary.h>
#include <string.h>
#include <semaphore.h>
#include <pthread.h>
#include "globales.h"
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>
#include "estructuras_MUSE.h"

//ESTRUCTURAS DE DATOS
// --> Estan en el .h

void DESTROY_CLIENT(client* a_client) {
	free(a_client->clientProcessId);
	free(a_client);
}

void DESTROY_ADDRESS_SPACE(addressSpace* an_address_space) {
	free(an_address_space->owner);
	list_destroy_and_destroy_elements(an_address_space->segment_table, DESTROY_SEGMENT);
	free(an_address_space);
}

void DESTROY_PAGE(pageFrame* a_page) {
	free(a_page);
}

void CLIENT_LEFT_THE_SYSTEM(int client_socket) {
	/* NOTA
		El clock_table no hace falta tocarlo. Ej: se va un proceso, libera el frame 1 nadie va a pasar por
		clock_table[1] porque antes se le va a asignar el frame 1 segun el bitmap_memory y,cuando esto
		ocurra, ese proceso va a hacer clock_table[1]=su_nueva_pagina. Por este motivo, en ningun
		momento quedan posiciones del clock_table "corruptas".
	 */

	//Hay que:
	//	1)Liberar los frames ocupados por el proceso (bitmap_memory o bitmap_swap)
	//	2)Limpiar las estructuras administrativas asociadas al proceso
	//		a)Limpio la segment_table
	//		b)free del owner
	//		c)Lo saco de all_address_spaces
	//		d)Lo saco de la lista de clientes

	addressSpace* client_ad_sp = GET_ADDRESS_SPACE(client_socket);
	void liberar_frames(segment* a_segment) {
		void iterar_paginas(pageFrame* a_page) {
			if(a_page->presenceBit) {
				FREE_MEMORY_FRAME_BITMAP(a_page->frame_number);
			} else if (a_segment->isHeap) {
				FREE_SWAP_FRAME_BITMAP(a_page->frame_number);
			} else {
				//(COPIADO DEL UNMAP)
				//Controlar que la direccion que me pasan este OK
				mappedFile* mapped_file = GET_MAPPED_FILE(a_segment->path);
				mapped_file->references--;
				if(!mapped_file->references) {
					close(mapped_file->file_desc);
					munmap(mapped_file->pointer, mapped_file->length);
					//Borrar entrada de la lista mapped_files
					int index = GET_MAPPED_FILE_INDEX(mapped_file->path);
					list_remove_and_destroy_element(mapped_files, index, DESTROY_MAPPED_FILE);
				}
			}
		}
		list_iterate(a_segment->pageFrameTable, iterar_paginas);
	}

	client* a_client = FIND_CLIENT_BY_SOCKET(client_socket);

	bool el_address_spaces_es_del_cliente(void *an_address_space) {
		return strcmp(((addressSpace*)an_address_space)->owner, a_client->clientProcessId) == 0;
	}

	bool es_el_cliente(void* certain_client) {
		return strcmp(((client*)certain_client)->clientProcessId, a_client->clientProcessId) == 0;
	}

	//Libero los frames ocupados tanto en bitmap_memory como en bitmap_swap
	list_iterate(client_ad_sp->segment_table, liberar_frames);
	list_remove_and_destroy_by_condition(all_address_spaces, el_address_spaces_es_del_cliente, DESTROY_ADDRESS_SPACE);
	list_remove_and_destroy_by_condition(client_list, es_el_cliente, DESTROY_CLIENT);
}

void FREE_SWAP_FRAME_BITMAP(int frame_number) {
	int limit = bitarray_get_max_bit(bitmap_swap);
	if(frame_number < limit) { //Menor estricto o amplio?
		bitarray_clean_bit(bitmap_swap, frame_number);
	} else {
		//Estas queriendo liberar un numero de frame que no existe
	}
}

void SWAP_INIT() {
	int fd_swap = open("SWAP_FILE", O_RDWR | O_CREAT, S_IRUSR | S_IRGRP | S_IROTH); //https://stackoverflow.com/questions/2395465/create-a-file-in-linux-using-c
	swap_file = (char*) malloc(swap_size);
	//swap_file = mmap(NULL, swap_size, PROT_READ, PROT_WRITE, MAP_SHARED, fd_swap, 0); "too many arguments"
	swap_file = mmap(NULL, swap_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd_swap, 0);
}

int GET_FREE_SWAP_FRAME() {
	int counter = bitarray_get_max_bit(bitmap_swap);
	for(int i=0;i<counter;i++){
		if(!bitarray_test_bit(bitmap_swap, i)){
			bitarray_set_bit(bitmap_swap,i);
			return i;
		}
	}
	return -1; //Si no hay mas lugar en swap muere un gatito
}

void SET_BITMAP_SWAP(){
	int number_of_frames = swap_size / page_size;
	//Lo "paso" a bits
	if((number_of_frames % 8) == 0) {
		number_of_frames = number_of_frames / 8;
	} else {
		number_of_frames = (number_of_frames / 8) + 1;
	}

	char* frames_vector=(char*)malloc(number_of_frames);
	frames_vector=string_repeat('\0',number_of_frames);

	bitmap_swap = bitarray_create_with_mode(frames_vector, number_of_frames, LSB_FIRST);

	free(frames_vector); //Esto se libera o tiene que vivir porque esta el bitmap?
}

int CLOCK() {
	int counter = bitarray_get_max_bit(bitmap_memory);
	for(int i=0;i<counter;i++){
		if(!bitarray_test_bit(bitmap_memory, i)){
			bitarray_set_bit(bitmap_memory,i);
			SUBSTRACT_MEMORY_LEFT(page_size);
			return i;
		}
	}
	//No consegui frames libres, tengo que ejecutar el algoritmo
	int initial_position = clock_pointer;
	int frame_found=-1;
	int iterations=0;
	pageFrame* page_to_replace;
	//Busco uso=0, modificado=0
	while(frame_found < 0 && iterations < counter) {
		pageFrame* page_frame = clock_table[initial_position];
		if(page_frame->useBit == 0 && page_frame->modifiedBit == 0) {
			frame_found = initial_position;
			clock_pointer = frame_found + 1;
			page_to_replace = page_frame;
			if(clock_pointer > counter)
				clock_pointer = 0;
		}
		initial_position++;
		if(initial_position > counter)
			initial_position = 0;
	}
	//Busco uso=0, modificado=1 (pongo uso en 0)
	initial_position = clock_pointer;
	while(frame_found < 0 && iterations < counter) {
		pageFrame* page_frame = clock_table[initial_position];
		if(page_frame->useBit == 0 && page_frame->modifiedBit == 1) {
			frame_found = initial_position;
			clock_pointer = frame_found + 1;
			page_to_replace = page_frame;
			if(clock_pointer > counter)
				clock_pointer = 0;
		}
		page_frame->useBit = 0;
		initial_position++;
		if(initial_position > counter)
			initial_position = 0;
	}
	//Repito 0,0
	initial_position = clock_pointer;
	while(frame_found < 0 && iterations < counter) {
		pageFrame* page_frame = clock_table[initial_position];
		if(page_frame->useBit == 0 && page_frame->modifiedBit == 0) {
			frame_found = initial_position;
			clock_pointer = frame_found + 1;
			page_to_replace = page_frame;
			if(clock_pointer > counter)
				clock_pointer = 0;
		}
		initial_position++;
		if(initial_position > counter)
			initial_position = 0;
	}
	//Repito 0,1
	initial_position = clock_pointer;
	while(frame_found < 0 && iterations < counter) {
		pageFrame* page_frame = clock_table[initial_position];
		if(page_frame->useBit == 0 && page_frame->modifiedBit == 1) {
			frame_found = initial_position;
			clock_pointer = frame_found + 1;
			page_to_replace = page_frame;
			if(clock_pointer > counter)
				clock_pointer = 0;
		}
		initial_position++;
		if(initial_position > counter)
			initial_position = 0;
	}

	//Escribo en swap las cosas del frame que estoy entregando
	void* frame_pointer = GET_FRAME_POINTER(page_to_replace->frame_number);
	int swap_position = GET_FREE_SWAP_FRAME();
	//Juega a algo el bit de modificado a la hora de escribir?
	for(int i = 0; i < page_size; i++) { //Escribo en swap, byte a byte
		swap_file[swap_position + i] = (frame_pointer + i);
	}
	//Seteo el page frame con presence=0 y la direccion de swap donde esta la info
	page_to_replace->presenceBit=0;
	page_to_replace->frame_number=swap_position;
	return frame_found;

}

void DESTROY_SEGMENT(segment* a_segment) {
	if(!a_segment->isHeap)
		free(a_segment->path);
	list_destroy_and_destroy_elements(a_segment->pageFrameTable, DESTROY_PAGE);
	free(a_segment);
}

int GET_SEGMENT_INDEX(t_list* segment_table, uint32_t a_base) {
	int index;
	int iterator=0;

	void es_el_segmento(void *a_segment) {
		if(((segment*)a_segment)->base == a_base)
			index = iterator;

		iterator++;
	}

	list_iterate(segment_table, es_el_segmento);
	return index;
}

void DESTROY_MAPPED_FILE(mappedFile* mapped_file) {
	free(mapped_file->path);
	free(mapped_file->owner); //Quien lo tiene abierto
	//free al pointer? mepa que no
	free(mapped_file);
}

int GET_MAPPED_FILE_INDEX(char* path) {
	int index;
	int iterator=0;

	void es_el_archivo(void *a_mapped_file) {
		if(strcmp(((mappedFile*)a_mapped_file)->path, path) == 0)
			index = iterator;

		iterator++;
	}

	list_iterate(mapped_files, es_el_archivo);
	return index;
}

mappedFile* GET_MAPPED_FILE(char* path) {
	bool es_el_archivo(void *a_mapped_file) {
		return strcmp(((mappedFile*)a_mapped_file)->path, path) == 0;
	}

	return list_find(mapped_files, es_el_archivo);
}

bool FILE_ALREADY_MAPPED(char* path) {
	bool es_el_archivo(void *a_mapped_file) {
		return strcmp(((mappedFile*)a_mapped_file)->path, path) == 0;
	}

	return list_count_satisfying(mapped_files, es_el_archivo);
}

uint32_t FIRST_FIT(t_list* segment_table, uint32_t base, uint32_t size) { //Te da la primer dir virtual donde entras
	int iterator = 0;
	uint32_t final_base = base;
	while(iterator < segment_table->elements_count) {
		uint32_t intended_direction = (final_base + size);
		segment* iterative_segment = list_get(segment_table, iterator);
		bool cond_1 = final_base == iterative_segment->base;
		bool cond_2 = (final_base < iterative_segment->base) && intended_direction > iterative_segment->base;
		bool cond_3 = intended_direction > iterative_segment->base && intended_direction < iterative_segment->base + iterative_segment->size;
		final_base = iterative_segment->base + iterative_segment->size + 1;
		iterator++;
		if(!cond_1 && !cond_2 && !cond_3)
			break;
	}

	return final_base;
}

segment* GET_SEGMENT_FROM_BASE(uint32_t base, addressSpace* address_space) {
	int iterator = 0;
	while(address_space->segment_table->elements_count > iterator) {
		segment* a_segment = list_get(address_space->segment_table, iterator);
		if(a_segment->base == base)
			return a_segment;
		iterator++;
	}
	return NULL;
}

int GET_OFFSET_FROM_POINTER(void* pointer) {
	return (pointer - mp_pointer) % page_size;
}

int GET_FRAME_NUMBER_FROM_POINTER(void* pointer) {
	return (pointer - mp_pointer) / page_size;
}

void* GET_LAST_METADATA(segment* a_segment) {
	int segment_size = a_segment->size;
	int segment_move_counter = 0;
	int page_move_counter = 0;
	int page_number = 0;
	t_list* page_frame_table = a_segment->pageFrameTable;
	void* pointer;

	while(segment_move_counter < segment_size && page_number < page_frame_table->elements_count) { //Mientras este en mi segmento
		int current_frame = list_get(page_frame_table, page_number);
		pointer = GET_FRAME_POINTER(current_frame);
		while(page_move_counter < page_size) {
			heapMetadata* new_metadata = READ_HEAPMETADATA_IN_MEMORY(pointer);
			if(new_metadata->isFree && page_number == (page_frame_table->elements_count - 1)) {
				//Si estoy en la ultima pagina y la metadata esta free
				free(new_metadata);
				return pointer;
			}
			page_move_counter = page_move_counter + new_metadata->size + 5;
			pointer = pointer + page_move_counter;
			segment_move_counter = segment_move_counter + page_move_counter;
			free(new_metadata);
		}
		page_number++;
	}

	return NULL; //No deberia caer nunca aca, pero lo pongo asi Eclipse no jode
}

bool SEGMENT_CAN_BE_EXTENDED(segment* a_segment, addressSpace* an_address_space, uint32_t intended_size) {
	uint32_t real_size_needed;
	t_list* segment_table = an_address_space->segment_table;
	void* last_metadata = GET_LAST_METADATA(a_segment);
	uint32_t internal_fragmentation;

	memcpy(&internal_fragmentation, last_metadata, sizeof(uint32_t));

	real_size_needed = intended_size - internal_fragmentation;

	uint32_t intended_direction = a_segment->base + a_segment->size + real_size_needed;

	if(intended_direction % page_size > 0)
		intended_direction = ((intended_direction / page_size) + 1) * page_size;

	bool la_direccion_pertenece_a_otro_segmento(void *one_segment) {
		segment* iterative_segment = (segment*)one_segment;
		bool cond_1 = (a_segment->base < iterative_segment->base) && intended_direction > iterative_segment->base;
		bool cond_2 = intended_direction > iterative_segment->base && intended_direction < iterative_segment->base + iterative_segment->size;
		return cond_1 || cond_2;
	}

	return list_count_satisfying(segment_table, la_direccion_pertenece_a_otro_segmento);
}

void* SEGMENT_IS_BIG_ENOUGH(segment* a_segment, uint32_t intended_size) {

	int segment_size = a_segment->size;
	int segment_move_counter = 0;
	int page_move_counter = 0;
	int page_number = 0;
	t_list* page_frame_table = a_segment->pageFrameTable;
	void* pointer;

	if(a_segment->size - 10 < intended_size) //Si el tamanio  (-2 metadatas) no me alcanza ni miro el resto
		return NULL;

	while(segment_move_counter < segment_size && page_number < page_frame_table->elements_count) { //Mientras este en mi segmento
		int current_frame = list_get(page_frame_table, page_number);
		pointer = GET_FRAME_POINTER(current_frame);
		while(page_move_counter < page_size) {
			heapMetadata* new_metadata = READ_HEAPMETADATA_IN_MEMORY(pointer);
			if(new_metadata->isFree)
				if(new_metadata->size >= intended_size) {
					pointer = pointer + 5;
					free(new_metadata);
					return pointer;
				}
			page_move_counter = page_move_counter + new_metadata->size + 5;
			pointer = pointer + page_move_counter;
			segment_move_counter = segment_move_counter + page_move_counter;
			free(new_metadata);
		}
		page_number++;
	}

	return NULL;
}

t_list* GET_HEAP_SEGMENTS(addressSpace* address_space) {
	bool es_segmento_heap(void *a_segment) {
		return ((segment*)a_segment)->isHeap;
	}

	return list_filter(address_space->segment_table, es_segmento_heap);
}

bool THERE_ARE_EXISTING_HEAP_SEGMENTS(addressSpace* an_address_space) {
	bool es_segmento_heap(void *a_segment) {
		return ((segment*)a_segment)->isHeap;
	}

	return list_count_satisfying(an_address_space->segment_table, es_segmento_heap);
}

addressSpace* GET_ADDRESS_SPACE(int client_socket) {
	client* a_client = FIND_CLIENT_BY_SOCKET(client_socket);

	bool el_address_spaces_es_del_cliente(void *an_address_space) {
		return strcmp(((addressSpace*)an_address_space)->owner, a_client->clientProcessId) == 0;
	}

	return list_find(all_address_spaces, el_address_spaces_es_del_cliente);
}

void FREE_MEMORY_FRAME_BITMAP(int frame_number) {
	int limit = bitarray_get_max_bit(bitmap_memory);
	if(frame_number < limit) { //Menor estricto o amplio?
		bitarray_clean_bit(bitmap_memory, frame_number);
	} else {
		//Estas queriendo liberar un numero de frame que no existe
	}
}

void* GET_FRAME_POINTER(int frame_number) {
	return mp_pointer + frame_number * page_size;
}

int CREATE_ADDRESS_SPACE(char* IP_ID) {
	addressSpace* new_address_space = (addressSpace*)malloc(sizeof(addressSpace));
	new_address_space->owner = (char*)malloc(sizeof(IP_ID));

	memcpy(new_address_space->owner, IP_ID, sizeof(IP_ID));
	new_address_space->segment_table = list_create();

	list_add(all_address_spaces, new_address_space);

	return 0; //En que caso podria retornar -1?
}

void SET_BITMAP_MEMORY(){
	int number_of_frames = memory_size / page_size;
	//Lo "paso" a bits
	if((number_of_frames % 8) == 0) {
		number_of_frames = number_of_frames / 8;
	} else {
		number_of_frames = (number_of_frames / 8) + 1;
	}

	char* frames_vector=(char*)malloc(number_of_frames);
	frames_vector=string_repeat('\0',number_of_frames);

	bitmap_memory = bitarray_create_with_mode(frames_vector, number_of_frames, LSB_FIRST);

	free(frames_vector); //Esto se libera o tiene que vivir porque esta el bitmap?
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


segment* GET_SEGMENT_FROM_ADDRESS(uint32_t address, addressSpace* address_space){
	int iterator = 0;
	while(address_space->segment_table->elements_count > iterator) {
		segment* a_segment = list_get(address_space->segment_table, iterator);
		if(address >= a_segment->base && address <= a_segment->size)
			return a_segment;
		iterator++;
	}
	return NULL;
}


int GET_FRAME_FROM_ADDRESS(uint32_t address, segment* a_segment){
	int page_number = 0;
	t_list* page_frame_table = a_segment->pageFrameTable;
	uint32_t page_size_plus_base = a_segment->base;

	while(page_frame_table->elements_count > page_number && page_size_plus_base < address){
		page_size_plus_base = page_size + a_segment->base; // Estoy sumando un int y un uint32 ¿Se puede?
		if(page_size_plus_base > address){
			pageFrame* page = list_get(page_frame_table, page_number);
			return page->frame_number;
		}
		page_number++;
	}
	return NULL;
}

int GET_PAGE_NUMBER_FROM_ADDRESS(uint32_t address, segment* a_segment){
	int page_number = 0;
	t_list* page_frame_table = a_segment->pageFrameTable;
	uint32_t page_size_plus_base = a_segment->base;

	while(page_frame_table->elements_count > page_number && page_size_plus_base < address){
		page_size_plus_base = page_size + a_segment->base;
		if(page_size_plus_base > address){
			return page_number;
		}
		page_number++;
	}
	return NULL;
}
//TODO: Hacer frees de current metadata y next metadata

// Cada vez que hago un Free() invoco esta funcion que recorre toda la TDP mergeando las metadatas libres y borra las paginas que estan
// todas free

void MERGE_CONSECUTIVES_FREE_BLOCKS(segment* a_segment){
	int page_move_counter = 0;
	int new_page_move_counter = 0;
	int page_number = 0;
	heapMetadata* current_metadata;
	heapMetadata* next_metadata;
	t_list* page_frame_table = a_segment->pageFrameTable;
	pageFrame* first_page = list_get(page_frame_table, page_number);
	int current_frame = first_page->frame_number;
	void* ptr_to_current_frame = GET_FRAME_POINTER(current_frame);
	void* ptr_to_current_metadata = ptr_to_current_frame;

	while(page_frame_table->elements_count > page_number){

		current_metadata = READ_HEAPMETADATA_IN_MEMORY(ptr_to_current_metadata);

		if(current_metadata->isFree){ // si estoy libre
			page_move_counter = page_move_counter + current_metadata->size + 5; // el +5 es para avanzar los 5b de la primera metadata

			if(page_move_counter < page_size){ // si sigo en mi pagina after moverme
				void* ptr_to_next_metadata = ptr_to_current_metadata + current_metadata->size + 5;
				next_metadata = READ_HEAPMETADATA_IN_MEMORY(ptr_to_next_metadata);

				if(next_metadata->isFree){ // si mi proxima metadata esta libre
					uint32_t total_size = current_metadata->size + next_metadata->size + 5; // el +5 de la otra metadata
					page_move_counter = page_move_counter - current_metadata->size - 5; // anulo el desplazamiento, la proxima lo hace con el nuevo size
					WRITE_HEAPMETADATA_IN_MEMORY(ptr_to_current_metadata, total_size, 1);
				} else{ // si el proximo no esta free... me paro en ese y se vuelve mi actual
					ptr_to_current_metadata = ptr_to_next_metadata;
				}
			} else{ // si no sigo en mi pagina after moverme... busco nuevo frame y puntero a la metadata del frame
				new_page_move_counter = page_move_counter - page_size; //lo que me sobro en una pagina va a estar en la otra
				page_number++;
				if(page_number < page_frame_table->elements_count){
					pageFrame* next_page = list_get(page_frame_table, page_number);
					current_frame = next_page->frame_number;
					void* ptr_to_new_frame = GET_FRAME_POINTER(current_frame);
					void* ptr_to_first_metadata_new_frame = ptr_to_new_frame + new_page_move_counter;
					next_metadata = READ_HEAPMETADATA_IN_MEMORY(ptr_to_first_metadata_new_frame);


					if(next_metadata->isFree){ // si la proxima metadata que esta en otro frame esta libre
						uint32_t total_size = current_metadata->size + next_metadata->size + 5;
						page_move_counter = page_move_counter - current_metadata->size - 5;
						WRITE_HEAPMETADATA_IN_MEMORY(ptr_to_current_metadata, total_size, 1);
						page_number--;
					} else{ // si no esta libre
						ptr_to_current_metadata = ptr_to_first_metadata_new_frame;
						page_move_counter = new_page_move_counter;

					}
				}
			}
		} else{ // si no esta free... me paro en el proximo y se vuelve mi actual
			page_move_counter = page_move_counter + current_metadata->size + 5;

			if(page_move_counter < page_size){ // si sigo en mi pagina after moverme
				void* ptr_to_next_metadata = ptr_to_current_metadata + current_metadata->size + 5;
				ptr_to_current_metadata = ptr_to_next_metadata;
			} else{ // si no sigo en mi pagina after moverme... busco nuevo frame y puntero a la metadata del frame
				new_page_move_counter = page_move_counter - page_size;
				page_number++;
				if(page_number < page_frame_table->elements_count) {
					pageFrame* next_page = list_get(page_frame_table, page_number);
					current_frame = next_page->frame_number;
					void* ptr_to_new_frame = GET_FRAME_POINTER(current_frame);
					void* ptr_to_first_metadata_new_frame = ptr_to_new_frame + new_page_move_counter;
					ptr_to_current_metadata = ptr_to_first_metadata_new_frame;
					page_move_counter = new_page_move_counter;
				}
			}

		}
	}

	REMOVE_FREE_PAGES_FROM_SEGMENT(a_segment);
}



int FREE_USED_FRAME(uint32_t address, addressSpace* address_space) {
	segment* a_segment = GET_SEGMENT_FROM_ADDRESS(address, address_space);
	if(a_segment != NULL && a_segment->isHeap){
		int frame = GET_FRAME_FROM_ADDRESS(address, a_segment);
		if(frame != NULL){
			void* ptr_to_frame = GET_FRAME_POINTER(frame);
			void* ptr_to_metadata = ptr_to_frame + address;
			heapMetadata* frame_metadata = READ_HEAPMETADATA_IN_MEMORY(ptr_to_metadata); //donde hago el free
			WRITE_HEAPMETADATA_IN_MEMORY(ptr_to_metadata, frame_metadata->size, 1);
			MERGE_CONSECUTIVES_FREE_BLOCKS(a_segment);
			return frame_metadata->size; // Asi puedo saber cuanto libera y registrarlo en las metricas
		} else log_error(logger,"La pagina no se encuentra en memoria"); // generar page fault y swappear
	} else log_error(logger,"El segmento no se encuentra en memoria");
	return NULL;
}


//TODO: La parte del swap
//TODO: Si nos dicen que no nos importa que traiga la metadata siguiente queda asi, sino agregar "get_metadata_behind_address"
void* GET_N_BYTES_DATA_FROM_MUSE(addressSpace* address_space, uint32_t src, size_t bytes_a_copiar) {
	segment* a_segment = GET_SEGMENT_FROM_ADDRESS(src, address_space);
	t_list* page_frame_table = a_segment->pageFrameTable;
	int page_move_counter = 0;
	int page_number = GET_PAGE_NUMBER_FROM_ADDRESS(src, a_segment);
	int offset = 0;
	int bytes = bytes_a_copiar;
	void* data = malloc(bytes_a_copiar);

	if(a_segment != NULL){
		src = TRANSLATE_DL_TO_DF(src);
		pageFrame* current_page = list_get(page_frame_table, page_number);
		void* ptr_to_frame = GET_FRAME_POINTER(current_page->frame_number);
		void* ptr_to_data = ptr_to_frame + src; // va a donde empieza la data a leer

		while(page_frame_table->elements_count > page_number && bytes > 0){
			if(!current_page->presenceBit){
				//swap
				int swap_frame =  current_page->frame_number;
				int swap_pos = swap_frame * page_size;
				int free_frame = CLOCK();
				void* ptr_to_free_frame = GET_FRAME_POINTER(free_frame);

				for(int i = 0; i < page_size; i++){
					memcpy(ptr_to_free_frame + i, swap_file[swap_pos + i], 1);
				}

				FREE_SWAP_FRAME_BITMAP(swap_frame);

				current_page->frame_number = free_frame;
				current_page->presenceBit = 1;
			}

			void* ptr_to_frame = GET_FRAME_POINTER(current_page->frame_number);
			void* ptr_to_data = ptr_to_frame + src; // va a donde empieza la data a leer
			page_move_counter = ptr_to_data + bytes_a_copiar;

			if(page_move_counter < page_size){ // si al sumar esos bytes sigo en mi pagina
				memcpy(data+offset, ptr_to_data, bytes_a_copiar);
				bytes = bytes - bytes_a_copiar;
				current_page->useBit = 1;

			} else{ // si al sumar esos bytes me pase... voy a la pagina siguiente y leo lo que me queda
				int bytes_restantes_en_frame_siguiente = page_move_counter - page_size;
				int bytes_en_frame_anterior = bytes_a_copiar - bytes_restantes_en_frame_siguiente;
				memcpy(data, ptr_to_data, bytes_en_frame_anterior);
				bytes = bytes - bytes_en_frame_anterior;
				page_number++;
				if(page_number < page_frame_table->elements_count){
					current_page->useBit = 1;
					current_page = list_get(page_frame_table, page_number);
					bytes_a_copiar = bytes_restantes_en_frame_siguiente;
					offset = offset + bytes_en_frame_anterior;
				}
			}
		}
	}
	return data;
}

int TRANSLATE_DL_TO_DF(uint32_t dl){
	int r = dl / page_size;
	int df = dl - (page_size * r);

	return df;
}

// no estoy seguro de como me llega la data por parametro lo dejo como void* data
void WRITE_N_BYTES_DATA_TO_MUSE(uint32_t dst, addressSpace* address_space, size_t bytes_a_copiar, void* data){
	segment* a_segment = GET_SEGMENT_FROM_ADDRESS(dst, address_space);
	t_list* page_frame_table = a_segment->pageFrameTable;
	int page_move_counter = 0;
	int page_number = GET_PAGE_NUMBER_FROM_ADDRESS(dst, a_segment);
	int offset = 0;
	int bytes = bytes_a_copiar;

	if(a_segment != NULL){
		dst = TRANSLATE_DL_TO_DF(dst);
		pageFrame* current_page = list_get(page_frame_table, page_number);

		while(page_frame_table->elements_count > page_number && bytes > 0){
			if(!current_page->presenceBit){
				//swap
				int swap_frame =  current_page->frame_number;
				int swap_pos = swap_frame * page_size;
				int free_frame = CLOCK();
				void* ptr_to_free_frame = GET_FRAME_POINTER(free_frame);

				for(int i = 0; i < page_size; i++){
					memcpy(ptr_to_free_frame + i, swap_file[swap_pos + i], 1);
				}

				FREE_SWAP_FRAME_BITMAP(swap_frame);

				current_page->frame_number = free_frame;
				current_page->presenceBit = 1;
			}

			heapMetadata* metadata = GET_METADATA_BEHIND_ADDRESS(dst, page_frame_table);
			if(metadata->size > bytes_a_copiar + dst){ //Si tengo espacio suficiente para copiar, copio
				void* ptr_to_frame = GET_FRAME_POINTER(current_page->frame_number);
				void* ptr_to_data = ptr_to_frame + dst; // va a donde empieza la data a escribir

				page_move_counter = ptr_to_data + bytes_a_copiar;

				if(page_move_counter < page_size){ // si al sumar esos bytes sigo en mi pagina
					memcpy(ptr_to_data, data+offset, bytes_a_copiar);
					bytes = bytes - bytes_a_copiar;
					current_page->useBit = 1;

				} else{ // si al sumar esos bytes me pase... voy a la pagina siguiente y copio lo que me queda
					int bytes_restantes_en_frame_siguiente = page_move_counter - page_size;
					int bytes_en_frame_anterior = bytes_a_copiar - bytes_restantes_en_frame_siguiente;
					memcpy(ptr_to_data, data+offset, bytes_en_frame_anterior);
					bytes = bytes - bytes_en_frame_anterior;
					page_number++;
					if(page_number < page_frame_table->elements_count){
						current_page->useBit = 1;
						current_page = list_get(page_frame_table, page_number);
						bytes_a_copiar = bytes_restantes_en_frame_siguiente;
						offset = offset + bytes_en_frame_anterior;
					} //ultima pagina, no se puede seguir
				}
			} // else no tengo espacio suficiente
		}
	}
}

heapMetadata* GET_METADATA_BEHIND_ADDRESS(uint32_t address, t_list* page_frame_table){
	heapMetadata* current_metadata;
	int page_number = 0;
	pageFrame* current_page = list_get(page_frame_table, 0);
	int new_page_move_counter = 0;
	int page_move_counter = 0;
	int global_move_counter = 0;
	int offset = 0;
	void* ptr_to_LA_metadata;
	void* ptr_to_current_metadata = GET_FRAME_POINTER(current_page->frame_number);

	while(global_move_counter < address){
		current_metadata = READ_HEAPMETADATA_IN_MEMORY(ptr_to_current_metadata);
		page_move_counter = new_page_move_counter;

		while(page_move_counter < page_size && global_move_counter < address){ // mientras sigo en mi pagina
			page_move_counter = page_move_counter + current_metadata->size + 5;
			global_move_counter = global_move_counter + page_move_counter;
			if(page_move_counter < page_size){ // si sigo en mi pagina after moverme
				void* ptr_to_next_metadata = ptr_to_current_metadata + current_metadata->size + 5;
				ptr_to_LA_metadata = ptr_to_current_metadata;
				free(current_metadata);
				current_metadata = READ_HEAPMETADATA_IN_MEMORY(ptr_to_next_metadata);
			} else{ // cambio de pagina
				page_number++;
				new_page_move_counter = page_move_counter - page_size;
				global_move_counter = global_move_counter + page_move_counter;
				pageFrame* next_page = list_get(page_frame_table, page_number);
				ptr_to_LA_metadata = ptr_to_current_metadata;
				free(current_metadata);
				if(next_page != NULL){
					void* ptr_to_new_frame = GET_FRAME_POINTER(next_page->frame_number);
					ptr_to_current_metadata = ptr_to_new_frame + new_page_move_counter;
				}
			}

		}
	}

	return READ_HEAPMETADATA_IN_MEMORY(ptr_to_LA_metadata);
}

int FREE_MEMORY_IN_LAST_SEGMENT_ASIGNED(int a_client_socket){
	client* client = FIND_CLIENT_BY_SOCKET(a_client_socket);
	addressSpace* client_address_space = GET_ADDRESS_SPACE(a_client_socket);
	segment* a_segment = GET_SEGMENT_FROM_BASE(client->last_requested_segment_base, client_address_space);
	t_list* page_frame_table = a_segment->pageFrameTable;
	int free_bytes_in_segment = 0;
	int page_number = 0;
	int page_move_counter = 0;
	int new_page_move_counter = 0;
	heapMetadata* current_metadata;
	pageFrame* first_page = list_get(page_frame_table, page_number);
	int current_frame = first_page->frame_number;
	void* ptr_to_current_frame = GET_FRAME_POINTER(current_frame);
	void* ptr_to_current_metadata = ptr_to_current_frame;

	while(page_frame_table->elements_count > page_number){

		current_metadata = READ_HEAPMETADATA_IN_MEMORY(ptr_to_current_metadata);

		if(current_metadata->isFree){ // si estoy libre
			page_move_counter = page_move_counter + current_metadata->size + 5; // el +5 es para avanzar los 5b de la primera metadata
			free_bytes_in_segment += current_metadata->size;

			if(page_move_counter < page_size){ // si sigo en mi pagina after moverme
				ptr_to_current_metadata = ptr_to_current_metadata + current_metadata->size + 5;
			}else{ // si no sigo en mi pagina after moverme... busco nuevo frame y puntero a la metadata del frame
				new_page_move_counter = page_move_counter - page_size; //lo que me sobro en una pagina va a estar en la otra
				page_number++;
				pageFrame* next_page = list_get(page_frame_table, page_number);
				current_frame = next_page->frame_number;
				void* ptr_to_new_frame = GET_FRAME_POINTER(current_frame);
				ptr_to_current_metadata = ptr_to_new_frame + new_page_move_counter;
			}
		}
	}

	return free_bytes_in_segment;
}

void REMOVE_FREE_PAGES_FROM_SEGMENT(segment* a_segment){
	int i;
	void* ptr_to_last_metadata = GET_LAST_METADATA(a_segment);
	heapMetadata* last_metadata = READ_HEAPMETADATA_IN_MEMORY(ptr_to_last_metadata);
	uint32_t address = ptr_to_last_metadata; // Quiero la direcc de la metadata para sacar el page number
	int page = GET_PAGE_NUMBER_FROM_ADDRESS(address, a_segment);

	if(last_metadata->isFree){
		int pages = last_metadata->size / page_size;
		int new_free_size = last_metadata->size - pages * page_size;
		WRITE_HEAPMETADATA_IN_MEMORY(ptr_to_last_metadata, new_free_size, 1);

		while(page < a_segment->pageFrameTable->elements_count){
			list_remove(a_segment->pageFrameTable, page+1);
		}
	}
}

int PORCENTAJE_ASIGNACION_MEM(int socket){
	int percentage;
	addressSpace* client_address_space = GET_ADDRESS_SPACE(socket);
	int my_segments = list_size(client_address_space->segment_table); //segmentos asignados
	int total_segments = GET_TOTAL_SEGMENTS(); // segmentos totales de mi sist

	percentage = (my_segments / total_segments) * 100;

	return percentage;
}

int GET_TOTAL_SEGMENTS(){
	int i;
	int total_segments = 0;
	int limit = list_size(all_address_spaces);

	for(i = 0; i < limit; i++){
		addressSpace* client_address_space = list_get(all_address_spaces, i);
		total_segments += list_size(client_address_space->segment_table);
	}

	return total_segments;
}

void LOG_SOCKET_METRICS(int socket){
	int percentage = PORCENTAJE_ASIGNACION_MEM(socket);
	int free_bytes_last_segment = FREE_MEMORY_IN_LAST_SEGMENT_ASIGNED(socket);

	log_info(logger,"Porcentaje de asignacion de memoria [socket %d]: %d", socket, percentage);
	log_info(logger,"Memoria disponible en ultimo segmento asignado [socket %d]: %d", socket, free_bytes_last_segment);
}

void LOG_PROGRAM_METRICS(int a_client_socket){
	client* client = FIND_CLIENT_BY_SOCKET(a_client_socket);

	log_info(logger,"Memoria total pedida por cliente %s: %d bytes", client->clientProcessId, client->total_memory_requested);
	log_info(logger,"Memoria total liberada por cliente %s: %d bytes", client->clientProcessId, client->total_memory_freed);
	// falta lo de memory leaks

}

void LOG_SYSTEM_METRICS(){
	log_info(logger,"Memoria disponible: %d bytes", memory_size);
}

void LOG_METRICS(int socket){

	LOG_SOCKET_METRICS(socket);
	LOG_PROGRAM_METRICS(socket);
	LOG_SYSTEM_METRICS();
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

void CREATE_TABLES(){
	segmentation_table = list_create();
}

void DESTROY_TABLES(){

	DESTROY_ELEMENTS(pageFrame* frame){
		frame->
	}

	list_destroy_and_destroy_elements(segmentation_table,);
}

 */
