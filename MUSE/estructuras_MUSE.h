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
	int frame_number; //Si esta en memoria es el frame, si esta en swap la posicion
} pageFrame;

typedef struct{	//Pesa SIEMPRE 5 bytes
	uint32_t size;
	bool isFree;
} heapMetadata;

typedef struct{
	t_list* pageFrameTable;
	int base; //Base logica
	//Tamanio es un atributo o es "calculable"
} segment;

typedef struct{
	char* owner;
	t_list* segment_table; //Como limito 2 a la 32 direcciones?
} addressSpace;

typedef struct{
	char* clientProcessId;
	int clientSocket;
} client;

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
void* SEGMENT_IS_BIG_ENOUGH(segment* a_segment, uint32_t intended_size);
t_list* GET_CLIENT_SEGMENTS(int a_client_socket);
bool CLIENT_HAS_SEGMENT(int client);
void CREATE_TABLES();
void DESTROY_TABLES();



#endif /* ESTRUCTURAS_MUSE_H_ */
