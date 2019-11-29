/*
 Notas sobre como tengo configuradas las commons:
 	 .Segui los pasos de aca: https://www.youtube.com/watch?v=Aw9kXFqWu_I&t=394s

 	 .Tengo un proyecto de Eclipse que se llama commons

 	 .Mi Eclipse abre en el workspace default: /home/utnso/workspace/

 	 .Las commons las copie en: /home/utnso/workspace/commons/

 	 .Si vemos que algo no anda bien preguntemos la mejor manera de configurarlo
 	  porque el cuatri pasado con Jorge nos pasaba que pusheabamos configuraciones
 	  locales y haciamos que el otro no pudiera correr/compilar

 	  .Si vemos que igual es un lio, agarramos y no subimos los archivos de configuracion
 	   como el .cproject, el subdir.mk y esas cosas
 */

//Bibliotecas propias
#include "libMUSE.h"
#include "serializacionMUSE.h"
#include "get_local_IP.h"

//Commons
#include <commons/string.h>

//Standards
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <signal.h>

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
	char* IP_como_char=get_local_IP();
	char* guion = "-";
	char* id_como_char=string_itoa(id);
	char* IP_id = (char*)malloc(strlen(IP_como_char)+strlen(guion)+strlen(id_como_char)+1);

	strcpy(IP_id, IP_como_char);
	strcat(IP_id, guion);
	strcat(IP_id, id_como_char);

	void* paquete_init = crear_paquete_init(100,IP_id);
	int tamanio_paquete;
	memcpy(&tamanio_paquete, paquete_init, 4);
	send(socket_MUSE, paquete_init,tamanio_paquete+4,0); //El +4 es para contemplar el peso del atributo peso
	free(paquete_init);

	//free(IP_como_char);
	free(id_como_char);
	free(IP_id);

	void* var_recepcion;
	int tamanio_recepcion = sizeof(int);
	var_recepcion = malloc(tamanio_recepcion);
	recv(socket_MUSE, var_recepcion, tamanio_recepcion, 0);
	int resultado;
	memcpy(&resultado, var_recepcion, tamanio_recepcion);
	free(var_recepcion);

	return resultado;

}

void muse_close(){
	close(socket_MUSE);
	//Le aviso a MUSE que me di de baja?
	//Tendremos que cerrar mas cosas?
	//Veremos en la medida que levantemos mas estructuras administrativas
}

uint32_t muse_alloc(uint32_t tam){
	void* paquete_alloc = crear_paquete_alloc(102,tam);
	send(socket_MUSE, paquete_alloc,sizeof(paquete_alloc),0);
	free(paquete_alloc);

	void* var_recepcion;
	int tamanio_recepcion = sizeof(uint32_t);
	var_recepcion = malloc(tamanio_recepcion);
	recv(socket_MUSE, var_recepcion, tamanio_recepcion, 0);
	uint32_t resultado;
	memcpy(&resultado, var_recepcion, tamanio_recepcion);
	free(var_recepcion);

	return resultado;
}

void muse_free(uint32_t dir) {
	//Siendo void quiere decir que nada puede malir sal? --> No hacer recv (?)
	void* paquete_free = crear_paquete_free(103,dir);
	send(socket_MUSE, paquete_free,sizeof(paquete_free),0);

	void* var_recepcion;
	int tamanio_recepcion = sizeof(uint32_t);
	var_recepcion = malloc(tamanio_recepcion);
	recv(socket_MUSE, var_recepcion, tamanio_recepcion, 0);
	uint32_t resultado;
	memcpy(&resultado, var_recepcion, tamanio_recepcion);
	free(var_recepcion);

	if(resultado == -1)
		raise(SIGSEGV);

	free(paquete_free);
}

int muse_get(void* dst, uint32_t src, size_t n){
	void* paquete_get = crear_paquete_get(104, dst, src, n);
	send(socket_MUSE, paquete_get,sizeof(paquete_get),0);
	free(paquete_get);

	void* var_recepcion;
	int tamanio_recepcion = n; //MUSE me manda la cantidad de bytes que le pedi
	var_recepcion = malloc(tamanio_recepcion);
	recv(socket_MUSE, var_recepcion, tamanio_recepcion, 0);
	int resultado;

	//Agarro el primer caracter de lo que recibi para ver que onda
	char* primer_caracter_var_recepcion = (char*) malloc(1);
	memcpy(primer_caracter_var_recepcion, var_recepcion, 1);

	if(primer_caracter_var_recepcion != '\0') { //Puedo hacer eso con var_recepcion? //Sera un problema comparar por \0?
		//Tutti OK
		memcpy(dst, var_recepcion, tamanio_recepcion);
		resultado = 0;
	} else {
		//Algo fallo
		resultado = -1;
		raise(SIGSEGV); //Aca es necesario o simplemente devuelvo -1?
	}
	free(var_recepcion);

	//En que caso devuelvo -1?
	//Tendria que, entonces, recibir los bytes y ademas un int con exito/fracaso?

	return resultado;
}

int muse_cpy(uint32_t dst, void* src, int n){
	void* paquete_cpy = crear_paquete_cpy(105, dst, src, n);
	send(socket_MUSE, paquete_cpy,sizeof(paquete_cpy),0);
	free(paquete_cpy);

	void* var_recepcion;
	int tamanio_recepcion = sizeof(int);
	var_recepcion = malloc(tamanio_recepcion);
	recv(socket_MUSE, var_recepcion, tamanio_recepcion, 0);
	int resultado;
	memcpy(&resultado, var_recepcion, tamanio_recepcion);
	free(var_recepcion);

	if(resultado < 0)
		raise(SIGSEGV);

	return resultado;
}

/////////////////////////////////////////////////////////////////////////////
uint32_t muse_map(char *path, size_t length, int flags){
	void* paquete_map = crear_paquete_map(106, path, length, flags);
	send(socket_MUSE, paquete_map,sizeof(paquete_map),0);
	free(paquete_map);

	void* var_recepcion;
	int tamanio_recepcion = sizeof(uint32_t);
	var_recepcion = malloc(tamanio_recepcion);
	recv(socket_MUSE, var_recepcion, tamanio_recepcion, 0);
	uint32_t resultado;
	memcpy(&resultado, var_recepcion, tamanio_recepcion);
	free(var_recepcion);

	return resultado;
}

int muse_sync(uint32_t addr, size_t len){
	void* paquete_sync = crear_paquete_sync(107, addr, len);
	send(socket_MUSE, paquete_sync,sizeof(paquete_sync),0);
	free(paquete_sync);

	void* var_recepcion;
	int tamanio_recepcion = sizeof(int);
	var_recepcion = malloc(tamanio_recepcion);
	recv(socket_MUSE, var_recepcion, tamanio_recepcion, 0);
	int resultado;
	memcpy(&resultado, var_recepcion, tamanio_recepcion);

	if(resultado < 0)
		raise(SIGSEGV);

	free(var_recepcion);

	return resultado;
}

int muse_unmap(uint32_t dir){
	void* paquete_unmap = crear_paquete_unmap(108, dir);
	send(socket_MUSE, paquete_unmap,sizeof(paquete_unmap),0);
	free(paquete_unmap);

	void* var_recepcion;
	int tamanio_recepcion = sizeof(int);
	var_recepcion = malloc(tamanio_recepcion);
	recv(socket_MUSE, var_recepcion, tamanio_recepcion, 0);
	int resultado;
	memcpy(&resultado, var_recepcion, tamanio_recepcion);
	free(var_recepcion);

	if(resultado < 0)
		raise(SIGSEGV);

	return resultado;
}
