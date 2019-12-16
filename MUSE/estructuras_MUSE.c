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
#include <math.h>
#include "estructuras_MUSE.h"

//ESTRUCTURAS DE DATOS
// --> Estan en el .h

//TODO: Agregar condicion de break en get_metadata_behind y en alguno mas creo (en el id del while)
//TODO: offset cuando mi page counter es mayor al page size

void DESTROY_CLIENT(client* a_client) {
	free(a_client->clientProcessId);
	sem_destroy(&a_client->client_sempahore);
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
		if(a_segment->isHeap) {
			for(int i = 0; i < a_segment->pageFrameTable->elements_count; i++) {
				pageFrame* a_page = list_get(a_segment->pageFrameTable, i);
				if(a_page->presenceBit) {
					FREE_MEMORY_FRAME_BITMAP(a_page->frame_number);
				} else {
					FREE_SWAP_FRAME_BITMAP(a_page->frame_number);
				}
			}
		} else {
			sem_wait(&mapped_files_semaphore);
			mappedFile* mapped_file = GET_MAPPED_FILE(a_segment->path);
			for(int i = 0; a_segment->pageFrameTable->elements_count > i; i++) {
				pageFrame* current_page = list_get(a_segment->pageFrameTable, i);
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
		}
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
	sem_wait(&bitmap_swap_semaphore);
	int limit = bitarray_get_max_bit(bitmap_swap);
	if(frame_number < limit) { //Menor estricto o amplio?
		bitarray_clean_bit(bitmap_swap, frame_number);
	} else {
		//Estas queriendo liberar un numero de frame que no existe
	}
	sem_post(&bitmap_swap_semaphore);
}

void SWAP_INIT() {
	int fd_swap = open("SWAP_FILE", O_RDWR | O_CREAT, S_IRUSR | S_IRGRP | S_IROTH); //https://stackoverflow.com/questions/2395465/create-a-file-in-linux-using-c
	swap_file = (char*) malloc(swap_size);
	ftruncate(fd_swap, swap_size);
	//swap_file = mmap(NULL, swap_size, PROT_READ, PROT_WRITE, MAP_SHARED, fd_swap, 0); "too many arguments"
	swap_file = mmap(NULL, swap_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd_swap, 0);
}

int GET_FREE_SWAP_FRAME() {
	sem_wait(&bitmap_swap_semaphore);
	int counter = bitarray_get_max_bit(bitmap_swap);
	for(int i=0;i<counter;i++){
		if(!bitarray_test_bit(bitmap_swap, i)){
			bitarray_set_bit(bitmap_swap,i);
			sem_post(&bitmap_swap_semaphore);
			return i;
		}
	}
	sem_post(&bitmap_swap_semaphore);
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
	sem_wait(&bitmap_memory_semaphore);
	int counter = bitarray_get_max_bit(bitmap_memory);
	for(int i=0;i<counter;i++){
		if(!bitarray_test_bit(bitmap_memory, i)){
			bitarray_set_bit(bitmap_memory,i);
			SUBSTRACT_MEMORY_LEFT(page_size);
			sem_post(&bitmap_memory_semaphore);
			return i;
		}
	}
	sem_post(&bitmap_memory_semaphore);
	//No consegui frames libres, tengo que ejecutar el algoritmo
	int initial_position = clock_pointer;
	int frame_found=-1;
	int iterations=0;
	pageFrame* page_to_replace;
	//Busco uso=0, modificado=0
	sem_wait(&clock_semaphore);
	while(frame_found < 0 && iterations < counter) {
		pageFrame* page_frame = dictionary_get(clock_table, string_itoa(initial_position));
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
		pageFrame* page_frame = dictionary_get(clock_table, string_itoa(initial_position));
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
		pageFrame* page_frame = dictionary_get(clock_table, string_itoa(initial_position));
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
		pageFrame* page_frame = dictionary_get(clock_table, string_itoa(initial_position));
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
	sem_post(&clock_semaphore);

	//Escribo en swap las cosas del frame que estoy entregando
	void* frame_pointer = GET_FRAME_POINTER(page_to_replace->frame_number);
	int free_swap_frame = GET_FREE_SWAP_FRAME();
	int swap_position = free_swap_frame * page_size;
	//Juega a algo el bit de modificado a la hora de escribir?
	memcpy(swap_file + swap_position, frame_pointer, page_size);
	//Seteo el page frame con presence=0 y la direccion de swap donde esta la info
	page_to_replace->presenceBit=0;
	page_to_replace->frame_number=free_swap_frame;
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
		final_base = iterative_segment->base + iterative_segment->size; //Aca habia un +1 pero pensamos que no hace falta
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
	if((pointer - mp_pointer) < page_size)
		return 0;
	return (pointer - mp_pointer) / page_size;
}

void* GET_LAST_METADATA(segment* a_segment, int *page, uint32_t *metadata_size, int *bytes_next_frame, int *bytes_current_frame) { //Devuelve el puntero adelante de la metadata
	int segment_size = a_segment->size;
	int segment_move_counter = 0;
	int page_move_counter = 0;
	int page_number = 0;
	t_list* page_frame_table = a_segment->pageFrameTable;
	void* pointer;
	void* buffer;
	bool metadata_is_splitted = false;
	int next_frame_metadata_bytes = 0;
	int current_frame_metadata_bytes = 0;

	while(segment_move_counter < segment_size && page_number < page_frame_table->elements_count) { //Mientras este en mi segmento
		pageFrame* current_frame = list_get(page_frame_table, page_number);
		page_move_counter = 0;

		if(!current_frame->presenceBit)
			BRING_FROM_SWAP(a_segment, current_frame);

		pointer = GET_FRAME_POINTER(current_frame->frame_number);

		if(metadata_is_splitted){
			heapMetadata* metadata = (heapMetadata*)malloc(sizeof(heapMetadata));
			metadata_is_splitted = false;
			// Ya se pelotudo que no esta inicialziado el buffer pero nunca va a entrar aca sin haber pasado por el if de abajo pelotudo forro
			memcpy(buffer+current_frame_metadata_bytes, pointer, next_frame_metadata_bytes);
			memcpy(metadata, buffer, 5);

			if(metadata->isFree && page_number == (page_frame_table->elements_count - 1)){
				*page = page_number;
				*metadata_size = metadata->size;
				*bytes_next_frame = next_frame_metadata_bytes;
				*bytes_current_frame = current_frame_metadata_bytes;
				free(metadata);
				return pointer + next_frame_metadata_bytes;
			}

			page_move_counter = page_move_counter  + next_frame_metadata_bytes + metadata->size;
			pointer = pointer + page_move_counter;
			free(metadata);
			free(buffer);
		}

		while(page_move_counter < page_size) {
			if(page_move_counter < page_size && page_move_counter + 5 > page_size){
				next_frame_metadata_bytes = page_move_counter + 5 - page_size;
				current_frame_metadata_bytes = next_frame_metadata_bytes - 5;
				metadata_is_splitted = true;
				buffer = malloc(5);

				memcpy(buffer, pointer, current_frame_metadata_bytes);
				// Si entra aca despues va a salir de este while y pasar de pagina
			}

			if(!metadata_is_splitted){
				heapMetadata* new_metadata = READ_HEAPMETADATA_IN_MEMORY(pointer);
				if(new_metadata->isFree && page_number == (page_frame_table->elements_count - 1)) {
					//Si estoy en la ultima pagina y la metadata esta free
					*metadata_size = new_metadata->size;
					*page = page_number;
					free(new_metadata);
					return pointer + 5;
				}
				page_move_counter = page_move_counter + new_metadata->size + 5;
				pointer = pointer + page_move_counter;
				segment_move_counter = segment_move_counter + page_move_counter;
				free(new_metadata);
			}

		}
		page_number++;
	}

	return NULL; //No deberia caer nunca aca, pero lo pongo asi Eclipse no jode
}

bool SEGMENT_CAN_BE_EXTENDED(segment* a_segment, addressSpace* an_address_space, uint32_t intended_size) {

	t_list* segment_table = an_address_space->segment_table;
	uint32_t intended_direction = a_segment->base + a_segment->size + intended_size;

	bool addr_is_from_another_segment(void *one_segment) {
		segment* iterative_segment = (segment*)one_segment;
		bool cond_1 = (a_segment->base < iterative_segment->base) && intended_direction > iterative_segment->base;
		bool cond_2 = intended_direction > iterative_segment->base && intended_direction < iterative_segment->base + iterative_segment->size;
		return cond_1 || cond_2;
	}

	return (list_count_satisfying(segment_table, addr_is_from_another_segment) == 0);


	/* Ahora extiendo a lo negro sin mirar la ultima metadata
	uint32_t real_size_needed;
	t_list* segment_table = an_address_space->segment_table;
	int page = 0;
	int bytes_next_frame;
	int bytes_current_frame;
	uint32_t metadata_size = 0;
	void* last_metadata = GET_LAST_METADATA(a_segment, &page, &metadata_size, &bytes_next_frame, &bytes_current_frame);
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

	return (list_count_satisfying(segment_table, la_direccion_pertenece_a_otro_segmento) == 0);
	 */
}

void* SEGMENT_IS_BIG_ENOUGH(segment* a_segment, uint32_t intended_size) {

	int segment_size = a_segment->size;
	int segment_move_counter = 0;
	int page_move_counter = 0;
	int page_number = 0;
	t_list* page_frame_table = a_segment->pageFrameTable;
	void* pointer;
	void* buffer;
	bool metadata_is_splitted = false;
	int next_frame_metadata_bytes = 0;
	int current_frame_metadata_bytes = 0;
	bool falseStatus = 0;
	bool trueStatus = 1;
	int bytes_a_reservar = intended_size - 5;

	if(a_segment->size - 5 < intended_size) //Si el tamanio  (-2 metadatas) no me alcanza ni miro el resto
		return NULL;
	while(segment_move_counter < segment_size && page_number < page_frame_table->elements_count) { //Mientras este en mi segmento
		pageFrame* current_frame = list_get(page_frame_table, page_number);
		page_move_counter = 0;

		if(!current_frame->presenceBit)
			BRING_FROM_SWAP(a_segment, current_frame);

		pointer = GET_FRAME_POINTER(current_frame->frame_number);

		if(metadata_is_splitted){
			heapMetadata* metadata = (heapMetadata*)malloc(sizeof(heapMetadata));
			metadata_is_splitted = false;
			// Ya se pelotudo que no esta inicialziado el buffer pero nunca va a entrar aca sin haber pasado por el if de abajo pelotudo forro
			memcpy(buffer+current_frame_metadata_bytes, pointer, next_frame_metadata_bytes);
			memcpy(metadata, buffer, 5);

			if(metadata->isFree && metadata->size >= intended_size){
				pageFrame* current_page;
				pageFrame* previous_page;
				int current_page_metadata_bytes = next_frame_metadata_bytes; //Reasigno para que los nombbres de las variables nos ayuden
				int previous_page_metadata_bytes = current_frame_metadata_bytes; // a comprender mejor el estado actual
				uint32_t free_size = metadata->size - bytes_a_reservar - 5;

				//Piso buffer con los nuevos datos de la metadata
				memcpy(buffer, &bytes_a_reservar, sizeof(int));
				memcpy(buffer+bytes_a_reservar, &falseStatus, sizeof(bool));


				//Geteo paginas y ptr to metadata spliteada
				current_page = list_get(a_segment->pageFrameTable, page_number);
				previous_page = list_get(a_segment->pageFrameTable, page_number - 1);

				void* ptr_to_current_page = GET_FRAME_POINTER(current_page->frame_number);
				void* ptr_to_previous_page = GET_FRAME_POINTER(previous_page->frame_number);
				ptr_to_previous_page += page_size - previous_page_metadata_bytes;

				//Sobrescribo la metadata
				memcpy(ptr_to_previous_page, buffer, previous_page_metadata_bytes);
				memcpy(ptr_to_current_page, buffer+previous_page_metadata_bytes, current_page_metadata_bytes);

				//Escribo la nueva metadata
				void* ptr_to_new_metadata = ptr_to_current_page + current_page_metadata_bytes + bytes_a_reservar;
				memcpy(ptr_to_new_metadata, &free_size, sizeof(uint32_t));
				memcpy(ptr_to_new_metadata+sizeof(uint32_t), &trueStatus, sizeof(bool));

				free(buffer);
				free(metadata);
				return ptr_to_current_page + current_page_metadata_bytes;
			}

			page_move_counter = page_move_counter  + next_frame_metadata_bytes + metadata->size;
			pointer = pointer + page_move_counter;
			free(metadata);
			free(buffer);
		}

		while(page_move_counter < page_size) {
			if(page_move_counter + 5 > page_size){
				next_frame_metadata_bytes = page_move_counter + 5 - page_size;
				current_frame_metadata_bytes = 5 - next_frame_metadata_bytes;
				metadata_is_splitted = true;
				buffer = malloc(5);

				memcpy(buffer, pointer, current_frame_metadata_bytes);
				// Si entra aca despues va a salir de este while y pasar de pagina
			}

			if(!metadata_is_splitted){
				heapMetadata* new_metadata = READ_HEAPMETADATA_IN_MEMORY(pointer);
				if(new_metadata->isFree && new_metadata->size >= intended_size) {
					//Si la metadata esta free y me entra en el size
					//El segmento tiene lugar
					uint32_t bytes_que_habia;
					uint32_t bytes_sobrantes;
					memcpy(&bytes_que_habia, pointer, sizeof(uint32_t));
					//Sobreescribo la metadata
					memcpy(pointer, &bytes_a_reservar, sizeof(uint32_t));
					memcpy(pointer + 4, &falseStatus, sizeof(bool));
					//Escribo la nueva metadata
					bytes_sobrantes = bytes_que_habia - bytes_a_reservar - 5;
					memcpy(pointer + bytes_a_reservar, &bytes_sobrantes, sizeof(uint32_t));
					memcpy(pointer + bytes_a_reservar + sizeof(uint32_t), &trueStatus, sizeof(bool));

					free(new_metadata);
					return pointer + 5;
				}
				page_move_counter = page_move_counter + new_metadata->size + 5;
				pointer = pointer + page_move_counter;
				segment_move_counter = segment_move_counter + page_move_counter;
				free(new_metadata);
			}

		}
		page_number++;
	}

	return NULL; //No deberia caer nunca aca, pero lo pongo asi Eclipse no jode
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

	addressSpace* unAddr = list_find(all_address_spaces, el_address_spaces_es_del_cliente);
	return unAddr;
}

void FREE_MEMORY_FRAME_BITMAP(int frame_number) {
	sem_wait(&bitmap_memory_semaphore);
	int limit = bitarray_get_max_bit(bitmap_memory);
	if(frame_number < limit) { //Menor estricto o amplio?
		bitarray_clean_bit(bitmap_memory, frame_number);
	} else {
		//Estas queriendo liberar un numero de frame que no existe
	}
	sem_post(&bitmap_memory_semaphore);
}

void* GET_FRAME_POINTER(int frame_number) {
	return mp_pointer + frame_number * page_size;
}

int CREATE_ADDRESS_SPACE(char* IP_ID) {
	addressSpace* new_address_space = (addressSpace*)malloc(sizeof(addressSpace));
	new_address_space->owner = (char*)malloc(strlen(IP_ID)+1);

	memcpy(new_address_space->owner, IP_ID, strlen(IP_ID)+1);
	new_address_space->segment_table = list_create();

	sem_wait(&addresses_space_semaphore);
	list_add(all_address_spaces, new_address_space);
	sem_post(&addresses_space_semaphore);

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
	sem_wait(&client_table_semaphore);
	bool es_el_cli_de_ese_socket(void *a_client) {
		return ((client*)a_client)->clientSocket == a_client_socket;
	}
	client* cli = list_find(client_list, es_el_cli_de_ese_socket); //Consigo el id segun el socket
	sem_post(&client_table_semaphore);

	return cli;
}

int ADD_CLIENT_TO_LIST(char* client_ID, int client_socket){
	client* new_client = (client*)malloc(sizeof(client));
	new_client->clientProcessId = (char*)malloc(strlen(client_ID)+1);
	bzero(new_client->clientProcessId, strlen(client_ID)+1);
	new_client->total_memory_freed = 0;
	new_client->total_memory_requested = 0;

	memcpy(new_client->clientProcessId, client_ID, strlen(client_ID)+1);
	memcpy(&new_client->clientSocket, &client_socket, sizeof(int));

	sem_wait(&client_table_semaphore);
	list_add(client_list, new_client);
	sem_post(&client_table_semaphore);

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
	if(config!=NULL) {
		log_info(logger,"Archivo config levantado");
	} else {
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
	sem_init(&mapped_files_semaphore,0,1);
	sem_init(&bitmap_memory_semaphore,0,1);
	sem_init(&bitmap_swap_semaphore, 0, 1);
	sem_init(&client_table_semaphore,0,1);
	sem_init(&addresses_space_semaphore,0,1);
	sem_init(&clock_semaphore,0,1);
}

void DESTROY_SEMAPHORES(){
	sem_destroy(&mp_semaphore);
	sem_destroy(&logger_semaphore);
	sem_destroy(&segmentation_table_semaphore);
	sem_destroy(&memory_controller_semaphore);
	sem_destroy(&mapped_files_semaphore);
	sem_destroy(&bitmap_memory_semaphore);
	sem_destroy(&bitmap_swap_semaphore);
	sem_destroy(&client_table_semaphore);
	sem_destroy(&addresses_space_semaphore);
	sem_destroy(&clock_semaphore);
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
		//int aux = size-5; why -5?
		memcpy(pointer,&size,sizeof(uint32_t));
		memcpy(pointer+sizeof(uint32_t),&status,sizeof(bool));
		SUBSTRACT_MEMORY_LEFT(5);
		log_info(logger,"Se escribio la metadata en memoria");
	}else{
		//aca hay que ver que hacemos si no hay espacio en memoria
		//para escribir un metadata

		sem_wait(&logger_semaphore);
		log_error(logger,"No hay espacio en memoria");
		sem_post(&logger_semaphore);

	}
}

heapMetadata* READ_HEAPMETADATA_IN_MEMORY(void* pointer){
	heapMetadata* new_metadata = (heapMetadata*)malloc(sizeof(heapMetadata));

	memcpy(&new_metadata->size, pointer,sizeof(uint32_t));
	memcpy(&new_metadata->isFree, pointer+sizeof(uint32_t),sizeof(bool));

	return new_metadata;
}


segment* GET_SEGMENT_FROM_ADDRESS(uint32_t address, addressSpace* address_space){
	int iterator = 0;
	while(address_space->segment_table->elements_count > iterator) {
		segment* a_segment = list_get(address_space->segment_table, iterator);
		if(address >= a_segment->base && address < (a_segment->base + a_segment->size))
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
		page_size_plus_base += page_size; //page_size_plus_base = page_size + a_segment->base;

		if(page_size_plus_base > address){
			pageFrame* page = list_get(page_frame_table, page_number);

			if(!page->presenceBit)
				BRING_FROM_SWAP(a_segment, page);

			return page->frame_number;
		}
		page_number++;
	}
	return -1;
}

int GET_PAGE_NUMBER_FROM_ADDRESS(uint32_t address, segment* a_segment){
	int page_number = 0;
	t_list* page_frame_table = a_segment->pageFrameTable;
	uint32_t page_size_plus_base = a_segment->base;

	while(page_frame_table->elements_count > page_number && page_size_plus_base < address){
		page_size_plus_base += page_size;
		if(page_size_plus_base > address){
			return page_number;
		}
		page_number++;
	}
	return NULL;
}

// Cada vez que hago un Free() invoco esta funcion que recorre toda la TDP mergeando las metadatas libres y borra las paginas que estan
// todas free

//TODO: DEFINIR EL OFFSET AL CORRER DE PAG
void MERGE_CONSECUTIVES_FREE_BLOCKS(segment* a_segment){

	int segment_move_counter = 0;
	int page_move_counter = 0;
	int page_number = 0;
	t_list* page_frame_table = a_segment->pageFrameTable;
	void* pointer;
	void* next_pointer;
	void* buffer;
	bool metadata_is_splitted = false;
	int next_frame_metadata_bytes = 0;
	int current_frame_metadata_bytes = 0;
	bool falseStatus = 0;
	bool trueStatus = 1;
	bool previous_is_free = false;
	bool base_splitted = false;
	void* ptr_to_last_free_metadata;
	void* old_pointer;
	heapMetadata* new_metadata;
	int offset;
	int pages_in_the_middle;
	heapMetadata* metadata;


	while(segment_move_counter < a_segment->size && page_number < page_frame_table->elements_count) { //Mientras este en mi segmento
		pageFrame* current_frame = list_get(page_frame_table, page_number);
		page_move_counter = 0 + offset;

		if(!current_frame->presenceBit)
			BRING_FROM_SWAP(a_segment, current_frame);

		pointer = GET_FRAME_POINTER(current_frame->frame_number);

		//Este if hace que no pueda entrar y queda ciclando forever
		if(metadata_is_splitted){
			metadata = (heapMetadata*)malloc(sizeof(heapMetadata));
			memcpy(buffer+current_frame_metadata_bytes, pointer, next_frame_metadata_bytes);
			memcpy(metadata, buffer, 5);

			if(metadata->isFree && previous_is_free){
				//mergeo
				metadata_is_splitted = false;
				uint32_t total_size = metadata->size + new_metadata->size + 5;
				WRITE_HEAPMETADATA_IN_MEMORY(old_pointer, total_size, 1);
				pointer = old_pointer;
			} else{
				//se vuelve mi base
				//pointer = old_pointer + page_size - current_frame_metadata_bytes; ke flashamo?
				base_splitted = true;
				pointer = old_pointer + (page_size - GET_OFFSET_FROM_POINTER(old_pointer) - current_frame_metadata_bytes);
			}
		}

		while(page_move_counter < page_size) {

			if(base_splitted){
				metadata_is_splitted = true;
			}

			if(page_move_counter + 5 > page_size  && !base_splitted){
				next_frame_metadata_bytes = page_move_counter + 5 - page_size;
				current_frame_metadata_bytes = 5 - next_frame_metadata_bytes;
				metadata_is_splitted = true;
				buffer = malloc(5);
				old_pointer = pointer;

				memcpy(buffer, pointer, current_frame_metadata_bytes);
				break;
				// Si entra aca despues va a salir de este while y pasar de pagina
			}

			if(!metadata_is_splitted){ // ************* BASE NORMAL *************
				new_metadata = READ_HEAPMETADATA_IN_MEMORY(pointer);
				if(new_metadata->isFree) {
					page_move_counter = page_move_counter + new_metadata->size + 5;
					previous_is_free = true;

					if((page_move_counter - offset) / page_size > 0){
						pages_in_the_middle = ((page_move_counter - offset) / page_size);
						page_number = page_number + pages_in_the_middle + 1;
						current_frame = list_get(page_frame_table, page_number);

						//Si me da null es porque me fui al chori, entonces lo corto de prepo
						if(current_frame == NULL)
							return;

						page_move_counter = (page_move_counter + new_metadata->size + 5) % page_size;
						next_pointer = GET_FRAME_POINTER(current_frame->frame_number) + page_move_counter;
					} else{
						pages_in_the_middle = 0;
						next_pointer = pointer + page_move_counter;
					}

					if(page_move_counter + 5 > page_size){
						//spliteada, salgo y laburo

					} else{
						heapMetadata* next_metadata = READ_HEAPMETADATA_IN_MEMORY(next_pointer);
						if(next_metadata->isFree){
							//mergeo
							uint32_t total_size = new_metadata->size + next_metadata->size + 5; //el 5 de la otra metadata
							WRITE_HEAPMETADATA_IN_MEMORY(pointer, total_size, 1);
							page_move_counter = page_move_counter - new_metadata->size - 5;
							//de aca vuelvo a preguntar con el pointer a la primer metadata free y me voy a correr ahora con su nuevo size
						} else{
							//me paro en el proximo y vuelvo a preguntar again
							pointer = next_pointer;
						}
					}
				} else{
					//la base esta usada, voy a la proxima y se hace mi base
					page_move_counter = page_move_counter + new_metadata->size + 5;

					if((page_move_counter - offset) / page_size > 0){
						pages_in_the_middle = ((page_move_counter - offset) / page_size);
						page_number = page_number + pages_in_the_middle + 1;
						current_frame = list_get(page_frame_table, page_number);

						//Si me da null es porque me fui al chori, entonces lo corto de prepo
						if(current_frame == NULL)
							return;

						page_move_counter = (page_move_counter + new_metadata->size + 5) % page_size;
						pointer = GET_FRAME_POINTER(current_frame->frame_number) + page_move_counter;
					} else{
						pages_in_the_middle = 0;
						page_move_counter = page_move_counter + new_metadata->size + 5;
						pointer = pointer + page_move_counter;
					}
				}
			} else{ // ************* BASE SPLITEADA *************
				// la metadata que es mi base esta spliteada
				if(metadata->isFree) {
					page_move_counter = page_move_counter + metadata->size + 5;
					previous_is_free = true;

					if((page_move_counter - offset) / page_size > 0){
						pages_in_the_middle = ((page_move_counter - offset) / page_size);
						page_number = page_number + pages_in_the_middle + 1;
						current_frame = list_get(page_frame_table, page_number);

						//Si me da null es porque me fui al chori, entonces lo corto de prepo
						if(current_frame == NULL)
							return;

						page_move_counter = (page_move_counter + metadata->size + 5) % page_size;
						next_pointer = GET_FRAME_POINTER(current_frame->frame_number) + page_move_counter;
					} else{
						pages_in_the_middle = 0;
						next_pointer = pointer + page_move_counter;
					}

					if(page_move_counter + 5 > page_size){
						//spliteada, salgo y laburo

					} else{
						heapMetadata* next_metadata = READ_HEAPMETADATA_IN_MEMORY(next_pointer);
						if(next_metadata->isFree){
							//mergeo
							//sobreescribo la metadata spliteada...

							//Calculo el nuevo size y lo pongo en el buffer
							uint32_t total_size = new_metadata->size + next_metadata->size + 5; //el 5 de la otra metadata
							memcpy(buffer, &total_size, sizeof(uint32_t));

							//Pego lo primero
							memcpy(pointer, buffer, current_frame_metadata_bytes);

							//Me traigo el pointer a la pagina siguente para terminar de escribir
							pageFrame* page_after_metadata = list_get(page_frame_table, page_number - pages_in_the_middle);
							void* pointer_to_page_after_metadata = GET_FRAME_POINTER(page_after_metadata->frame_number);
							memcpy(pointer_to_page_after_metadata, buffer+current_frame_metadata_bytes, next_frame_metadata_bytes);

							//Actualizo la metadata con el nuevo valor
							memcpy(metadata, buffer, 5);

							page_move_counter = page_move_counter - metadata->size - 5;
							//de aca vuelvo a preguntar con el pointer a la primer metadata free y me voy a correr ahora con su nuevo size
						} else{
							//me paro en el proximo y vuelvo a preguntar again
							pointer = next_pointer;
						}
					}
				} else{
					//la base esta usada, voy a la proxima y se hace mi base
					page_move_counter = page_move_counter + new_metadata->size + 5;

					if((page_move_counter - offset) / page_size > 0){
						pages_in_the_middle = ((page_move_counter - offset) / page_size);
						page_number = page_number + pages_in_the_middle + 1;
						current_frame = list_get(page_frame_table, page_number);

						//Si me da null es porque me fui al chori, entonces lo corto de prepo
						if(current_frame == NULL)
							return;

						page_move_counter = (page_move_counter + new_metadata->size + 5) % page_size;
						pointer = GET_FRAME_POINTER(current_frame->frame_number) + page_move_counter;
					} else{
						pages_in_the_middle = 0;
						page_move_counter = page_move_counter + new_metadata->size + 5;
						pointer = pointer + page_move_counter;
					}
				}

			}

		}
		page_number++;
	}
}


/*
void MERGE_CONSECUTIVES_FREE_BLOCKS(segment* a_segment){
	int page_move_counter = 0;
	int new_page_move_counter = 0;
	int page_number = 0;
	heapMetadata* current_metadata;
	heapMetadata* next_metadata;
	t_list* page_frame_table = a_segment->pageFrameTable;
	pageFrame* first_page = list_get(page_frame_table, page_number);

	if(!first_page->presenceBit)
		BRING_FROM_SWAP(a_segment, first_page);

	int current_frame = first_page->frame_number;
	void* ptr_to_current_frame = GET_FRAME_POINTER(current_frame);
	void* ptr_to_current_metadata = ptr_to_current_frame;

	while(page_frame_table->elements_count > page_number){

		current_metadata = READ_HEAPMETADATA_IN_MEMORY(ptr_to_current_metadata);

		if(current_metadata->isFree){ // si estoy libre
			page_move_counter = page_move_counter + current_metadata->size + 5; // el +5 es para avanzar los 5b de la primera metadata
			log_trace(logger,"Metadata actual libre");

			if(page_move_counter < page_size){ // si sigo en mi pagina after moverme
				void* ptr_to_next_metadata = ptr_to_current_metadata + current_metadata->size + 5;
				next_metadata = READ_HEAPMETADATA_IN_MEMORY(ptr_to_next_metadata);
				log_trace(logger,"Sigo en mi pagina despues de moverme");

				if(next_metadata->isFree){ // si mi proxima metadata esta libre
					uint32_t total_size = current_metadata->size + next_metadata->size + 5; // el +5 de la otra metadata
					page_move_counter = page_move_counter - current_metadata->size - 5; // anulo el desplazamiento, la proxima lo hace con el nuevo size
					WRITE_HEAPMETADATA_IN_MEMORY(ptr_to_current_metadata, total_size, 1);
					log_trace(logger,"Proxima metadata libre");
					log_info(logger,"Mergeo metadatas");
				} else{ // si el proximo no esta free... me paro en ese y se vuelve mi actual
					ptr_to_current_metadata = ptr_to_next_metadata;
					log_trace(logger, "Proxima metadata usada");
					log_info(logger,"Me paro en la proxima metadata");
				}
			} else{ // si no sigo en mi pagina after moverme... busco nuevo frame y puntero a la metadata del frame
				new_page_move_counter = page_move_counter - page_size; //lo que me sobro en una pagina va a estar en la otra
				page_number++;
				log_trace(logger,"Paso de pagina despues de moverme");
				if(page_number < page_frame_table->elements_count){
					pageFrame* next_page = list_get(page_frame_table, page_number);

					if(!next_page->presenceBit)
						BRING_FROM_SWAP(a_segment, next_page);

					current_frame = next_page->frame_number;
					void* ptr_to_new_frame = GET_FRAME_POINTER(current_frame);
					void* ptr_to_first_metadata_new_frame = ptr_to_new_frame + new_page_move_counter;
					next_metadata = READ_HEAPMETADATA_IN_MEMORY(ptr_to_first_metadata_new_frame);


					if(next_metadata->isFree){ // si la proxima metadata que esta en otro frame esta libre
						uint32_t total_size = current_metadata->size + next_metadata->size + 5;
						page_move_counter = page_move_counter - current_metadata->size - 5;
						WRITE_HEAPMETADATA_IN_MEMORY(ptr_to_current_metadata, total_size, 1);
						page_number--;
						log_trace(logger,"Proxima metadata libre");
						log_info(logger,"Mergeo metadatas");
					} else{ // si no esta libre
						ptr_to_current_metadata = ptr_to_first_metadata_new_frame;
						page_move_counter = new_page_move_counter;
						log_trace(logger, "Proxima metadata usada");
						log_info(logger,"Me paro en la proxima metadata");

					}
				}
			}
		} else{ // si no esta free... me paro en el proximo y se vuelve mi actual
			page_move_counter = page_move_counter + current_metadata->size + 5;
			log_trace(logger,"Metadata actual usada");

			if(page_move_counter < page_size){ // si sigo en mi pagina after moverme
				void* ptr_to_next_metadata = ptr_to_current_metadata + current_metadata->size + 5;
				ptr_to_current_metadata = ptr_to_next_metadata;
				log_trace(logger,"Sigo en mi pagina despues de moverme");
				log_info(logger,"Me paro en la proxima metadata");
			} else{ // si no sigo en mi pagina after moverme... busco nuevo frame y puntero a la metadata del frame
				new_page_move_counter = page_move_counter - page_size;
				page_number++;
				log_trace(logger,"Paso de pagina despues de moverme");

				if(page_number < page_frame_table->elements_count) {
					pageFrame* next_page = list_get(page_frame_table, page_number);
					current_frame = next_page->frame_number;
					void* ptr_to_new_frame = GET_FRAME_POINTER(current_frame);
					void* ptr_to_first_metadata_new_frame = ptr_to_new_frame + new_page_move_counter;
					ptr_to_current_metadata = ptr_to_first_metadata_new_frame;
					page_move_counter = new_page_move_counter;
					log_info(logger,"Me paro en la proxima metadata");

				}
			}

		}
	}

	REMOVE_FREE_PAGES_FROM_SEGMENT(a_segment);
	log_info(logger,"Frames libres mergeados");
}
 */


int FREE_USED_FRAME(uint32_t address, addressSpace* address_space) {
	segment* a_segment = GET_SEGMENT_FROM_ADDRESS(address, address_space);
	uint32_t metadata_size;
	int offset = TRANSLATE_DL_TO_OFFSET_IN_THE_PAGE(address);
	if(a_segment != NULL && a_segment->isHeap){
		int frame = GET_FRAME_FROM_ADDRESS(address, a_segment);
		if(frame >= 0){
			//TODO: para ver si esta spliteada puedo valerme del page_size, el frame donde esta la metadata, el anterior
			// (que lo saco con cuentitas), el corrimiento de los punteros y algo mas? Con tutti eso hago las cuentas
			// y puedo laburar tranquilo.
			bool is_splitted;
			int md_page_num = 0;
			int md_offset = 0;
			heapMetadata* md_behind_address = GET_METADATA_BEHIND_ADDRESS(address, a_segment, &md_page_num, &md_offset);
			is_splitted = md_offset + 5 > page_size;
			//Tengo que escribir segun si la metadata esta spliteada o no
			if(is_splitted) {
				//Esta splitteada
				bool false_status = 1;
				void* buffer = (void*)malloc(5);

				memcpy(buffer , &md_behind_address->size, sizeof(uint32_t));
				memcpy(buffer + sizeof(uint32_t), &false_status, sizeof(bool));

				int previous_frame = GET_FRAME_FROM_ADDRESS(address - page_size, a_segment); //Si le resto a la direccion una pagina, estare en la anterior
				int bytes_to_write_in_second_frame = md_offset;
				int bytes_to_write_in_first_frame = 5 - bytes_to_write_in_second_frame;
				/*
				int bytes_to_write_in_first_frame = page_size - md_offset;
				int bytes_to_write_in_second_frame = 5 - bytes_to_write_in_first_frame;
				 */
				int pointer_first_frame_md = GET_FRAME_POINTER(previous_frame) + md_offset;
				int pointer_second_frame_md = GET_FRAME_POINTER(frame);

				memcpy(pointer_first_frame_md, buffer, bytes_to_write_in_first_frame);
				memcpy(pointer_second_frame_md, buffer + bytes_to_write_in_first_frame, bytes_to_write_in_second_frame);

				free(buffer);
				metadata_size = md_behind_address->size;
			} else {
				//No esta spliteada
				void* ptr_to_frame = GET_FRAME_POINTER(frame);
				void* ptr_to_metadata = ptr_to_frame + offset - 5;
				heapMetadata* frame_metadata = READ_HEAPMETADATA_IN_MEMORY(ptr_to_metadata); //donde hago el free
				WRITE_HEAPMETADATA_IN_MEMORY(ptr_to_metadata, frame_metadata->size, 1);
				metadata_size = frame_metadata->size;
				free(frame_metadata);
			}
			free(md_behind_address);
			MERGE_CONSECUTIVES_FREE_BLOCKS(a_segment);
			return metadata_size; // Asi puedo saber cuanto libera y registrarlo en las metricas
		} else log_error(logger,"La pagina no se encuentra en memoria"); // generar page fault y swappear
	} else log_error(logger,"El segmento no se encuentra en memoria o no es de heap");
	return -1;
}

void* GET_N_BYTES_DATA_FROM_MUSE(addressSpace* address_space, uint32_t src, size_t bytes_a_copiar) {
 	segment* a_segment = GET_SEGMENT_FROM_ADDRESS(src, address_space);
 	t_list* page_frame_table = a_segment->pageFrameTable;
 	int page_move_counter = 0;
 	int page_number = GET_PAGE_NUMBER_FROM_ADDRESS(src, a_segment);
 	int offset = 0;
 	int bytes = bytes_a_copiar;
 	void* data = malloc(bytes_a_copiar);

 	if(a_segment != NULL){
 		src = TRANSLATE_DL_TO_OFFSET_IN_THE_PAGE(src);
 		pageFrame* current_page = list_get(page_frame_table, page_number);
 		void* ptr_to_frame = GET_FRAME_POINTER(current_page->frame_number);
 		void* ptr_to_data = ptr_to_frame + src; // va a donde empieza la data a leer

 		while(page_frame_table->elements_count > page_number && bytes > 0){
 			if(!current_page->presenceBit){
 				BRING_FROM_SWAP(a_segment, current_page);
 			}

 			void* ptr_to_frame = GET_FRAME_POINTER(current_page->frame_number);
 			/*
 			void* ptr_to_data = ptr_to_frame + src; // va a donde empieza la data a leer
 			page_move_counter = ptr_to_data + bytes_a_copiar;
 			*/
 			void* ptr_to_data = ptr_to_frame + src; // va a donde empieza la data a leer
 			page_move_counter = src + bytes_a_copiar;

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
 	} else {
 		data = NULL;
 	}

 	return data;
}

int TRANSLATE_DL_TO_OFFSET_IN_THE_PAGE(uint32_t dl){
	//Is this an offset?
	int r = dl / page_size;
	int df = dl - (page_size * r);

	return df;
}

// no estoy seguro de como me llega la data por parametro lo dejo como void* data
int WRITE_N_BYTES_DATA_TO_MUSE(uint32_t dst, addressSpace* address_space, size_t bytes_a_copiar, void* data){
	segment* a_segment = GET_SEGMENT_FROM_ADDRESS(dst, address_space);
	t_list* page_frame_table = a_segment->pageFrameTable;
	int page_move_counter = 0;
	int page_number = GET_PAGE_NUMBER_FROM_ADDRESS(dst, a_segment);
	int offset = 0;
	int bytes = bytes_a_copiar;
	int metadata_page_num;
	int initial_offset;
	int metadata_offset;
	int pages_in_the_middle;
	bool first_iteration = true;
	bool isSplit; //Porque lo pide la f(x)

	if(a_segment != NULL){
		//initial_offset = TRANSLATE_DL_TO_DF(dst);
		pageFrame* current_page = list_get(page_frame_table, page_number);

		while(page_frame_table->elements_count > page_number && bytes > 0){
			if(!current_page->presenceBit){
				BRING_FROM_SWAP(a_segment, current_page);
			}

			heapMetadata* metadata = GET_METADATA_BEHIND_ADDRESS_V2(dst, a_segment, &metadata_page_num, &metadata_offset, &isSplit);

			if((((dst - a_segment->base)/page_size) - metadata_page_num) > 0) {
				pages_in_the_middle = ((dst - a_segment->base)/page_size) - metadata_page_num;
			} else {
				pages_in_the_middle = 0;
			}

			initial_offset = (pages_in_the_middle * page_size) + TRANSLATE_DL_TO_OFFSET_IN_THE_PAGE(dst) - metadata_offset; //(pages_in_the_middle * page_size) + TRANSLATE_DL_TO_DF(dst) - (metadata_offset + 5);

			if(metadata->size >= bytes_a_copiar + initial_offset){ //Si tengo espacio suficiente para copiar, copio //Era > y lo pongo >=
				void* ptr_to_frame = GET_FRAME_POINTER(current_page->frame_number);
				void* ptr_to_data;

				if(first_iteration) { //Si es la primera pagina, puede ser que arranque corrido
					ptr_to_data = ptr_to_frame + TRANSLATE_DL_TO_OFFSET_IN_THE_PAGE(dst); // va a donde empieza la data a escribir
					first_iteration = false;
				} else { //En todas las demas paginas, escribo desde el principio
					ptr_to_data = ptr_to_frame;
				}

				page_move_counter = TRANSLATE_DL_TO_OFFSET_IN_THE_PAGE(dst) + bytes_a_copiar;

				if(page_move_counter < page_size){ // si al sumar esos bytes sigo en mi pagina
					memcpy(ptr_to_data, data+offset, bytes_a_copiar);
					bytes = bytes - bytes_a_copiar;
					current_page->useBit = 1;
					current_page->modifiedBit = 1;

				} else{ // si al sumar esos bytes me pase... voy a la pagina siguiente y copio lo que me queda
					int bytes_restantes_en_frame_siguiente = page_move_counter - page_size;
					int bytes_en_frame_anterior = bytes_a_copiar - bytes_restantes_en_frame_siguiente;
					memcpy(ptr_to_data, data+offset, bytes_en_frame_anterior);
					bytes = bytes - bytes_en_frame_anterior;
					current_page->useBit = 1;
					current_page->modifiedBit = 1;
					page_number++;
					if(page_number < page_frame_table->elements_count){
						current_page = list_get(page_frame_table, page_number);
						bytes_a_copiar = bytes_restantes_en_frame_siguiente;
						offset = offset + bytes_en_frame_anterior;
					} //ultima pagina, no se puede seguir
				}
				free(metadata);
			} else {
				free(metadata);
				return -1; // else no tengo espacio suficiente
			}
		}
		return 0;
	} else {
		return -1;
	}
}

heapMetadata* GET_METADATA_BEHIND_ADDRESS(uint32_t address, segment* a_segment, int *metadata_page_num, int *metadata_offset){
	t_list* page_frame_table = a_segment->pageFrameTable;
	int segment_move_counter = 0;
	int page_move_counter = 0;
	int global_move_counter = 0;
	int page_number = 0;
	void* pointer;
	void* buffer;
	bool metadata_is_splitted = false;
	int next_frame_metadata_bytes = 0;
	int current_frame_metadata_bytes = 0;
	void* buffer_previous_metadata = malloc(5);

	while(segment_move_counter < a_segment->size && page_number < page_frame_table->elements_count) { //Mientras este en mi segmento
		pageFrame* current_frame = list_get(page_frame_table, page_number);
		page_move_counter = 0;

		if(!current_frame->presenceBit)
			BRING_FROM_SWAP(a_segment, current_frame);

		pointer = GET_FRAME_POINTER(current_frame->frame_number);

		if(metadata_is_splitted){
			heapMetadata* metadata = (heapMetadata*)malloc(sizeof(heapMetadata));
			metadata_is_splitted = false;
			// Ya se pelotudo que no esta inicialziado el buffer pero nunca va a entrar aca sin haber pasado por el if de abajo pelotudo forro
			memcpy(buffer+current_frame_metadata_bytes, pointer, next_frame_metadata_bytes);
			memcpy(buffer_previous_metadata+current_frame_metadata_bytes, pointer, next_frame_metadata_bytes);
			memcpy(metadata, buffer, 5);

			page_move_counter = page_move_counter + next_frame_metadata_bytes + metadata->size;
			//*metadata_offset = page_move_counter;
			*metadata_offset = page_move_counter % page_size;
			global_move_counter += page_move_counter;
			pointer = pointer + page_move_counter;
			free(metadata);
			free(buffer);

			if(global_move_counter >= address){ //Pongo el >=, si no se va al chori
				//retorno por caso feo (metadata splitted)
				heapMetadata* the_metadata = (heapMetadata*)malloc(sizeof(heapMetadata));
				memcpy(the_metadata, buffer_previous_metadata, 5);
				*metadata_page_num = page_number;
				free(buffer_previous_metadata);
				return the_metadata;
			}
		}

		while(page_move_counter < page_size) {
			if(page_move_counter < page_size && page_move_counter + 5 > page_size){
				next_frame_metadata_bytes = page_move_counter + 5 - page_size;
				current_frame_metadata_bytes = 5 - next_frame_metadata_bytes;
				metadata_is_splitted = true;
				buffer = malloc(5);

				memcpy(buffer, pointer, current_frame_metadata_bytes);
				memcpy(buffer_previous_metadata, pointer, current_frame_metadata_bytes);

				// Si entra aca despues va a salir de este while y pasar de pagina
				break;
			}

			if(!metadata_is_splitted){
				heapMetadata* new_metadata = READ_HEAPMETADATA_IN_MEMORY(pointer);
				memcpy(buffer_previous_metadata, &new_metadata->size, sizeof(uint32_t));
				memcpy(buffer_previous_metadata, &new_metadata->isFree, sizeof(bool));
				if(global_move_counter > address) {
					//retorno por caso feliz (no splitted)
					heapMetadata* the_metadata = (heapMetadata*)malloc(sizeof(heapMetadata));
					memcpy(the_metadata, buffer_previous_metadata, 5);
					*metadata_page_num = page_number;
					free(buffer_previous_metadata);
					return the_metadata;
				}

				*metadata_offset = page_move_counter;
				page_move_counter = page_move_counter + new_metadata->size + 5;
				pointer = pointer + page_move_counter;
				global_move_counter += page_move_counter;
				segment_move_counter = segment_move_counter + page_move_counter;
				free(new_metadata);
			}

		}
		page_number++;
	}

	return NULL; //No deberia caer nunca aca, pero lo pongo asi Eclipse no jode
}

/*
heapMetadata* GET_METADATA_BEHIND_ADDRESS(uint32_t address, t_list* page_frame_table, int *metadata_page_num, int *metadata_offset){
	heapMetadata* current_metadata;
	int page_number = 0;
	pageFrame* current_page = list_get(page_frame_table, 0);
	int new_page_move_counter = 0;
	int page_move_counter = 0;
	int global_move_counter = 0;
	void* buffer;
	bool metadata_splitted = false;
	void* ptr_to_next_metadata;
	int bytes_metadata_current_page;

	void* ptr_to_LA_metadata;
	void* ptr_to_current_metadata = GET_FRAME_POINTER(current_page->frame_number);

	while(global_move_counter < address){
		current_metadata = READ_HEAPMETADATA_IN_MEMORY(ptr_to_current_metadata);
		page_move_counter = new_page_move_counter;

		while(page_move_counter < page_size && global_move_counter < address){ // mientras sigo en mi pagina y no encontre la metadata
			global_move_counter = global_move_counter + page_move_counter;
			if(page_move_counter < page_size){ // si sigo en mi pagina after moverme
				if(page_move_counter < page_size && page_move_counter + 5 > page_size ){
					//Metadata spliteada
					metadata_splitted = true;
				} else {
					//leo mi metadata para saber donde esta la proxima y asi moverme hasta ahi

					//aca hago READ
					ptr_to_next_metadata = ptr_to_current_metadata + current_metadata->size + 5;
					ptr_to_LA_metadata = ptr_to_current_metadata;
					free(current_metadata);
					//current_metadata = READ_HEAPMETADATA_IN_MEMORY(ptr_to_next_metadata);
					page_move_counter = page_move_counter + current_metadata->size + 5;
				}
			} else{ // cambio de pagina
				page_number++;
				if(page_number < page_frame_table->elements_count){
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
				free(current_metadata); //Ojo con este free
			}

			if(metadata_splitted){
				//tengo que armar mi metadata spliteada para asi saber donde esta la proxima metadata y moverme
				int bytes_metadata_next_page = page_move_counter + 5 - page_size;
				int bytes_metadata_current_page = 5 - bytes_metadata_next_page;
				heapMetadata* metadata = (heapMetadata*)malloc(sizeof(heapMetadata));
				buffer = malloc(5);
				memcpy(buffer, ptr_to_next_metadata, bytes_metadata_current_page);
				pageFrame* next_page = list_get(page_frame_table, page_number++);
				void* ptr_to_next_frame = GET_FRAME_POINTER(next_page->frame_number);
				memcpy(buffer+bytes_metadata_current_page, ptr_to_next_frame, bytes_metadata_next_page);
				memcpy(metadata, buffer, 5);
				page_move_counter = metadata->size + bytes_metadata_next_page;
				ptr_to_current_metadata = ptr_to_next_frame + page_move_counter;
				page_number++;

				free(metadata);
				free(buffer);
		}
	}

	if(page_number < page_frame_table->elements_count) {
 *metadata_page_num = page_number;
	} else {
 *metadata_page_num = page_number - 1;
	}
	//Si me pase, le resto uno y me lo llevo. Si no, me lo llevo como esta

 *metadata_offset = (ptr_to_LA_metadata - mp_pointer) % page_size;
	return READ_HEAPMETADATA_IN_MEMORY(ptr_to_LA_metadata);
}
 */

int FREE_MEMORY_IN_LAST_SEGMENT_ASIGNED(int a_client_socket){
	client* client = FIND_CLIENT_BY_SOCKET(a_client_socket);
	addressSpace* client_address_space = GET_ADDRESS_SPACE(a_client_socket);
	segment* a_segment = GET_SEGMENT_FROM_BASE(client->last_requested_segment_base, client_address_space);
	t_list* page_frame_table = a_segment->pageFrameTable;
	int free_bytes_in_segment = 0;
	int page_number = 0;
	int page_move_counter = 0;
	int new_page_move_counter = 0;
	heapMetadata* current_metadata; //No hay que maloquearlo porque lo llena READ_HEAPMETADATA
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
				if(page_number < page_frame_table->elements_count){
					pageFrame* next_page = list_get(page_frame_table, page_number);
					current_frame = next_page->frame_number;
					void* ptr_to_new_frame = GET_FRAME_POINTER(current_frame);
					ptr_to_current_metadata = ptr_to_new_frame + new_page_move_counter;
				}
			}
		} else {
			page_move_counter = page_move_counter + current_metadata->size + 5; // el +5 es para avanzar los 5b de la primera metadata
			if(page_move_counter < page_size){ // si sigo en mi pagina after moverme
				ptr_to_current_metadata = ptr_to_current_metadata + current_metadata->size + 5;
			}else{ // si no sigo en mi pagina after moverme... busco nuevo frame y puntero a la metadata del frame
				new_page_move_counter = page_move_counter - page_size; //lo que me sobro en una pagina va a estar en la otra
				page_number++;
				if(page_number < page_frame_table->elements_count){
					pageFrame* next_page = list_get(page_frame_table, page_number);
					current_frame = next_page->frame_number;
					void* ptr_to_new_frame = GET_FRAME_POINTER(current_frame);
					ptr_to_current_metadata = ptr_to_new_frame + new_page_move_counter;
				}
			}
		}
		free(current_metadata);
	}

	return free_bytes_in_segment;
}

