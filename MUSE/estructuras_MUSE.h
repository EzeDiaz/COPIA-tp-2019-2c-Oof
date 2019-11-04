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
	bool modifiedBit;	//bit de modificado
	int frame_number; //Si esta en memoria es el frame, si esta en swap la posicion (presence)
} pageFrame;

typedef struct{	//Pesa SIEMPRE 5 bytes
	uint32_t size;
	bool isFree;
} heapMetadata;

typedef struct{
	bool isHeap;
	t_list* pageFrameTable;
	uint32_t base; //Base logica
	uint32_t size;
} segment;

typedef struct{
	char* owner;
	t_list* segment_table; //Como limito 2 a la 32 direcciones?
} addressSpace;

typedef struct{
	char* clientProcessId;
	int clientSocket;
} client;

segment* GET_SEGMENT_FROM_BASE(uint32_t base, addressSpace* address_space);
int GET_OFFSET_FROM_POINTER(void* pointer);
int GET_FRAME_NUMBER_FROM_POINTER(void* pointer);
void* GET_LAST_METADATA(segment* a_segment);
bool SEGMENT_CAN_BE_EXTENDED(segment* a_segment, addressSpace an_address_space, uint32_t intended_size);
void* SEGMENT_IS_BIG_ENOUGH(segment* a_segment, uint32_t intended_size);
t_list* GET_HEAP_SEGMENTS(addressSpace* address_space);
bool THERE_ARE_EXISTING_HEAP_SEGMENTS(addressSpace* an_address_space);
addressSpace* GET_ADDRESS_SPACE(int client_socket);
void FREE_FRAME(int frame_number);
void* GET_FRAME_POINTER(int frame_number);
int ASSIGN_FIRST_FREE_FRAME();
int CREATE_ADDRESS_SPACE(char* IP_ID);
void SET_BITMAP();
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

//Potentially deprecated
segment* CREATE_NEW_EMPTY_SEGMENT(char* name);
void WRITE_ADDRESSES_IN_SEGMENT(void* pointer, uint32_t size, segment* segment);
void CREATE_NEW_SEGMENT_IN_MEMORY(void* pointer, void* info, uint32_t size);
void CREATE_TABLES();
void DESTROY_TABLES();



#endif /* ESTRUCTURAS_MUSE_H_ */
