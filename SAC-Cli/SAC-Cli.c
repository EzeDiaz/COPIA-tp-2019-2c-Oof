/*
 * SAC-Cli.c
 *
 *  Created on: 30 oct. 2019
 *      Author: utnso
 */

#define _FILE_OFFSET_BITS 64
#define SERVER_NAME_LEN_MAX 255
#define FUSE_USE_VERSION 27

#include "SAC-Cli.h"
#include <readline/readline.h>
#include <stddef.h>
#include <stdlib.h>
#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>

#include <stdio.h>
#include <stdlib.h>

#define DEFAULT_MOUNT_POINT "/"

#define DEFAULT_FILE_CONTENT ""

#define DEFAULT_FILE_NAME ""

#define DEFAULT_FILE_PATH "/"

struct t_runtime_options {
	char* welcome_msg;
} runtime_options;

enum {
	KEY_VERSION,
	KEY_HELP,
};

static struct fuse_opt fuse_options[] = {
		FUSE_OPT_KEY("-V", KEY_VERSION),
		FUSE_OPT_KEY("--version", KEY_VERSION),
		FUSE_OPT_KEY("-h", KEY_HELP),
		FUSE_OPT_KEY("--help", KEY_HELP),
		FUSE_OPT_END,
};


static struct fuse_operations fs_oper = {

		.open        = serializar_fs_open,
		.read        = serializar_fs_read,
		.write       = serializar_fs_write,
		.opendir     = serializar_fs_opendir,
		.readdir     = serializar_fs_readdir,
		.create      = serializar_fs_create,
		.rmdir       = serializar_fs_rmdir,
		.mkdir       = serializar_fs_mkdir,
		.getattr     = serializar_fs_getattr,
		.mknod       = serializar_fs_mknod,
		.rename      = serializar_fs_rename,

		/*	.readlink    = fs_readlink,
		.unlink      = fs_unlink,
		.symlink     = fs_symlink,
		.link        = fs_link,
		.chmod       = fs_chmod,
		.chown       = fs_chown,
		.truncate    = fs_truncate,
		.statfs      = fs_statfs,
		.flush       = fs_flush,
		.release     = fs_release,
		.fsync       = fs_fsync,
		.setxattr    = fs_setxattr,
		.getxattr    = fs_getxattr,
		.listxattr   = fs_listxattr,
		.removexattr = fs_removexattr,
		.releasedir  = fs_releasedir,
		.fsyncdir    = fs_fsyncdir,
		.init        = fs_init,
		.destroy     = fs_destroy,
		.access      = fs_access,
		.ftruncate   = fs_ftruncate,
		.fgetattr    = fs_fgetattr,
		.lock        = fs_lock,
		.utimens     = fs_utimens,
		.bmap        = fs_bmap,
		.ioctl       = fs_ioctl,
		.poll        = fs_poll,*/
};



int main(int argc, char **argv){
	remove("/home/utnso/SAC");

	leer_config();
	printf("vamos a conectar con server \n");

	conectar_SAC_SERVER(argc,argv[1]);

	printf("conecta3\n");


	printf("vamos a iniciar fuse_args_init \n");

	struct fuse_args args = FUSE_ARGS_INIT(argc, argv);

	printf("memseteamos runtime options \n");

	// Limpio la estructura que va a contener los parametros
	memset(&runtime_options, 0, sizeof(struct t_runtime_options));

	printf("vamos a iniciar fuse_opt_parse \n");

	// Esta funcion de FUSE lee los parametros recibidos y los intepreta
	if (fuse_opt_parse(&args, &runtime_options, fuse_options, NULL) == -1){
		/** error parsing options */
		perror("Invalid arguments!");
		return EXIT_FAILURE;
	}

	// Si se paso el parametro --welcome-msg
	// el campo welcome_msg deberia tener el
	// valor pasado

	if( runtime_options.welcome_msg != NULL ){
		printf("%s\n", runtime_options.welcome_msg);
	}
	// Esta es la funcion principal de FUSE, es la que se encarga
	// de realizar el montaje, comuniscarse con el kernel, delegar todo
	// en varios threads
	printf("vamos a iniciar fuse_main \n");

	// system("fusermount -u /home/utnso/New_SAC");
	 return fuse_main(args.argc, args.argv, &fs_oper,NULL) ;

	}

void leer_config(){

	char* nombre_config= readline("Ingresar nombre de config: \n>");
	config= config_create(nombre_config);

	IP=config_get_string_value(config,"IP");
	puerto=config_get_int_value(config,"PUERTO_ESCUCHA");

}

void conectar_SAC_SERVER(int argc, char *argv) {

	printf("vamos a leer ip \n");

	char* server_name=IP;
	int server_port, socket_fd;
	struct hostent *server_host;
	struct sockaddr_in server_address;

		printf("vamos a leer puerto \n");

		server_port=puerto;
		/* Get server host from server name. */
	server_host = gethostbyname(server_name);

	/* Initialise IPv4 server address with server host. */
	memset(&server_address, 0, sizeof server_address);
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(server_port);
	memcpy(&server_address.sin_addr.s_addr, server_host->h_addr, server_host->h_length);

	/* Create TCP socket. */
	socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_fd == -1) {
		perror("Error en la creacion del socket");
		exit(1);
	}


	/* Connect to socket with server address. */
	if (connect(socket_fd, (struct sockaddr *)&server_address, sizeof server_address) == -1) {
		perror("Error en la conexion del socket");
		exit(1);
	}

	socket_sac_server=socket_fd;

}


/*
void handshake(int socket){

	send(socket,"bananitaDolca", 13,0);
	int pesoRecibir=sizeof(int);
	void* pesoPagina=(void*)malloc(sizeof(int));
	printf("vamos a recibir algo\n");
	recv(socket,pesoPagina,pesoRecibir,MSG_WAITALL);
	memcpy(&pesoRecibir,pesoPagina,sizeof(int));
	printf("%d\n",pesoRecibir);
	sizeValueFS=pesoRecibir;
	free(pesoPagina);

}
 */
