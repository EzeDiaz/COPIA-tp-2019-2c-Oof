#ifndef SERIALIZACIONMUSE_H_
#define SERIALIZACIONMUSE_H_

void* crear_paquete_init(int codigo_de_operacion, char* IP_id);
void* crear_paquete_alloc(int codigo_de_operacion, uint32_t bytes_a_reservar);
void* crear_paquete_free(int codigo_de_operacion, uint32_t dir);

#endif /* SERIALIZACIONMUSE_H_ */
