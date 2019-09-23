#ifndef SERIALIZACIONMUSE_H_
#define SERIALIZACIONMUSE_H_

void* crear_paquete_init(int codigo_de_operacion, int id);
void* crear_paquete_alloc(int codigo_de_operacion, uint32_t bytes_a_reservar);

#endif /* SERIALIZACIONMUSE_H_ */
