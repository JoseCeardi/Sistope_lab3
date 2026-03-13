# Simulador de Memoria Virtual (Sistope - Lab 3)

Este proyecto es un simulador multihilo en C que modela la traducción de direcciones de memoria utilizando dos enfoques: **Paginación** (con soporte para TLB) y **Segmentación**. 

## Requisitos y Compilación

El proyecto utiliza un `Makefile` para automatizar el proceso de compilación, utilizando el estándar estricto de C11 (`-std=c11`) y la librería POSIX Threads (`-pthread`).

Para compilar el proyecto completo:
```bash
make
```
Para limpiar los archivos generados:
```bash
make clean
```

## Ejecución de Experimentos

Para facilitar la corrección, hemos configurado una regla en el `Makefile` que ejecuta automáticamente los tres experimentos solicitados en la pauta de evaluación.

Para ejecutar un ejemplo por defecto:
```bash
make run
```
Para ejecutar los experimentos requeridos del laboratorio:
```bash
make reproduce
```

También puede ejecutar el simulador manualmente utilizando los parámetros por consola. Ejemplo (Segmentación - Experimento 1):
```bash
./simulator --mode seg --threads 1 --workload uniform --ops-per-thread 10000 --segments 4 --seg-limits 1024,2048,4096,8192 --seed 100 --stats
```

## Manejo de Concurrencia (Data Races)

El simulador soporta ejecución concurrente configurada a través del flag `--threads`. Para evitar condiciones de carrera al actualizar las métricas globales compartidas (como `total_translations_ok` o `total_segfaults`), se implementaron las siguientes medidas:

* **Modo SAFE (Por defecto):** Se utilizan candados `pthread_mutex_t` para proteger la sección crítica donde los hilos suman sus estadísticas locales a los contadores globales. Además, se protege el uso del generador aleatorio global `rand()` con un mutex dedicado, cumpliendo con el estándar C puro.
* **Modo UNSAFE (`--unsafe`):** Se desactivan los mutexes intencionalmente. Al someter el programa a una alta carga de estrés con múltiples hilos, se observa la pérdida de datos debido a las condiciones de carrera simultáneas.

