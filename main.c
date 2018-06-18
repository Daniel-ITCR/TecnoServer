#include <sys/time.h>
#include <sys/types.h>
#include <Socket_Servidor.h>
#include <Socket.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include <Commands.h>
#include <json-c/json.h>

#include "DataManager.h"

int socketArrayClientes[MAX_CLIENTS] = {-1, -1, -1, -1};     /* Descriptores de sockets con clientes */
pthread_t threadArray[MAX_CLIENTS];                 /* Array de threads*/

void servidorClienteComunicacion(void *ClientSocket);

void escucharPorClientes(void);

char *getLine(void);

int main()
{
    pthread_t threadListener;               /* Thread que escucha por clientes*/
    int err = pthread_create(&(threadListener), NULL, &escucharPorClientes, NULL);

    if (err != 0)
        printf("\ncan't create master thread :[%s]", strerror(err));
    else {
        pthread_detach(threadListener);
        //pthread_join(threadListener, NULL);
    }

    while (1) {
        printf("Escriba el comando a ejecutar (crearBomba, crearVida o crearBoost): ");
        char *comando = getLine();
        printf("Escriba la posicion horizontal del objeto: ");
        char *x_text = getLine();
        printf("Escriba la posicion vertical del objeto: ");
        char *pos_text = getLine();

        int x_value = *x_text - 48;
        int pos_value = *pos_text - 48;

        if (comando == "crearBomba") {
            crearBomba(x_value, pos_value);
        } else if (comando == "crearVida") {
            crearVida(x_value, pos_value);
        } else if (comando == "crearBoost") {
            crearBoost(x_value, pos_value);
        }

        printf("Elemento creado\n");
    }
}

void escucharPorClientes(void) {
    int sockerServidor;                     /* Descriptor del socket servidor */
    int numeroClientes = 0;                 /* Número clientes conectados */
    fd_set readFs;                          /* Descriptores de interes para select() */
    int buffer_size;                        /* Dice de que tamanho es el string que va a entrar */
    int maximo;                                /* Número de descriptor más grande */
    int i;                                    /* Para los for */

    /* Se abre el socket servidor, avisando por pantalla y saliendo si hay
     * algún problema */
    sockerServidor = Abre_Socket_Inet("dlsrap");
    if (sockerServidor == -1)
    {
        perror ("Error al abrir servidor, puerto ya utilizado");
        exit (-1);
    }

    while (1)
    {

        //Borra sockets inactivos al inicio de cada iteración
        trimClients(socketArrayClientes, &numeroClientes);
        //Se tiene que limpiar, inicializar y rellenar los file descriptors de los sockets cada vez, para que select sepa cual utilizar
        FD_ZERO (&readFs);
        FD_SET (sockerServidor, &readFs);
        /* Se añaden para select() los sockets con los clientes ya conectados */
        for (i=0; i<numeroClientes; i++)
            FD_SET (socketArrayClientes[i], &readFs);

        /* Se el valor del descriptor más grande. Si no hay ningún cliente,
         * devolverá 0 */
        maximo = maxMember(socketArrayClientes, numeroClientes);

        if (maximo < sockerServidor)
            maximo = sockerServidor;

        select (maximo + 1, &readFs, NULL, NULL, NULL);//Aqui queda esperando el servidor a que haya actividad en algun fd(file descriptor) activo

        if (FD_ISSET (sockerServidor, &readFs)) {//verifica si hay clientes nuevos y los registra
            newClient(sockerServidor, socketArrayClientes, &numeroClientes);
            int err = pthread_create(&(threadArray[i]), NULL, &servidorClienteComunicacion,
                                     (void *) &socketArrayClientes[numeroClientes - 1]);

            if (err != 0)
                printf("\ncan't create thread :[%s]", strerror(err));
            else {
                pthread_detach((threadArray[i]));
            }
        }

        if (numeroClientes = 0) {
            cleanPlayers();
        }
    }
}

void servidorClienteComunicacion(void *socketCliente) {

    int cancelability;
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, &cancelability);

    int player;

    if (socketCliente == &socketCliente[0]) {
        player = 1;
    } else if (socketCliente == &socketCliente[1]) {
        player = 2;
    } else if (socketCliente == &socketCliente[2]) {
        player = 3;
    } else if (socketCliente == &socketCliente[3]) {
        player = 4;
    } else {
        return;
    }

    int buffer_size;

    while (1) {
        /* Se lee lo enviado por el cliente y se escribe en pantalla */
        if ((Lee_Socket(*(int *) socketCliente, (char *) &buffer_size, sizeof(int)) >
             0)) { //si es mayor a 0 quiere decir que está entrando un dato
            int str_len = ntohl(
                    (uint32_t) buffer_size);//Buffer size tiene el tamaño del paquete en int pero en formato de red, hay que pasarlo a int de C.
            json_object *mensaje = receiveJson((int *) socketCliente, str_len);
            json_Parser(mensaje);
            json_object *respuesta = data_toSend();
            sendJson((int *) socketCliente, respuesta);
        } else {
            printf("Cliente %d ha cerrado la conexión\n", player);
            *(int *) socketCliente = -1;
            //pthread_cancel(threadArray[player - 1]);
            pthread_exit(NULL);
        }
    }
}


char *getLine(void) {
    char *line = malloc(100), *linep = line;
    size_t lenmax = 100, len = lenmax;
    int c;

    if (line == NULL)
        return NULL;

    for (;;) {
        c = fgetc(stdin);
        if (c == EOF)
            break;

        if (--len == 0) {
            len = lenmax;
            char *linen = realloc(linep, lenmax *= 2);

            if (linen == NULL) {
                free(linep);
                return NULL;
            }
            line = linen + (line - linep);
            linep = linen;
        }

        if ((*line++ = c) == '\n')
            break;
    }
    *line = '\0';
    return linep;
}
