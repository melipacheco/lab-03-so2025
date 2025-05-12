#include <stdio.h>      // Para printf
#include <stdlib.h>     // Para funciones estándar (como exit)
#include <signal.h>     // Para trabajar con señales
#include <string.h>     // Para strsignal (convierte número de señal a texto)
#include <unistd.h>     // Para pause()


void handler(int sig) {
    // Imprime el número de la señal y su descripción
    printf("%d: %s\n", sig, strsignal(sig));
    fflush(stdout);  // Asegura que se imprima en pantalla aunque haya buffering
}

int main() {
    struct sigaction sa;               // Estructura para definir la acción a tomar con cada señal
    sa.sa_handler = handler;           // Establece que el manejador de señales será nuestra función handler
    sigemptyset(&sa.sa_mask);          // Inicializa la máscara de señales vacía (no bloquea señales adicionales)
    sa.sa_flags = 0;                   // No se usan flags especiales

    // Registrar el handler para todas las señales posibles
    for (int i = 1; i < _NSIG; i++) {  // _NSIG es el número total de señales + 1
        // Ignorar señales que no se pueden atrapar
        if (i == SIGKILL || i == SIGSTOP)  // Estas señales NO pueden atraparse
            continue;                      // Así que las salteamos
        sigaction(i, &sa, NULL);           // Registramos el handler para cada señal válida
    }
    printf("Esperando señales... PID: %d\n", getpid());

    // Esperar señales indefinidamente
    while (1) {
        pause();  // El proceso se detiene aquí hasta que reciba una señal
    }
    return 0;
}