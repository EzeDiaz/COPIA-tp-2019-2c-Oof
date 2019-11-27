/*
 * estructuras_MUSE.h
 *
 *  Created on: 65 oct. 2019
 *      Author: utnso
 */

#ifndef ESTRUCTURAS_MUSE_H_
#define ESTRUCTURAS_MUSE_H_

#include <inttypes.h>
#include <commons/config.h>
#include <commons/log.h>
#include <commons/collections/list.h>

typedef struct{
	bool presenceBit;	//bit de presencia
	bool useBit; 		//bit de uso
	bool modifiedBit;	//bit de modificado
	int frame_number; //Si esta en memoria es el frame, si esta en swap la posicion (presence)
} pageFrame;

typedef struct{	//Pesa SIEMPRE 5 bytes
	uint32_t size;
	bool isFree;
} heapMetadata;

typedef struct{
	bool isHeap; //Si esta en false quiere decir que es un segmento mmap
	char* path; //Path del archivo mappeado. Solo se llena si es segmento de map
	t_list* pageFrameTable;
	uint32_t base; //Base logica
	uint32_t size;
} segment;

typedef struct{
	char* owner;
	t_list* segment_table;
} addressSpace;

typedef struct{
	char* clientProcessId;
	int clientSocket;
	int total_memory_requested;
	int total_memory_freed;
	uint32_t last_requested_segment_base;
} client;

typedef struct{
	char* path; //Path del archivo
	size_t length;
	int file_desc;
	char* owner; //Quien lo tiene abierto
	char* pointer; //Puntero al archivo mapeado
	int flag; //MAP_SHARED o MAP_PRIVATE
	int references;
} mappedFile; //1 por cada archivo. Los privados pueden solo ser ref 1 vez? VER MAP_PRIVATE y sus implicancias

int TRANSLATE_DL_TO_DF(uint32_t dl);
void LOG_METRICS(int socket);
void LOG_SYSTEM_METRICS();
void LOG_PROGRAM_METRICS(int a_client_socket);
void LOG_SOCKET_METRICS(int socket);
int PORCENTAJE_ASIGNACION_MEM(int socket);
void WRITE_N_BYTES_DATA_TO_MUSE(uint32_t dst, addressSpace* address_space, size_t bytes_a_copiar, void* data);
void* GET_N_BYTES_DATA_FROM_MUSE(addressSpace* address_space, uint32_t src, size_t bytes_a_copiar);
void DESTROY_SEGMENT(segment* a_segment);
void DESTROY_ADDRESS_SPACE(addressSpace* an_address_space);
void DESTROY_PAGE(pageFrame* a_page);
void CLIENT_LEFT_THE_SYSTEM(int client);
void FREE_SWAP_FRAME_BITMAP(int frame_number);
void SWAP_INIT();
int GET_FREE_SWAP_FRAME();
void SET_BITMAP_SWAP();
int CLOCK();
int GET_SEGMENT_INDEX(t_list* segment_table, uint32_t a_base);
void DESTROY_MAPPED_FILE(mappedFile* mapped_file);
int GET_MAPPED_FILE_INDEX(char* path);
mappedFile* GET_MAPPED_FILE(char* path);
bool FILE_ALREADY_MAPPED(char* path);
uint32_t FIRST_FIT(t_list* segment_table, uint32_t base, uint32_t size);
segment* GET_SEGMENT_FROM_BASE(uint32_t base, addressSpace* address_space);
int GET_OFFSET_FROM_POINTER(void* pointer);
int GET_FRAME_NUMBER_FROM_POINTER(void* pointer);
void* GET_LAST_METADATA(segment* a_segment);
bool SEGMENT_CAN_BE_EXTENDED(segment* a_segment, addressSpace* an_address_space, uint32_t intended_size);
void* SEGMENT_IS_BIG_ENOUGH(segment* a_segment, uint32_t intended_size);
t_list* GET_HEAP_SEGMENTS(addressSpace* address_space);
bool THERE_ARE_EXISTING_HEAP_SEGMENTS(addressSpace* an_address_space);
addressSpace* GET_ADDRESS_SPACE(int client_socket);
void FREE_MEMORY_FRAME_BITMAP(int frame_number);
void* GET_FRAME_POINTER(int frame_number);
int CREATE_ADDRESS_SPACE(char* IP_ID);
void SET_BITMAP_MEMORY();
client* FIND_CLIENT_BY_SOCKET(int a_client_socket);
int ADD_CLIENT_TO_LIST(char* client_ID, int client_socket);
void INITIALIZE_SEMAPHORES();
void DESTROY_SEMAPHORES();
void CHECK_LOGGER();
void CHECK_CONFIG();
void CHECK_FIELDS();
void GET_CONFIG_VALUES();
void CHECK_MEMORY();
void WRITE_HEAPMETADATA_IN_MEMORY(void* pointer, uint32_t size, bool status);
heapMetadata* READ_HEAPMETADATA_IN_MEMORY(void* pointer);
void SUBSTRACT_MEMORY_LEFT(int size);
segment* GET_SEGMENT__ADDRESS(uint32_t, addressSpace*);
int GET_FRAME_FROM_ADDRESS(uint32_t address, segment* a_segment);
void MERGE_CONSECUTIVES_FREE_BLOCKS(segment* a_segment);
int FREE_USED_FRAME(uint32_t address, addressSpace* address_space);
heapMetadata* GET_METADATA_BEHIND_ADDRESS(uint32_t address, t_list* page_frame_table);

//Potentially deprecated
segment* CREATE_NEW_EMPTY_SEGMENT(char* name);
void WRITE_ADDRESSES_IN_SEGMENT(void* pointer, uint32_t size, segment* segment);
void CREATE_NEW_SEGMENT_IN_MEMORY(void* pointer, void* info, uint32_t size);
void CREATE_TABLES();
void DESTROY_TABLES();



#endif /* ESTRUCTURAS_MUSE_H_ */
