#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>   
#include <sys/times.h>

void test_fork(int count, int wait);
void test_thread(int count, int wait);

void* dummy_thread(void* arg) {
    // Función ejecutada por cada hilo
    pthread_exit(NULL);
    return NULL;
}

int main(int argc, char *argv[]) 
{
    int modo, wait, count;
    
    // Chequea los  parametros
    if (argc < 3) {
        fprintf(stderr, "Uso: %s [-p | -h] [-w] cantidad\n", argv[0]);
        fprintf(stderr, "\t-p           Crear procesos.\n");
        fprintf(stderr, "\t-t           Crear hilos.\n");
        fprintf(stderr, "\t-w           Esperar que proceso o hilo finalice.\n");
        fprintf(stderr, "\tcantidad     Número de procesos o hilos a crear.\n");
        exit(EXIT_FAILURE);
    }
    
    modo = argv[1][1];  // debe ser p o t

    if (argc == 4) { 
        wait = argv[2][1];
        if (wait != 'w') {
            fprintf(stderr, "Error: opción invalida %s.\n", argv[2]);
            exit(EXIT_FAILURE);
        }
        count = atoi(argv[3]);
    } else {
        wait = 0;
        count = atoi(argv[2]);
    }

    
    if (count <= 0) {
        fprintf(stderr, "Error: el contador debe ser mayor que cero.\n");
        exit(EXIT_FAILURE);
    }

    if (modo != 'p' && modo != 't') {
        fprintf(stderr, "Error: opción invalida %s.\nUsar -p (procesos) o -t (hilos)\n", argv[1]);
        exit(EXIT_FAILURE);
    }
    
    time_t inicio = time(NULL);
    if (modo == 'p') {
        printf("Probando fork()...\n");
        test_fork(count, wait);
    } else if (modo == 't') {
        printf("Probando pthread_create()...\n");
        test_thread(count, wait);
    }
    time_t fin = time(NULL);
    printf("Tiempo: %li\n", fin - inicio);


    exit(EXIT_SUCCESS);
}

//==========================================================================
// Código para la prueba con fork()
//==========================================================================
void test_fork(int count, int wait)
{
    pid_t pids[count];
    for(int i = 0; i < count; i++){
        pid_t pid = fork();
        if (pid == 0) {
            exit(0);
        } else {
            pids[i] = pid;
            if(wait && i < count-1){
                waitpid(pid, NULL, 0);            
            }
        }     
    }
}

//==========================================================================
// Código para la prueba con pthread_create()
//==========================================================================

void test_thread(int count, int wait) {
    pthread_t threads[count];

    for (int i = 0; i < count; i++) {
        // Crea el hilo y verifica errores
        if (pthread_create(&threads[i], NULL, dummy_thread, NULL) != 0) {
            perror("pthread_create");
            exit(EXIT_FAILURE);
        }

        // Si wait está activado, esperamos a que este hilo termine
        if (wait) {
            if (pthread_join(threads[i], NULL) != 0) {
                perror("pthread_join");
                exit(EXIT_FAILURE);
            }
        }
    }

    // Si no esperamos uno por uno, esperamos a todos al final
    if (!wait) {
        for (int i = 0; i < count; i++) {
            if (pthread_join(threads[i], NULL) != 0) {
                perror("pthread_join");
                exit(EXIT_FAILURE);
            }
        }
    }
}

