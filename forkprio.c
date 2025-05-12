#include <stdio.h>          // Para printf, fprintf
#include <stdlib.h>         // Para exit, atoi
#include <unistd.h>         // Para fork, sleep, getpid
#include <signal.h>         // Para señales: sigaction, kill, SIGTERM
#include <sys/wait.h>       // Para waitpid
#include <sys/resource.h>   // Para nice, setpriority, getpriority
#include <sys/times.h>      // Para times (función que consume CPU)
#include <time.h>           // Para medir tiempo con time()

volatile sig_atomic_t running = 1;  // Variable modificada por señal

void handler(int sig) {
    running = 0;  // Cuando recibe SIGTERM, deja de correr
}


int busywork() {
    struct tms buf;
    while (running) {
        times(&buf);  // Llama repetidamente a times para gastar CPU
    }
    return 0;
}


int main(int argc, char *argv[])
{
    if (argc != 4) {
    fprintf(stderr, "Uso: %s <cant_hijos> <segundos> <prioridades>\n", argv[0]);
    exit(EXIT_FAILURE);
    }

    int hijos = atoi(argv[1]);
    int segundos = atoi(argv[2]);
    int cambiar_prioridad = atoi(argv[3]);

    pid_t pids[hijos];

    for (int i = 0; i < hijos; i++) {
        pid_t pid = fork();
        if (pid == 0) {  // Hijo
            if (cambiar_prioridad) {
                nice(i);  // Aumenta el nice → menos prioridad
            }

            struct sigaction sa;
            sa.sa_handler = handler;
            sigemptyset(&sa.sa_mask);
            sa.sa_flags = 0;
            sigaction(SIGTERM, &sa, NULL);

            struct timeval inicios, fins;
            gettimeofday(&inicios, NULL);

            time_t inicio = time(NULL);
            busywork();
            time_t fin = time(NULL);

            int prio = getpriority(PRIO_PROCESS, 0);
            //printf("Child %d (nice %2d):\t%li\n", getpid(), prio, fin - inicio);

            gettimeofday(&fins, NULL);

            long segundos = fins.tv_sec - inicios.tv_sec;
            long milisegundos = (fins.tv_usec - inicios.tv_usec) / 1000;
            long total_ms = segundos * 1000 + milisegundos;

            printf("Child %d (nice %2d):\t%ld ms\n", getpid(), prio, total_ms);
            
            fflush(stdout);
            exit(EXIT_SUCCESS);
        } else if (pid > 0) {
            pids[i] = pid;
        } else {
            perror("fork");
            exit(EXIT_FAILURE);
        }
    }

    if (segundos > 0) {
        sleep(segundos);
        for (int i = 0; i < hijos; i++) {
            kill(pids[i], SIGTERM);
        }

        for (int i = 0; i < hijos; i++) {
            waitpid(pids[i], NULL, 0);
        }
    } else {
        pause();  // El padre espera indefinidamente si segundos == 0
    }

    return 0;
}

