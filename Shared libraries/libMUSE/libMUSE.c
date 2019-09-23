//Bibliotecas propias
#include "libMUSE.h"
#include "serializacionMUSE.h"
#include "get_local_IP.h"

//Standards
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

//Para poder pedir el pid
#include <unistd.h>

//Para usar sockets
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>

int muse_init(int id, char* ip, int puerto){
	struct hostent *server_host;
	struct sockaddr_in server_address;

	/* Get server host from server name. */
	server_host = gethostbyname(ip);

	//Inicializa IPv4 server address con server host.
	memset(&server_address, 0, sizeof server_address);
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(puerto);
	memcpy(&server_address.sin_addr.s_addr, server_host->h_addr, server_host->h_length);

	//Crear socket TCP
	socket_MUSE = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_MUSE == -1) {
		printf("libMUSE no pudo crear el socket\n");
		return -1; //Manejar este codigo de error para poder saber que fallo
	}


	//Conectarse a MUSE
	if (connect(socket_MUSE, (struct sockaddr *)&server_address, sizeof server_address) == -1) {
		printf("La conexion con MUSE no pudo establecerse");
		return -1; //Manejar este codigo de error para poder saber que fallo
	}

	//Concatenar id con IP local --> IP_local=get_local_IP()

	void* paquete_init = crear_paquete_init(00,id);
	send(socket_MUSE, paquete_init,sizeof(paquete_init),0);
	free(paquete_init);
	//recv

	return 0;

}

void muse_close(){
	close(socket_MUSE);
	//Tendremos que cerrar mas cosas?
	//Veremos en la medida que levantemos mas estructuras administrativas
}

uint32_t muse_alloc(uint32_t tam){
    return (uint32_t) malloc(tam);
}

void muse_free(uint32_t dir) {
    free((void*) dir);
}

int muse_get(void* dst, uint32_t src, size_t n){
    memcpy(dst, (void*) src, n);
    return 0;
}

int muse_cpy(uint32_t dst, void* src, int n){
    memcpy((void*) dst, src, n);
    return 0;
}

/////////////////////////////////////////////////////////////////////////////
uint32_t muse_map(char *path, size_t length, int flags){
    return 0;
}

int muse_sync(uint32_t addr, size_t len){
    return 0;
}

int muse_unmap(uint32_t dir){
    return 0;
}