void REMOVE_FREE_PAGES_FROM_SEGMENT(segment* a_segment){
	int page = 0;
	uint32_t metadata_size;
	int bytes_next_frame;
	int bytes_current_frame;
	void* ptr_to_last_metadata = GET_LAST_METADATA(a_segment, &page, &metadata_size, &bytes_next_frame, &bytes_current_frame);

	int pages = metadata_size / page_size; //Cantidad de paginas que ocupa de mas
	int new_free_size = metadata_size - pages * page_size;
	WRITE_HEAPMETADATA_IN_MEMORY(ptr_to_last_metadata, new_free_size, 1); // cambiar

	while(page + 1 < a_segment->pageFrameTable->elements_count){ //Mientras la page no sea la ultima
		list_remove_and_destroy_element(a_segment->pageFrameTable, page+1, DESTROY_PAGE);
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

int TOTAL_MEMORY_LEAKS(int a_client_socket){
	int leaked_bytes = 0;
	int segment_counter = 0;
	int page_number = 0;
	int page_move_counter = 0;
	int new_page_move_counter = 0;
	heapMetadata* current_metadata;
	addressSpace* client_address_space = GET_ADDRESS_SPACE(a_client_socket);
	t_list* segment_table = client_address_space->segment_table;
	void* pointer;
	bool metadata_is_splitted = false;
	void* buffer;
	int current_frame_metadata_bytes;
	int next_frame_metadata_bytes;
	int memory_leaks = 0;
	int segment_move_counter = 0;
	int pages_in_the_middle;
	int offset;


	while(segment_table->elements_count > segment_counter){
		segment* a_segment = list_get(segment_table, segment_counter);

		if(a_segment->isHeap){
			t_list* page_frame_table = a_segment->pageFrameTable;

			while(page_frame_table->elements_count > page_number){

				pageFrame* current_page = list_get(page_frame_table, page_number);
				page_move_counter = 0;

				if(!current_page->presenceBit)
					BRING_FROM_SWAP(a_segment, current_page);

				pointer = GET_FRAME_POINTER(current_page->frame_number);

				if(metadata_is_splitted){
					// nunca va a entrar aca sin haber pasado por el if de abajo
					heapMetadata* metadata = (heapMetadata*)malloc(sizeof(heapMetadata));
					metadata_is_splitted = false;

					memcpy(buffer+current_frame_metadata_bytes, pointer, next_frame_metadata_bytes);
					memcpy(metadata, buffer, 5);

					if(!metadata->isFree)
						memory_leaks += metadata->size;
					// SEGUIR DE ACA -> LA CAGA EL MOVIMIENTO HAY QUE VER PAGNIAS EN EL DIOME
					page_move_counter = page_move_counter + next_frame_metadata_bytes + metadata->size;

					if((page_move_counter - offset) / page_size > 0){ //SI LA CHINGA ES ACA
						pages_in_the_middle = ((page_move_counter - offset) / page_size);
						page_number = page_number + pages_in_the_middle + 1;
						current_page = list_get(page_frame_table, page_number);

						//Si me da null es porque me fui al chori, entonces lo corto de prepo
						if(current_page == NULL)
							return memory_leaks;

						page_move_counter = (page_move_counter + metadata->size + 5) % page_size;
						pointer = GET_FRAME_POINTER(current_page->frame_number) + page_move_counter;
					} else{
						pages_in_the_middle = 0;
						page_move_counter = page_move_counter + metadata->size + 5;
						pointer = pointer + page_move_counter;
					}

					free(metadata);
					free(buffer);
				}

				while(page_move_counter < page_size) {
					if(page_move_counter + 5 > page_size){
						next_frame_metadata_bytes = page_move_counter + 5 - page_size;
						current_frame_metadata_bytes = 5 - next_frame_metadata_bytes;
						metadata_is_splitted = true;
						buffer = malloc(5);

						memcpy(buffer, pointer, current_frame_metadata_bytes);
						break;
						// Si entra aca despues va a salir de este while y pasar de pagina
					}

					if(!metadata_is_splitted){
						heapMetadata* new_metadata = READ_HEAPMETADATA_IN_MEMORY(pointer);

						if(!new_metadata->isFree)
							memory_leaks += new_metadata->size;

						page_move_counter = page_move_counter + 5 + new_metadata->size;

						if((page_move_counter - offset) / page_size > 0){ //SI LA CHINGA ES ACA
							pages_in_the_middle = ((page_move_counter - offset) / page_size);
							page_number = page_number + pages_in_the_middle + 1;
							current_page = list_get(page_frame_table, page_number);

							//Si me da null es porque me fui al chori, entonces lo corto de prepo
							if(current_page == NULL)
								return memory_leaks;

							page_move_counter = (page_move_counter + new_metadata->size + 5) % page_size;
							pointer = GET_FRAME_POINTER(current_page->frame_number) + page_move_counter;
						} else{
							pages_in_the_middle = 0;
							page_move_counter = page_move_counter + new_metadata->size + 5;
							pointer = pointer + page_move_counter;
						}

						free(new_metadata);
					}

				}
				page_number++;
			}

		} else{
			mappedFile* mapped_file = GET_MAPPED_FILE(a_segment->path);
			leaked_bytes += mapped_file->length;
			segment_counter++;
		}
		segment_counter++;
	}

	return leaked_bytes;
}

void LOG_SOCKET_METRICS(int socket){
	int percentage = PORCENTAJE_ASIGNACION_MEM(socket);
	int free_bytes_last_segment = FREE_MEMORY_IN_LAST_SEGMENT_ASIGNED(socket);

	log_info(logger,"Porcentaje de asignacion de memoria [socket %d]: %d%%", socket, percentage);
	log_info(logger,"Memoria disponible en ultimo segmento asignado [socket %d]: %d bytes", socket, free_bytes_last_segment);
}

void LOG_PROGRAM_METRICS(int a_client_socket){
	client* client = FIND_CLIENT_BY_SOCKET(a_client_socket);
	int bytes_leaked = TOTAL_MEMORY_LEAKS(a_client_socket);

	log_info(logger,"Memoria total pedida por cliente %s: %d bytes", client->clientProcessId, client->total_memory_requested);
	log_info(logger,"Memoria total liberada por cliente %s: %d bytes", client->clientProcessId, client->total_memory_freed);
	log_info(logger,"Memory leaks del cliente %s: %d bytes", client->clientProcessId, bytes_leaked);

}

void LOG_SYSTEM_METRICS(){
	log_info(logger,"Memoria disponible: %d bytes", TOTAL_MEMORY_SIZE());
}

void LOG_METRICS(int socket){
	/*
	LOG_SOCKET_METRICS(socket);
	LOG_PROGRAM_METRICS(socket);
	LOG_SYSTEM_METRICS();
	 */
}

void BRING_FROM_SWAP(segment* a_segment, pageFrame* current_page){ //Trae de swap o del archivo de map segun si es un segmento de heap o de map
	//swap
	int swap_frame =  current_page->frame_number;
	int swap_pos = swap_frame * page_size;
	int free_frame = CLOCK();
	//Hago o no? --> dictionary_remove_and_destroy(clock_table, string_itoa(free_frame), DESTROY_PAGE);
	dictionary_put(clock_table, string_itoa(free_frame), current_page);
	void* ptr_to_free_frame = GET_FRAME_POINTER(free_frame);
	if(a_segment->isHeap){
		memcpy(ptr_to_free_frame, swap_file + swap_pos, page_size);
		FREE_SWAP_FRAME_BITMAP(swap_frame);

	} else{
		mappedFile* mapped_file = GET_MAPPED_FILE(a_segment->path);
		printf("Memory pos: %s", mapped_file->pointer);

		memcpy(ptr_to_free_frame, (mapped_file->pointer) + swap_pos, page_size);
		//FREE_SWAP_FRAME_BITMAP(swap_frame); va o no?
	}

	current_page->frame_number = free_frame;
	current_page->presenceBit = 1;
	current_page->useBit = 1;
}


int TOTAL_MEMORY_SIZE(){ // probablemente haya que modificarlo para que en los free me libere memoria
	sem_wait(&bitmap_memory_semaphore);
	int counter = bitarray_get_max_bit(bitmap_memory);
	int free_frames = 0;
	int free_bytes_in_memory = 0;

	for(int i=0;i<counter;i++){
		if(!bitarray_test_bit(bitmap_memory, i)){
			free_frames++;
		}
	}
	sem_post(&bitmap_memory_semaphore);

	free_bytes_in_memory = free_frames * page_size;

	memory_size = free_bytes_in_memory;

	return free_bytes_in_memory;
}

//RENOVANDO EL MERGE_CONSECUTIVE_FREE_BLOCKS
void MERGE_CONSECUTIVES_FREE_BLOCKS_V2(segment* a_segment) {
	t_list* page_frame_table = a_segment->pageFrameTable;
	int pages_amount = page_frame_table->elements_count;
	int current_page_number = 0;
	int move_in_page = 0;
	void* buffer = (void*)malloc(5);
	heapMetadata* base_md = (heapMetadata*)malloc(sizeof(heapMetadata));
	heapMetadata* asking_md = (heapMetadata*)malloc(sizeof(heapMetadata));

	//Estos los pongo por los valores por referencia que usamos
	int md_page_num = 0;
	int md_offset = 0;

	while(current_page_number < pages_amount) {
		if(move_in_page + 5 > page_size) {
			//SPLIT
			pageFrame* current_page = list_get(page_frame_table, current_page_number);
			pageFrame* next_page = list_get(page_frame_table, current_page_number + 1);

			if(!current_page->presenceBit)
				BRING_FROM_SWAP(a_segment, current_page);

			if(!next_page->presenceBit)
				BRING_FROM_SWAP(a_segment, next_page);

			void* pointer_current_frame = GET_FRAME_POINTER(current_page->frame_number);
			void* pointer_next_frame = GET_FRAME_POINTER(next_page->frame_number);

			int bytes_current_frame = (page_size - move_in_page);
			int bytes_next_frame = 5 - (page_size - move_in_page);

			memcpy(buffer, pointer_current_frame + move_in_page, bytes_current_frame);
			memcpy(buffer + bytes_current_frame, pointer_current_frame + move_in_page, bytes_next_frame);

			memcpy(base_md, buffer, 5);

			if(base_md->isFree) {
				//SPLIT - FREE
				/*
				int asking_addr = current_page * page_size + move_in_page;
				asking_md = GET_METADATA_BEHIND_ADDRESS(asking_addr, a_segment, &md_page_num, &md_offset);
				 */
				int asking_addr = current_page_number * page_size + move_in_page + 5 + base_md->size + 5; //Ojo esta cuenta

				if(asking_addr + 5 >= a_segment->base + a_segment->size)
					return;

				bool isSplit; //Porque lo  pide
				asking_md = GET_METADATA_BEHIND_ADDRESS_V2(asking_addr, a_segment, &md_page_num, &md_offset, &isSplit);

				if(asking_md->isFree) {
					//SPLIT - FREE - ASKING FREE => MERGE
					int new_size = asking_md->size + base_md->size + 5;
					memcpy(buffer, &new_size, sizeof(uint32_t));
					memcpy(buffer + bytes_current_frame, &asking_md->isFree, 1);

					memcpy(pointer_current_frame + move_in_page, buffer, bytes_current_frame);
					memcpy(pointer_current_frame + move_in_page, buffer + bytes_current_frame, bytes_next_frame);

					//No toco nada mas porque la base sigue siendo la misma
					//De aca me voy al while de vuelta
				} else {
					//SPLIT - FREE - ASKING USED => CAMINO
					int new_addr = (current_page_number * page_size + move_in_page) + base_md->size + asking_md->size;
					move_in_page = new_addr % page_size;
					current_page_number = new_addr / page_size;
					//De aca me voy al while de vuelta
				}
			} else {
				//SPLIT - USED => CAMINO
				int new_addr = (current_page_number * page_size + move_in_page) + base_md->size;
				move_in_page = new_addr % page_size;
				current_page_number = new_addr / page_size;
				//De aca me voy al while de vuelta
			}
		} else {
			//COMUN
			pageFrame* current_page = list_get(page_frame_table, current_page_number);

			if(!current_page->presenceBit)
				BRING_FROM_SWAP(a_segment, current_page);

			void* pointer_current_frame = GET_FRAME_POINTER(current_page->frame_number);
			void* pointer_base_md = pointer_current_frame + move_in_page;
			base_md = READ_HEAPMETADATA_IN_MEMORY(pointer_base_md);

			if(base_md->isFree) {
				//COMUN - FREE
				int asking_addr = current_page_number * page_size + move_in_page + 5 + base_md->size + 5; //Ojo esta cuenta

				if(asking_addr >= a_segment->base + a_segment->size)
					return;

				bool isSplit; //Porque lo pide
				asking_md = GET_METADATA_BEHIND_ADDRESS_V2(asking_addr, a_segment, &md_page_num, &md_offset, &isSplit);
				if(asking_md->isFree) {
					//COMUN - FREE - ASKING FREE => MERGEO
					int new_size = base_md->size + asking_md->size;

					WRITE_HEAPMETADATA_IN_MEMORY(pointer_base_md, new_size, 1);
					//No toco nada mas porque la base sigue siendo la misma
					//De aca me voy al while de vuelta
				} else {
					//COMUN - FREE - ASKING USED => CAMINO
					int new_addr = (current_page_number * page_size + move_in_page) + 5 + base_md->size + 5 + asking_md->size + 5;
					move_in_page = new_addr % page_size;
					current_page_number = new_addr / page_size;
					//De aca me voy al while de vuelta
				}
			} else {
				//COMUN - USED => CAMINO
				int new_addr = (current_page_number * page_size + move_in_page) + 5 + base_md->size + 5;
				move_in_page = new_addr % page_size;
				current_page_number = new_addr / page_size;
				//De aca me voy al while de vuelta
			}
		}
	}

}

heapMetadata* GET_METADATA_BEHIND_ADDRESS_V2(uint32_t address, segment* a_segment, int *metadata_page_num, int *metadata_offset, bool *isSplit){
	/*
	void* bu = (void*)malloc(5);
	uint32_t dos = 2;
	bool tru = 1;
	memcpy(bu, &dos, 4);
	memcpy(bu + 4, &tru, 1);
	void* p1 = GET_FRAME_POINTER(2);
	void* p2 = GET_FRAME_POINTER(3);
	memcpy(p1 + 1, bu, 3);
	memcpy(p2, bu + 3, 2);
	heapMetadata* md = (heapMetadata*)malloc(sizeof(heapMetadata));
	memcpy(md, bu, 5);
	 */
	t_list* page_frame_table = a_segment->pageFrameTable;
	int pages_amount = page_frame_table->elements_count;
	int current_page_number = 0;
	int move_in_page = 0;
	void* buffer = (void*)malloc(5);
	heapMetadata* base_md = (heapMetadata*)malloc(sizeof(heapMetadata));
	heapMetadata* asking_md = (heapMetadata*)malloc(sizeof(heapMetadata));

	while(current_page_number < pages_amount) {
		if(move_in_page + 5 > page_size) {
			//SPLIT
			pageFrame* current_page = list_get(page_frame_table, current_page_number);
			pageFrame* next_page = list_get(page_frame_table, current_page_number + 1);

			if(!current_page->presenceBit)
				BRING_FROM_SWAP(a_segment, current_page);

			if(!next_page->presenceBit)
				BRING_FROM_SWAP(a_segment, next_page);

			void* pointer_current_frame = GET_FRAME_POINTER(current_page->frame_number);
			void* pointer_next_frame = GET_FRAME_POINTER(next_page->frame_number);

			int bytes_current_frame = (page_size - move_in_page);
			int bytes_next_frame = 5 - (page_size - move_in_page);

			memcpy(buffer, pointer_current_frame + move_in_page, bytes_current_frame);
			memcpy(buffer + bytes_current_frame, pointer_next_frame, bytes_next_frame);

			memcpy(base_md, buffer, 5);

			isSplit = true;
		} else {
			//COMUN
			pageFrame* current_page = list_get(page_frame_table, current_page_number);

			if(!current_page->presenceBit)
				BRING_FROM_SWAP(a_segment, current_page);

			void* pointer_current_frame = GET_FRAME_POINTER(current_page->frame_number);
			void* pointer_base_md = pointer_current_frame + move_in_page;
			base_md = READ_HEAPMETADATA_IN_MEMORY(pointer_base_md);

			isSplit = false;
		}

		int lower_bound = (current_page_number * page_size + move_in_page) + 5;
		int upper_bound = lower_bound + base_md->size;

		if(lower_bound <= address && upper_bound >= address) {
			*metadata_page_num = current_page_number;
			*metadata_offset = move_in_page; //address - lower_bound; ?
			return base_md;
		} else {
			int asking_md_addr = (current_page_number * page_size + move_in_page) + 5 + base_md->size;
			move_in_page = asking_md_addr % page_size;
			current_page_number = asking_md_addr / page_size;
			//De aca me voy al while de vuelta
		}
	}
	return NULL;
}

uint32_t ALLOC(addressSpace* addr_spc, uint32_t bytes) {

	if(THERE_ARE_EXISTING_HEAP_SEGMENTS(addr_spc)){
		// Hay segmentos de heap
		t_list* segment_table = addr_spc->segment_table;

		//CHEQUEO SI HAY ESPACIO EN ALGUNO DE MIS SEGMENTOS
		for(int i = 0; i < segment_table->elements_count; i++){
			segment* segment = list_get(segment_table, i);
			uint32_t offset = ITERATE_SEGMENT_TO_FIND_FREE_SPACE(segment, bytes);

			if(offset >= 0){
				return segment->base + offset; //de esta forma devuelvo la address posta posta
			}
		}

		//COMO NO HABIA ESPACIO, ME FIJO SI PUEDO EXTENDER ALGUN SEGMENTO
		for(int i = 0; i < segment_table->elements_count; i++){
			segment* segment = list_get(segment_table, i);

			if(SEGMENT_CAN_BE_EXTENDED(segment, addr_spc, bytes + 10)){ //DECISION DE DISEÑO: si tengo espacio extiendo a lo cabeza, me la suda la ultima metadata
				uint32_t offset = segment->size + 5;
				WRITE_USED_AND_FREE_METADATA(segment, bytes);
				return segment->base + offset;
			}
		}

	}
	// No hay segmentos de heap perrako => Tengo que crear
	segment* new_segment = (segment*)malloc(sizeof(segment));
	new_segment->isHeap = true;
	new_segment->pageFrameTable = list_create();
	WRITE_USED_AND_FREE_METADATA(new_segment, bytes);

	if(addr_spc->segment_table->elements_count) {
		//Ya hay segmentos en el addr space => Hago first fit
		new_segment->base = FIRST_FIT(addr_spc->segment_table, 0, new_segment->size);
	} else {
		//No hay segmentos at all => Creo el primero
		new_segment->base = 0;
	}

	list_add(addr_spc->segment_table, new_segment);
	return new_segment->base + 5;
}

uint32_t ITERATE_SEGMENT_TO_FIND_FREE_SPACE(segment* segment, int bytes) {
	t_list* page_frame_table = segment->pageFrameTable;
	int current_page_number = 0;
	int move_in_page = 0;
	int new_free_size;
	int new_used_size;
	int new_offset;
	bool write_next_free_metadata_flag = false;
	bool trueStatus = 1;
	bool falseStatus = 0;
	void* buffer = malloc(5);
	heapMetadata* base_md = (heapMetadata*)malloc(sizeof(heapMetadata));

	while(current_page_number < page_frame_table->elements_count){
		if(move_in_page + 5 > page_size){
			//SPLIT
			pageFrame* current_page = list_get(page_frame_table, current_page_number);
			pageFrame* next_page = list_get(page_frame_table, current_page_number + 1);

			if(!current_page->presenceBit){
				BRING_FROM_SWAP(segment, current_page);
			}

			if(!next_page->presenceBit){
				BRING_FROM_SWAP(segment, next_page);
			}

			void* ptr_current_frame = GET_FRAME_POINTER(current_page->frame_number);
			void* ptr_next_frame = GET_FRAME_POINTER(next_page->frame_number);

			int bytes_current_frame = (page_size - move_in_page);
			int bytes_next_frame = 5 - (page_size - move_in_page);

			memcpy(buffer, ptr_current_frame + move_in_page, bytes_current_frame);
			memcpy(buffer + bytes_current_frame, ptr_next_frame + move_in_page, bytes_next_frame);

			memcpy(base_md, buffer, 5);

			if(base_md->isFree && base_md->size >= bytes + 5){
				//SPLIT - TIENE ESPACIO

				//Sobreescribimos la base
				new_used_size = bytes;
				new_free_size = base_md->size - bytes - 5;

				memcpy(buffer, &new_used_size, sizeof(uint32_t));
				memcpy(buffer + sizeof(uint32_t), &falseStatus, 1);

				memcpy(ptr_current_frame + move_in_page, buffer, bytes_current_frame);
				memcpy(ptr_next_frame, buffer + bytes_current_frame, bytes_next_frame);

				//Camino hacia donde escribir la nueva metadata free
				new_offset = current_page_number * page_size + move_in_page + 5 + base_md->size;

				move_in_page = new_offset % page_size;
				current_page_number = new_offset / page_size;

				write_next_free_metadata_flag = true; //activo el flag para que despues me escriba la proxima metadata free
			}

		} else{
			//COMUN

			pageFrame* current_page = list_get(page_frame_table, current_page_number);

			if(current_page->presenceBit)
				BRING_FROM_SWAP(segment, current_page);

			void* ptr_current_frame = GET_FRAME_POINTER(current_page->frame_number);
			void* ptr_base_md = ptr_current_frame + move_in_page;
			base_md = READ_HEAPMETADATA_IN_MEMORY(ptr_base_md);

			if(base_md->isFree && base_md->size >= bytes + 5){
				//COMUN - TIENE ESPACIO

				new_used_size = bytes;
				new_free_size = base_md->size - bytes - 5;

				WRITE_HEAPMETADATA_IN_MEMORY(ptr_base_md, new_used_size, falseStatus);

				//Camino hacia donde escribir la nueva metadata free
				new_offset = current_page_number * page_size + move_in_page + 5 + base_md->size;

				move_in_page = new_offset % page_size;
				current_page_number = new_offset / page_size;

				write_next_free_metadata_flag = true; //activo el flag para que despues me escriba la proxima metadata free
			}
		}

		if(write_next_free_metadata_flag){
			if(move_in_page + 5 > page_size){
				//NEW FREE METADATA SPLITTED

				pageFrame* current_page = list_get(page_frame_table, current_page_number);
				pageFrame* next_page = list_get(page_frame_table, current_page_number + 1);

				if(!current_page->presenceBit){
					BRING_FROM_SWAP(segment, current_page);
				}

				if(!next_page->presenceBit){
					BRING_FROM_SWAP(segment, next_page);
				}

				void* ptr_current_frame = GET_FRAME_POINTER(current_page->frame_number);
				void* ptr_next_frame = GET_FRAME_POINTER(next_page->frame_number);

				int bytes_current_frame = (page_size - move_in_page);
				int bytes_next_frame = 5 - (page_size - move_in_page);

				memcpy(buffer, &new_free_size, sizeof(uint32_t));
				memcpy(buffer + sizeof(uint32_t), &trueStatus, 1);

				memcpy(ptr_current_frame + move_in_page, buffer, bytes_current_frame);
				memcpy(ptr_next_frame, buffer + bytes_current_frame, bytes_next_frame);

				return current_page_number * page_size + move_in_page + 5; //devuelvo el offset dentro del frame adelante de la nueva metadata free

			} else{
				//NEW FREE METADATA COMUN

				pageFrame* current_page = list_get(page_frame_table, current_page_number);

				if(!current_page->presenceBit)
					BRING_FROM_SWAP(segment, current_page);

				void* ptr_current_frame = GET_FRAME_POINTER(current_page->frame_number) + move_in_page;

				WRITE_HEAPMETADATA_IN_MEMORY(ptr_current_frame, new_free_size, trueStatus);

				return current_page_number * page_size + move_in_page + 5; //devuelvo el offset dentro del frame adelante de la nueva metadata free
			}
		}

		//Camino hasta la parada de rutatlantica

		new_offset = (current_page_number * page_size + move_in_page) + 5 + base_md->size;
		move_in_page = new_offset % page_size;
		current_page_number = move_in_page / page_size;
	}
	return -1;
}

void WRITE_USED_AND_FREE_METADATA(segment* a_segment, uint32_t bytes) {
	t_list* page_frame_table = a_segment->pageFrameTable;
	int amount_of_pages_to_add = (bytes + 10) / page_size;
	if((amount_of_pages_to_add * page_size) < bytes + 10)
		amount_of_pages_to_add += 1;
	int base_page = page_frame_table->elements_count;
	int new_free_size;
	int move_in_page = 0;
	bool trueStatus = 1;
	bool falseStatus = 0;
	void* buffer = malloc(5);
	heapMetadata* base_md = (heapMetadata*)malloc(sizeof(heapMetadata));

	if(5 > page_size){
		//SPLIT

		int current_frame_number = CLOCK();
		pageFrame* current_new_page = (pageFrame*)malloc(sizeof(pageFrame));
		current_new_page->modifiedBit = 0;
		current_new_page->presenceBit = 1;
		current_new_page->frame_number = current_frame_number;
		list_add(page_frame_table, current_new_page);

		int next_frame_number = CLOCK();
		pageFrame* next_new_page = (pageFrame*)malloc(sizeof(pageFrame));
		next_new_page->modifiedBit = 0;
		next_new_page->presenceBit = 1;
		next_new_page->frame_number = next_frame_number;
		list_add(page_frame_table, next_new_page);

		dictionary_put(clock_table, string_itoa(current_frame_number), current_new_page);
		dictionary_put(clock_table, string_itoa(next_frame_number), next_new_page);

		void* ptr_current_frame = GET_FRAME_POINTER(current_new_page->frame_number);
		void* ptr_next_frame = GET_FRAME_POINTER(next_new_page->frame_number);

		int bytes_current_frame = page_size;
		int bytes_next_frame = 5 - bytes_current_frame;

		memcpy(buffer, &bytes, sizeof(uint32_t));
		memcpy(buffer + sizeof(uint32_t), &falseStatus, 1);

		memcpy(ptr_current_frame, buffer, bytes_current_frame);
		memcpy(ptr_next_frame, buffer + bytes_current_frame, bytes_next_frame);

		base_page += 2;


	} else{
		//COMUN

		int current_frame_number = CLOCK();
		pageFrame* current_new_page = (pageFrame*)malloc(sizeof(pageFrame));
		current_new_page->modifiedBit = 0;
		current_new_page->presenceBit = 1;
		current_new_page->frame_number = current_frame_number;
		list_add(page_frame_table, current_new_page);

		dictionary_put(clock_table, string_itoa(current_frame_number), current_new_page);

		void* ptr_current_frame = GET_FRAME_POINTER(current_new_page->frame_number);

		WRITE_HEAPMETADATA_IN_MEMORY(ptr_current_frame, bytes, falseStatus);

		base_page += 1;
	}

	for(int i = base_page; i < amount_of_pages_to_add; i++){
		int frame_number = CLOCK();
		pageFrame* current_new_page = (pageFrame*)malloc(sizeof(pageFrame));
		current_new_page->modifiedBit = 0;
		current_new_page->presenceBit = 1;
		current_new_page->frame_number = frame_number;
		list_add(page_frame_table, current_new_page);
		dictionary_put(clock_table, string_itoa(frame_number), current_new_page);
	}

	move_in_page = (move_in_page + 5 + bytes) % page_size;
	new_free_size = (page_frame_table->elements_count * page_size) - 5 - bytes - 5;

	if(move_in_page + 5 > page_size){
		//SIGUIENTE METADATA SPLIT

		pageFrame* current_page = list_get(page_frame_table, page_frame_table->elements_count - 2);
		pageFrame* next_page = list_get(page_frame_table, page_frame_table->elements_count - 1);

		void* ptr_current_frame = GET_FRAME_POINTER(current_page->frame_number);
		void* ptr_next_frame = GET_FRAME_POINTER(next_page->frame_number);

		int bytes_current_frame = (page_size - move_in_page);
		int bytes_next_frame = 5 - bytes_current_frame;

		memcpy(buffer, &new_free_size, sizeof(uint32_t));
		memcpy(buffer + sizeof(uint32_t), &trueStatus, 1);

		memcpy(ptr_current_frame + move_in_page, buffer, bytes_current_frame);
		memcpy(ptr_next_frame, buffer + bytes_current_frame, bytes_next_frame);

	} else{
		//SIGUIENTE METADATA COMUN

		pageFrame* current_page = list_get(page_frame_table, page_frame_table->elements_count - 1);

		void* ptr_current_frame = GET_FRAME_POINTER(current_page->frame_number);

		WRITE_HEAPMETADATA_IN_MEMORY(ptr_current_frame, new_free_size, trueStatus);
	}

	a_segment->size = a_segment->pageFrameTable->elements_count * page_size;
}

int WRITE_N_BYTES_DATA_TO_MUSE_V2(uint32_t dst, addressSpace* address_space, size_t bytes_a_copiar, void* data){
	//TODO
	segment* a_segment = GET_SEGMENT_FROM_ADDRESS(dst, address_space);
	t_list* page_frame_table = a_segment->pageFrameTable;
	uint32_t md_page_num;
	uint32_t md_move_in_page;
	bool isSplit;
	heapMetadata* md_base = GET_METADATA_BEHIND_ADDRESS_V2(dst, a_segment, &md_page_num, &md_move_in_page, &isSplit);
	uint32_t offset_to_md = OFFSET_ADDR_TO_METADATA(a_segment->base, md_page_num, md_move_in_page, dst);
	uint32_t data_move_in_page = TRANSLATE_DL_TO_OFFSET_IN_THE_PAGE(dst);
	uint32_t writen_bytes = 0;
	/*
	int page_move_counter = 0;
	int page_number = GET_PAGE_NUMBER_FROM_ADDRESS(dst, a_segment);
	int offset = 0;
	int bytes = bytes_a_copiar;
	int metadata_page_num;
	int initial_offset;
	int metadata_offset;
	int pages_in_the_middle;
	bool first_iteration = true;
	 */
	if(a_segment != NULL && (md_base->size - offset_to_md) >= bytes_a_copiar) {
		int data_page_num = GET_PAGE_NUMBER_FROM_ADDRESS(dst, a_segment);
		int pages_to_be_writen;
		if((bytes_a_copiar + data_move_in_page)% page_size) {
			pages_to_be_writen = bytes_a_copiar / page_size + 1;
		} else {
			pages_to_be_writen = bytes_a_copiar / page_size;
		}
		int limit_page = data_page_num + pages_to_be_writen;
		//Paso a escribir los bytes que sean necesarios
		for(int i = data_page_num; i < limit_page; i++) {
			pageFrame* current_page = list_get(page_frame_table, i);

			if(!current_page->presenceBit)
				BRING_FROM_SWAP(a_segment, current_page);

			current_page->useBit = 1;
			current_page->modifiedBit = 1;

			void* pointer_current_frame = GET_FRAME_POINTER(current_page->frame_number);

			int bytes_current_frame;
			if((page_size - data_move_in_page) > (bytes_a_copiar - writen_bytes)) {
				bytes_current_frame = (bytes_a_copiar - writen_bytes);
			} else {
				bytes_current_frame = (page_size - data_move_in_page);
			}

			memcpy(pointer_current_frame + data_move_in_page, data + writen_bytes, bytes_current_frame);

			writen_bytes += bytes_current_frame;
			data_move_in_page = 0;
		}
		return 0;
	} else {
		//Si no encuentra el segmento, es que la direccion es re falopa => SEG FAULT
		//Si no tengo espacio => SEG FAULT
		return -1;
	}
}

void* GET_N_BYTES_DATA_FROM_MUSE_V2(addressSpace* address_space, uint32_t src, size_t bytes_a_copiar) {
	segment* a_segment = GET_SEGMENT_FROM_ADDRESS(src, address_space);
	t_list* page_frame_table = a_segment->pageFrameTable;
	int page_move_counter = 0;
	int page_number;
	int metadata_page_number = 0;;
	int metadata_offset = 0;
	int offset = 0;
	int move_in_page = 0;
	int bytes = bytes_a_copiar;
	void* data = malloc(bytes_a_copiar);
	heapMetadata* metadata;
	pageFrame* current_page;
	bool isSplit; //Porque lo pide

	if(a_segment != NULL){
		page_number = GET_PAGE_NUMBER_FROM_ADDRESS(src, a_segment);
		move_in_page = TRANSLATE_DL_TO_OFFSET_IN_THE_PAGE(src);
		metadata = GET_METADATA_BEHIND_ADDRESS_V2(src, a_segment, &metadata_page_number, &metadata_offset, &isSplit);
		current_page = list_get(page_frame_table, page_number);

		void* ptr_to_frame = GET_FRAME_POINTER(current_page->frame_number);
		void* ptr_to_data = ptr_to_frame + move_in_page; // va a la direcc donde tengo que empezar a copiar

		while(page_frame_table->elements_count > page_number && bytes_a_copiar > 0){

			current_page = list_get(page_frame_table, page_number);

			if(!current_page->presenceBit){
				BRING_FROM_SWAP(a_segment, current_page);
			}

			ptr_to_frame = GET_FRAME_POINTER(current_page->frame_number);

			if(move_in_page + bytes_a_copiar < page_size){ // si al sumar esos bytes sigo en mi pagina
				memcpy(data+offset, ptr_to_data, bytes_a_copiar);
				bytes_a_copiar = 0;
				current_page->useBit = 1;

			} else{ // si al sumar esos bytes me pase... voy a la pagina siguiente y leo lo que me queda
				int bytes_en_frame_anterior = page_size - move_in_page;
				int bytes_restantes_en_frame_siguiente = (move_in_page + bytes_a_copiar) - bytes_en_frame_anterior;
				memcpy(data+offset, ptr_to_data, bytes_en_frame_anterior);
				current_page->useBit = 1;

				bytes_a_copiar = bytes_restantes_en_frame_siguiente;
				offset = offset + bytes_en_frame_anterior;
				move_in_page = 0;

				page_number++;
			}
		}
	} else{
		data = NULL;
	}

	return data;
}

uint32_t OFFSET_ADDR_TO_METADATA(uint32_t segment_base, uint32_t md_page_num, uint32_t md_move_in_page, uint32_t addr) {
	//Te da la distancia del addr al final de la metadata
	uint32_t md_virtual_addr = segment_base + (md_page_num * page_size) + md_move_in_page + 5;
	return addr - md_virtual_addr;
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

