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

typedef struct{		//administracion de paginas
	bool presenceBit;	//bit de presencia
	bool modifiedBit;	//bit de modificado
	void* pagePointer;		//puntero al inicio de pagina en memoria principal
} pageFrame;	//hay que ahorrar por la macrisis, asi que pongo bool y no algun tipo numerico

typedef struct{	//estructura propuesta por la catedra para la memoria, pesa SIEMPRE 5B
	uint32_t size;
	bool isFree;
} heapMetadata;

typedef struct{
	char* owner; //para poder identificar de que proceso es el segmento
	t_list* pageFrameTable;
	void* segmentPointer; //puntero al inicio del segmento (datos). Redundante con el pagePointer?
} segment;

typedef struct{
	char* clientProcessId;
	int clientSocket;
} client;

t_list* GET_CLIENT_SEGMENTS(int a_client_socket);
client* FIND_CLIENT_BY_SOCKET(int a_client_socket);
bool CLIENT_HAS_SEGMENT(int client);
int ADD_CLIENT_TO_LIST(char* client_ID, int client_socket);
void INITIALIZE_SEMAPHORES();
void DESTROY_SEMAPHORES();
void CHECK_LOGGER();
void CHECK_CONFIG();
void CHECK_FIELDS();
void GET_CONFIG_VALUES();
void CHECK_MEMORY();
void CREATE_TABLES();
void WRITE_HEAPMETADATA_IN_MEMORY(void* pointer, uint32_t size, bool status);
void READ_HEAPMETADATA_IN_MEMORY(void* pointer);
void SUBSTRACT_MEMORY_LEFT(int size);
segment* CREATE_NEW_EMPTY_SEGMENT(char* name);
void WRITE_ADDRESSES_IN_SEGMENT(void* pointer, uint32_t size, segment* segment);
void CREATE_NEW_SEGMENT_IN_MEMORY(void* pointer, void* info, uint32_t size);

#endif /* ESTRUCTURAS_MUSE_H_ */
