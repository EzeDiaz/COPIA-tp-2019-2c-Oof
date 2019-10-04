#ifndef SERVERMUSE_H_
#define SERVERMUSE_H_

void iniciarServidor();
void atenderCliente(int cliente);
void* recibirBuffer(int* alocador, int cliente);
void realizarRequest(void *buffer,int cliente);

#endif /* SERVERMUSE_H_ */
