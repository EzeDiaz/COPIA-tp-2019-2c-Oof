//Bibliotecas propias
#include "libMUSE.h"

void* crear_paquete_init(int codigo_de_operacion, int id) {
	//Todos los pesos de los elementos
	int peso_total=0;
	int peso_codigo_de_operacion=sizeof(int);
	int peso_del_siguiente;

	//Suma del peso total: sizeof(tamanio del elem)+sizeof(elem) --> Sumado por cada elem.
	//El peso del codigo_de_operacion no tiene sentido ponerlo, ya se que es un int.
	peso_total+=peso_codigo_de_operacion;

	peso_del_siguiente=sizeof(id);
	peso_total+=sizeof(peso_del_siguiente);
	peso_total+=peso_del_siguiente;

	void* paquete=(void*)malloc(peso_total);

	int offset=0;
	peso_del_siguiente=0;

	memcpy(paquete+offset, &codigo_de_operacion, peso_codigo_de_operacion);
	offset+=peso_codigo_de_operacion;

	peso_del_siguiente=sizeof(id);
	memcpy(paquete+offset, &peso_del_siguiente, sizeof(peso_del_siguiente));
	offset+=sizeof(peso_del_siguiente);
	memcpy(paquete+offset, &id, peso_del_siguiente);

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
