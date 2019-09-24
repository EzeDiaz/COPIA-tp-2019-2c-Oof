//Bibliotecas propias
#include "libMUSE.h"

//Standards
#include <stdlib.h>
#include <string.h>

void* crear_paquete_init(int codigo_de_operacion, char* IP_id) {
	//Todos los pesos de los elementos
	int peso_total=0;
	int peso_codigo_de_operacion=sizeof(int);
	int peso_del_siguiente;

	//Suma del peso total: sizeof(tamanio del elem)+sizeof(elem) --> Sumado por cada elem.
	//El peso del codigo_de_operacion no tiene sentido ponerlo, ya se que es un int.
	peso_total+=peso_codigo_de_operacion;

	peso_del_siguiente=strlen(IP_id)+1;
	peso_total+=sizeof(peso_del_siguiente);
	peso_total+=peso_del_siguiente;

	void* paquete=(void*)malloc(peso_total);

	int offset=0;
	peso_del_siguiente=0;

	memcpy(paquete+offset, &codigo_de_operacion, peso_codigo_de_operacion);
	offset+=peso_codigo_de_operacion;

	peso_del_siguiente=strlen(IP_id)+1;
	memcpy(paquete+offset, &peso_del_siguiente, sizeof(peso_del_siguiente));
	offset+=sizeof(peso_del_siguiente);
	memcpy(paquete+offset, strlen(IP_id)+1, peso_del_siguiente);

	return paquete;
}

void* crear_paquete_alloc(int codigo_de_operacion, uint32_t bytes_a_reservar) {
	//Todos los pesos de los elementos
	int peso_total=0;
	int peso_codigo_de_operacion=sizeof(int);
	int peso_del_siguiente;

	//Suma del peso total: sizeof(tamanio del elem)+sizeof(elem) --> Sumado por cada elem.
	//El peso del codigo_de_operacion no tiene sentido ponerlo, ya se que es un int.
	peso_total+=peso_codigo_de_operacion;

	peso_del_siguiente=sizeof(bytes_a_reservar);
	peso_total+=sizeof(peso_del_siguiente);
	peso_total+=peso_del_siguiente;

	void* paquete=(void*)malloc(peso_total);

	int offset=0;
	peso_del_siguiente=0;

	memcpy(paquete+offset, &codigo_de_operacion, peso_codigo_de_operacion);
	offset+=peso_codigo_de_operacion;

	peso_del_siguiente=sizeof(bytes_a_reservar);
	memcpy(paquete+offset, &peso_del_siguiente, sizeof(peso_del_siguiente));
	offset+=sizeof(peso_del_siguiente);
	memcpy(paquete+offset, &bytes_a_reservar, peso_del_siguiente);

	return paquete;
}

void* crear_paquete_free(int codigo_de_operacion, uint32_t dir) {
	//Todos los pesos de los elementos
	int peso_total=0;
	int peso_codigo_de_operacion=sizeof(int);
	int peso_del_siguiente;

	//Suma del peso total: sizeof(tamanio del elem)+sizeof(elem) --> Sumado por cada elem.
	//El peso del codigo_de_operacion no tiene sentido ponerlo, ya se que es un int.
	peso_total+=peso_codigo_de_operacion;

	peso_del_siguiente=sizeof(dir);
	peso_total+=sizeof(peso_del_siguiente);
	peso_total+=peso_del_siguiente;

	void* paquete=(void*)malloc(peso_total);

	int offset=0;
	peso_del_siguiente=0;

	memcpy(paquete+offset, &codigo_de_operacion, peso_codigo_de_operacion);
	offset+=peso_codigo_de_operacion;

	peso_del_siguiente=sizeof(dir);
	memcpy(paquete+offset, &peso_del_siguiente, sizeof(peso_del_siguiente));
	offset+=sizeof(peso_del_siguiente);
	memcpy(paquete+offset, &dir, peso_del_siguiente);

	return paquete;
}
