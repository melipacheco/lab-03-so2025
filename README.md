[![Review Assignment Due Date](https://classroom.github.com/assets/deadline-readme-button-22041afd0340ce965d47ae6ef1cefeee28c7c493a6346c4f15d667ab976d596c.svg)](https://classroom.github.com/a/JxQQMDtt)
# Laboratorio 3 - Planificación

## Ejercicio 1

Una señal es análoga a una interrupción de hardware. Al recibir una señal, se interrumpe el normal funcionamiento de un proceso. Las señales pueden ser generadas por el kernel para indicar que ocurrió una excepción de hardware, que el usuario tecleo una combinación especial de carácteres (por ejemplo `^C`) o que ocurrio un evento de software (por ejemplo, un proceso hijo finalizó su ejecución). Un programa puede enviar señales a otros procesos o a si mismo. Una lista completa de las señales se puede consultar en [`signals`](https://man7.org/linux/man-pages/man7/signal.7.html).

En este ejercicio vamos a interceptar las señales que llegan a un proceso. Por defecto, cuando un proceso recibe una señal finaliza su ejecución (en la mayoría de los casos) o la ignora.

Completar el programas `sigcatch.c` de manera que espere por la recepción de señales. Por cada señal que recibe, debe imprimir el identificador de la señal y una descripción de la misma.

Para enviar señales a `sigcatch.c` vamos a utilizar el comando [`kill`](http://man7.org/linux/man-pages/man1/kill.1.html). Por ejemplo, para enviar las señales 15 (`SIGTERM`) y 8 (`SIGFPE`):
```
$ ./sigcatch &
[1] 27309
$ kill -15 27309 
15: Terminated
$ kill -8 27309
8: Floating point exception
$
```

Para implementar `sigcatch.c`, utilizar las siguientes funciones y llamadas al sistema:

* [`sigaction()`](http://man7.org/linux/man-pages/man2/sigaction.2.html): permite cambiar la acción tomada por el proceso al recibir una señal.
* [`pause()`](http://man7.org/linux/man-pages/man2/pause.2.html): espera a recibir una señal.
* [`strsignal()`](http://man7.org/linux/man-pages/man3/strsignal.3.html): retorna un puntero a una cadena con la descripción de la señal indicada.
* El archivo `signal.h` provee la constante `_NSIG` que indica el número de señales del sistema más uno.

También, responder la siguiente pregunta:

1. ¿Cuáles son las señales que no es posible atrapar? ¿Por qué?

## Ejercicio 2: creación y planificación de procesos
El programa `forkprio.c` debe crear un cierto número de procesos hijos, cada uno ejecutando con un valor de prioridad progresivamente más bajo. Luego de un cierto número de segundos se debe envíar una señal de terminación (`SIGTERM`) a todos los procesos hijos. Estos, antes de terminar su ejecución, deben imprimir su PID, prioridad y el número de segundos que ejecutaron.


El siguiente esquema muestra como es la relación con los procesos hijos:
```
      ┌──────────┐                             fork()
      │ forkprio ├───────────┬───────────┬────────────────────┐
      └─────┬────┘           │           │                    │
            │                │           │                    │
            │                │           │                    │
 sleep(secs)│           ┌────▼───┐   ┌───▼────┐          ┌────▼───┐
            .           │ Hijo 1 │   │ Hijo 2 │   ...    │ Hijo n │
            .           └────▲───┘   └───▲────┘          └────▲───┘
            .                │           │                    │
            │                │           │                    │
            │                │           │                    │
            ├────────────────┴───────────┴────────────────────┘
            │                                 kill(SIGTERM)
            │
            ▼
          exit()
```

Requisitos:

- El primer argumento en la linea de comandos debe ser el número de procesos hijos a crear. El segundo parámetro el número de segundos que deben ejecutar. El tercer parámetro indica si se reducen las prioridades (1) o no (0).
- Si el número de segundos es 0 (cero) se debe ejecutar indefinidamente.
- El formato que deben utilizar los procesos hijos al imprimir es `"Child %d (nice %2d):\t%3li\n"` (pid, prioridad, tiempo de ejecución).
- La función `busywork()` en el archivo `forkprio.c` tiene un bucle infinito que repetidamente invoca la función `times()` para consumir tiempo de CPU. Es la función que deben invocar los procesos hijos.

Utilizar las siguientes llamadas al sistema:

* [`fork()`](https://man7.org/linux/man-pages/man2/fork.2.html): crea un nuevo proceso.
* [`setpriority()`](https://man7.org/linux/man-pages/man2/setpriority.2.html): cambia la prioridad de un proceso (también se puede utilizar [`nice()`](https://man7.org/linux/man-pages/man2/nice.2.html)).
* [`getpriority()`](https://man7.org/linux/man-pages/man2/setpriority.2.html): retorna el valor de prioridad un proceso.
* [`getrusage()`](https://man7.org/linux/man-pages/man2/getrusage.2.html): obtiene estadísticas de ejecución de un proceso.
* [`sleep()`](https://man7.org/linux/man-pages/man3/sleep.3.html): suspende la ejecución de un proceso durante el número indicado de segundos.
* [`kill()`](http://man7.org/linux/man-pages/man2/kill.2.html): envía una señal a un proceso.
* [`sigaction()`](http://man7.org/linux/man-pages/man2/sigaction.2.html): permite modificar la acción a ejecutar al recibir una señal.

Para verificar que efectivamente se estan creando los procesos hijos, pueden realizar la siguiente prueba:
```console
$ ./forkprio 5 0 0 >/dev/null &
$ pstree -cp $(pgrep forkprio | head -1)
$ pkill forkprio
```

Responder:

1. Ejecutar `./forkprio 1 0 1 >/dev/null &`. Luego ejecutar el comando `ps -fp $(pgrep forkprio)`. ¿En que estados están los procesos? Explicar.
2. Ejecutar `./forkprio 1 10 1` en tu computadora y luego en el servidor de la cátedra ¿Existe alguna diferencia en el resultado? Ejecutar múltiples veces para estar seguro de los resultados.
3. Ejecutar `./forkprio 10 30 1 | sort -k 4 -h` y describir el resultado de la ejecución. ¿Por qué el total del tiempo ejecutado por los procesos hijos puede ser mayor que el tiempo que espera el proceso padre?
4. Si el comando anterior se ejecuta indicando que no se cambien las prioridades de los procesos hijos, ¿Cúal es el resultado? Explicar por qué.

## Ejercicio 3: creación y planificación de hilos 
Completar el programa `sched-threads.c` para que cree una cierta cantidad de hilos que escriben en un buffer compartido. Este buffer es un arreglo de enteros, donde cada hilo escribe su identificador una cierta cantidad de veces. Al finalizar el programa se imprime este arreglo, de manera que se observa el orden de ejecución de los hilos. Se debe poder especificar como algoritmos de planificación `SCHED_FIFO`, `SCHED_RR` y `SCHED_OTHER`.

Por ejemplo, una vez implementado el programa, una ejecución que cree tres hilos, cada uno escribiendo tres items en el buffer, planificados mediante FIFO, debería dar un resultado similar al siguiente:
```
$ sudo ./sched-threads -F 3 3
0 0 0 1 1 1 2 2 2
$
```

Notar que se debe ejecutar el comando con `sudo` ya que se necesitan mayores privilegios para utilizar las políticas `SCHED_RR` y `SCHED_FIFO`.

## Ejercicio 4: procesos versus hilos
En este ejercicio vamos a comparar el costo de la creación de hilos con el de creación de procesos.

El programa `benchmark.c` crea la cantidad de procesos o de hilos indicada en la línea de comandos. El programa ya cuenta con la funcionalidad de procesos implementanda. Por ejemplo, el siguiente comando crea 1000 procesos hijos (opción `-p`): 
```sh
$ ./benchmark -p -w 1000
```

Cada vez que crea un proceso, espera a que termine antes de crear el siguiente (la opción `-w`).

Se debe implementar el código necesario para crear procesos e hilos (opción `-t`). 

Para el caso de procesos, utilizar las siguientes funciones:

* [`fork()`](https://man7.org/linux/man-pages/man2/fork.2.html): crea un nuevo proceso.
* [`wait()`](https://man7.org/linux/man-pages/man2/wait.2.html) o [`waitpid()`](https://man7.org/linux/man-pages/man2/waitpid.2.html): para esperar por la finalización de un proceso hijo.

Para el caso de hilos, utilizar las siguientes funciones:

* [`pthread_create()`](http://man7.org/linux/man-pages/man3/pthread_create.3.html): crea un hilo.
* [`pthread_join()`](http://man7.org/linux/man-pages/man3/pthread_join.3.html): espera a que el hilo indicado finalice.
* [`pthread_exit()`](http://man7.org/linux/man-pages/man3/pthread_exit.3.html): finaliza la ejecución de un hilo.

Una vez implementada la generación de procesos e hilos, comparar los tiempos de ejecución de ambas opciones utilizando el comando [`time`](http://man7.org/linux/man-pages/man1/time.1.html):

```sh
$ /usr/bin/time -p ./benchmark -p 1000
$ /usr/bin/time -p ./benchmark -t 1000
$ /usr/bin/time -p ./benchmark -p -w 1000
$ /usr/bin/time -p ./benchmark -t -w 1000
```

**Nota**: es importante usar el _path_ completo (`/usr/bin/time`) para no ejecutar el comando `time` provisto por el _shell_. 

Responder:

1. ¿Cual de las dos variantes tuvo menos costo, la creación de hilos o la creación de procesos? Justificar.
2. ¿Cuánto afecta el uso de la opción `-w`?

---

¡Fin del Laboratorio 3!
