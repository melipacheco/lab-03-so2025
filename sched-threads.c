#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <strings.h>
#include <pthread.h>

pthread_t *threads;
int *buf;

int items;
int count;
int pos = 0;
int done = 0;

void *write_buffer(void *thread_id) {
    int i, j;
    long id;
    id = (long) thread_id;

    for (i = 0; i < items; i++) {   
        // Escribe el mismo valor en el mismo lugar varias veces.
        // Asi se consumo tiempo de CPU para ocupar lo más posible el quantum.
        for(j= 0; j< 0xFFFFFF; j++) { 
            buf[pos] = (int) id;
        }
        pos = pos + 1;
    }

    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    int i, sched_policy;
    void *status;

    if (argc != 4) {
        fprintf(stderr, "Uso: %s [-R | -F] hilos items\n", argv[0]);
        fprintf(stderr, "\t-R       SCHED_RR.\n");
        fprintf(stderr, "\t-F       SCHED_FIFO.\n");
        fprintf(stderr, "\thilos    Número de hilos a generar.\n");
        fprintf(stderr, "\titems    Cuantos items escribe el hilo en el buffer.\n");
        exit(EXIT_SUCCESS);
    }
    
    count = atoi(argv[2]);
    items = atoi(argv[3]);
    
    if (count <= 0) {
        fprintf(stderr, "Error: el numero de hilos debe ser mayor que cero.\n");
        exit(EXIT_FAILURE);
    }

    if (items <= 0) {
        fprintf(stderr, "Error: el numero de items debe ser mayor que cero.\n");
        exit(EXIT_FAILURE);
    }

    if (argv[1][1] != 'R' && argv[1][1] != 'F') {
        fprintf(stderr, "Error: opción invalida %s.\nUsar -R (SCHED_RR) o -F (SCHED_FIFO)\n", argv[1]);
        exit(EXIT_FAILURE);
    }

    sched_policy = argv[1][1] == 'R' ? SCHED_RR : SCHED_FIFO;

    // Reserva espacio para un arreglo donde guardar variables de tipo pthread_t
    threads = (pthread_t*) malloc(count * sizeof(pthread_t));

    // Reserva espacio para el buffer donde los hilos escribiran su identificador
    buf = (int*) malloc( count * items * sizeof(int));
    bzero(buf, items * count * sizeof(int));

    srand(time(NULL));

    pthread_attr_t attr;
    struct sched_param param;

    // Inicializa la estructura attr
    pthread_attr_init(&attr);

    // Indica que al crear un hilo usando attr como parámetros, este debe
    // utilizar la política de planificación indicada en dichos parámetros.
    pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);

    // Especifica la política de planificación.
    pthread_attr_setschedpolicy(&attr, sched_policy);

    // Indica el nivel de prioridad que tendrá el hilo creado utilizando attr.
    // param.sched_priority = 1;
    param.sched_priority = sched_get_priority_max(sched_policy);
    pthread_attr_setschedparam(&attr, &param);

    // Indica que el hilo creado utilizando el atributo attr debe ejecutar
    // siempre en la CPU 0.
    // COMPLETAR: usar CPU_ZERO, CPU_SET y pthread_attr_setaffinity_np()

    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(0, &cpuset);
    pthread_attr_setaffinity_np(&attr, sizeof(cpu_set_t), &cpuset);

    // Crea los hilos.
    for (i = 0; i < count; i++) {
        if (pthread_create(&threads[i], &attr, write_buffer, (void *)(long)i)) {
            perror("Error creando hilo");
            exit(EXIT_FAILURE);
        }
    }

    // Espera a que terminen todos los hilos.
    for (i = 0; i < count; i++) {
        pthread_join(threads[i], &status);
    }

    // Imprime el buffer.
    for (i = 0; i < count * items; i++) {
        printf("%d ", buf[i]);
    }
    printf("\n");

    pthread_attr_destroy(&attr);
    free(threads);
    free(buf);

    pthread_exit(NULL);
}
