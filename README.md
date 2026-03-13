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

## Comandos Rápidos y Experimentos

Para facilitar la evaluación y las pruebas, nuestro `Makefile` incluye varias reglas preconfiguradas que levantan el simulador con distintos escenarios, las cuales puedes ejecutarlas directamente en la terminal:

### Comandos de Ejecución (Pruebas de Desarrollo)
* **`make run-page-safe`**: Paginación segura con semáforos/mutex (4 hilos).
* **`make run-page-unsafe`**: Paginación sin semáforos (causa errores de métricas por condición de carrera).
* **`make run-tlb-safe`**: Paginación segura utilizando una TLB de 16 entradas.
* **`make run-seg-safe`**: Segmentación segura con límites de memoria predefinidos (2 hilos).
* **`make run-seg-unsafe`**: Segmentación sin semáforos para máxima velocidad (demostración de *data races*).

### Experimentos (Evaluación)
* **`make exp1`**: Test de segmentación para probar protección de memoria (generación de *SegFaults*).
* **`make exp2-notlb`**: Paginación base sin caché para medir la latencia y lentitud pura.
* **`make exp2-tlb`**: Paginación con TLB para demostrar la mejora de rendimiento gracias a la localidad de referencias (workload 80-20).
* **`make exp3-1t`**: Genera *Thrashing* (alta tasa de reemplazo de páginas) con 1 hilo.
* **`make exp3-8t`**: Genera *Thrashing* masivo (Test de estrés) con 8 hilos compitiendo por poca RAM.

## Resultados y Análisis de Experimentos

A continuación, se presentan los resultados obtenidos al ejecutar los casos de prueba solicitados mediante nuestro `Makefile`, junto con el análisis de su comportamiento.

### Experimento 1: Segmentación y Protección de Memoria
* **Configuración:** 1 Hilo, 10.000 operaciones, Workload Uniforme, 4 Segmentos.
* **Resultados Clave:**
  * **Traducciones Exitosas:** 4.264
  * **Segmentation Faults:** 5.736 (57.36%)
  * **Tiempo Promedio:** 49 ns

**Análisis:**
Se observa una tasa de 57.36% de *Segmentation Faults*. Esto valida rigurosamente que el simulador está verificando de manera correcta los límites de memoria (`offset < limit`). Dado que se utilizó un *workload* uniforme para la generación de direcciones, es estadísticamente coherente que más de la mitad de las direcciones aleatorias caigan fuera de los rangos limitados de los segmentos permitidos, siendo bloqueadas por el sistema.

### Experimento 2: Paginación y el Efecto de la TLB
* **Configuración:** 1 Hilo, 50.000 operaciones, Workload 80-20 (Localidad), 128 Páginas, 64 Marcos Físicos.

| Métrica | Sin TLB (`exp2-notlb`) | Con TLB 32 entradas (`exp2-tlb`) |
| :--- | :--- | :--- |
| **TLB Hit Rate** | N/A (0%) | 12.7% (6.327 hits) |
| **Page Faults** | 64 | 64 |
| **Evictions (Desalojos)** | 0 | 0 |
| **Tiempo Promedio de Traducción** | 4.588 ns | 4.761 ns |
| **Throughput (Ops/sec)** | ~217.930 | ~210.000 |

**Análisis:**
Al comparar ambos escenarios, ocurre un fenómeno interesante y propio de la simulación por software: el tiempo promedio de traducción subió levemente al activar la TLB (de 4588 ns a 4761 ns). A primera vista parece contradictorio, pero tiene una explicación lógica en nuestra arquitectura: gestionar la caché (iterar buscando en la estructura de la TLB) consume ciclos de CPU del simulador que pesan más que el ahorro real. Esto sucede porque en nuestro programa el acceso a la Tabla de Páginas es casi "instantáneo" (un acceso directo a un índice de un arreglo), por lo que el costo extra de búsqueda secuencial en la TLB no alcanza a compensarse con los tiempos simulados.

A pesar de ese pequeño *overhead* computacional, los datos confirman que la TLB cumple su objetivo algorítmico: detectó con éxito la localidad de referencia espacial (workload 80-20), atrapando más de 6.000 traducciones directamente en caché (un 12.7% de Hit Rate). 

Adicionalmente, el hecho de que ambos escenarios registren exactamente 64 *Page Faults* y **0** *Evictions* nos indica que el *working set* del proceso se acomodó perfectamente en los 64 marcos físicos asignados en la RAM. En palabras simples: una vez que el sistema cargó las páginas necesarias al principio (misses obligatorios), la memoria se estabilizó y no tuvo que volver a aplicar políticas de reemplazo hacia el disco.

### Experimento 3: Thrashing y Competencia por Recursos (Stress Test)
* **Configuración:** Workload Uniforme, 64 Páginas Virtuales, 8 Marcos Físicos, TLB de 16, 10.000 operaciones por hilo.

| Métrica | 1 Hilo (`exp3-1t`) | 8 Hilos (`exp3-8t`) |
| :--- | :--- | :--- |
| **Page Faults** | 8.801 | 70.222 |
| **Evictions (Desalojos)** | 8.793 | 70.214 |
| **Tiempo Promedio de Traducción** | 2.833.656 ns | 2.775.691 ns |
| **Throughput (Ops/sec)** | 352.90 | 360.27 |
| **Tiempo Total de Ejecución** | 28.337 seg | 222.055 seg |

**Análisis:**
Los resultados demuestran de forma concluyente que el sistema entró en estado de *Thrashing* (hiperpaginación). Al contar con solo 8 marcos de memoria física para 64 páginas virtuales y un workload uniforme (sin localidad), el sistema pasó muchísimo más tiempo realizando desalojos (*evictions*) e intercambios simulados de disco que resolviendo accesos a memoria efectivos (hits). Como consecuencia directa, se malgastó completamente la utilización del procesador y la latencia promedio de traducción se volvió unas 600 veces más lenta (casi 3 milisegundos por operación) en comparación al Experimento 2.

El escenario empeora críticamente al someter el sistema a 8 hilos. El tiempo total de ejecución se disparó a casi 4 minutos (222 segundos). La RAM se convirtió en un cuello de botella enorme, generando conflictos y bloqueos constantes entre los distintos procesos que quedaban encolados esperando la liberación del candado de memoria (`RamMutex`). Esto demuestra empíricamente cómo el *Thrashing*, agravado por la alta concurrencia, reduce a niveles críticos la eficiencia general del sistema operativo.