#ifndef SERIALIZACIONMUSE_H_
#define SERIALIZACIONMUSE_H_

void* crear_paquete_init(int codigo_de_operacion, char* IP_id);
void* crear_paquete_alloc(int codigo_de_operacion, uint32_t bytes_a_reservar);
void* crear_paquete_free(int codigo_de_operacion, uint32_t dir);
void* crear_paquete_get(int codigo_de_operacion, void* dst, uint32_t src, size_t n);
void* crear_paquete_cpy(int codigo_de_operacion, uint32_t dst, void* src, size_t n);
void* crear_paquete_map(int codigo_de_operacion, char* path, size_t length, int flags);
void* crear_paquete_sync(int codigo_de_operacion, uint32_t addr, size_t len);
void* crear_paquete_unmap(int codigo_de_operacion, uint32_t dir);

#endif /* SERIALIZACIONMUSE_H_ */
