/*
 * SAC-Cli.c
 *
 *  Created on: 30 oct. 2019
 *      Author: utnso
 */

#define SERVER_NAME_LEN_MAX 255
#define _FILE_OFFSET_BITS  64
#define FUSE_USE_VERSION 26

#include "consola_tester.h"
#include "SAC-Cli.h"
#include <readline/readline.h>
#include <stddef.h>
#include <stdlib.h>
#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>

#define DEFAULT_FILE_PATH "/" DEFAULT_FILE_NAME

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
	//.getattr     = fs_getattr,
	//.readlink    = fs_readlink,
	//.mknod       = fs_mknod,
	.mkdir       = serializar_fs_mkdir,
	/*.unlink      = fs_unlink,
	*/.rmdir       = serializar_fs_rmdir,
	/*.symlink     = fs_symlink,
	.rename      = fs_rename,
	.link        = fs_link,
	.chmod       = fs_chmod,
	.chown       = fs_chown,
	.truncate    = fs_truncate,*/
	.open        = serializar_fs_open,
	.read        = serializar_fs_read,
	.write       = serializar_fs_write,
	/*.statfs      = fs_statfs,
	.flush       = fs_flush,
	.release     = fs_release,
	.fsync       = fs_fsync,
	.setxattr    = fs_setxattr,
	.getxattr    = fs_getxattr,
	.listxattr   = fs_listxattr,
	.removexattr = fs_removexattr,
	.opendir     = fs_opendir,*/
	.readdir     = serializar_fs_readdir,
	/*.releasedir  = fs_releasedir,
	.fsyncdir    = fs_fsyncdir,
	.init        = fs_init,
	.destroy     = fs_destroy,
	.access      = fs_access,
	.create      = fs_create,
	.ftruncate   = fs_ftruncate,
	.fgetattr    = fs_fgetattr,
	.lock        = fs_lock,
	.utimens     = fs_utimens,
	.bmap        = fs_bmap,
	.ioctl       = fs_ioctl,
	.poll        = fs_poll,*/
};

int main(int argc, char *argv[]){
	config= leer_config();
	conectar_SAC_SERVER(argc,argv);
	struct fuse_args args = FUSE_ARGS_INIT(argc, argv);

		// Limpio la estructura que va a contener los parametros
		//memset(&runtime_options, 0, sizeof(struct t_runtime_options));

		// Esta funcion de FUSE lee los parametros recibidos y los intepreta
		//if (fuse_opt_parse(&args, &runtime_options, fuse_options, NULL) == -1){
			/** error parsing options */
			//perror("Invalid arguments!");
			//return EXIT_FAILURE;
		//}

		consola();
		// Si se paso el parametro --welcome-msg
		// el campo welcome_msg deberia tener el
		// valor pasado

		// Esta es la funcion principal de FUSE, es la que se encarga
		// de realizar el montaje, comuniscarse con el kernel, delegar todo
		// en varios threads
	return /*fuse_main(argc, argv, &fs_oper,NULL)*/ 0;
}

t_config* leer_config(){

	char* nombre_config= readline("ingresar nombre de config: \n>");
	return config_create(nombre_config);

}

void conectar_SAC_SERVER(int argc, char *argv[]) {

		char* server_name=config_get_string_value(config,"IP");
	    int server_port, socket_fd;
	    struct hostent *server_host;
	    struct sockaddr_in server_address;

	    /* Get server name from command line arguments or stdin. */


	    /* Get server port from command line arguments or stdin. */
	    server_port = argc > 2 ? atoi(argv[2]) : 0;
	    if (!server_port) {
	    	server_port=config_get_int_value(config,"PUERTO_ESCUCHA");
	    }

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
