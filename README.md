# GSEA - General System for Encryption and Archiving

GSEA es una utilidad de línea de comandos desarrollada en C++17 para comprimir, descomprimir, encriptar y desencriptar archivos y directorios. El proyecto implementa algoritmos propios de compresión y criptografía sin depender de librerías externas, utilizando exclusivamente llamadas al sistema POSIX y técnicas de programación concurrente con pthreads.

## Características Principales

- ✅ **Algoritmos propios:** Implementación desde cero de RLE y Differential Encoding para compresión
- ✅ **Cifrado integrado:** Soporte para cifrado Vigenère y XOR
- ✅ **Procesamiento concurrente:** Un hilo por archivo para máximo paralelismo
- ✅ **Syscalls directas:** Uso de llamadas al sistema POSIX (open, read, write, close, stat, opendir, readdir)
- ✅ **Procesamiento recursivo:** Soporte para directorios completos
- ✅ **Operaciones combinadas:** Compresión + Encriptación en una sola operación

## Requisitos del Sistema

- Sistema operativo: Linux/Unix (POSIX)
- Compilador: GCC/G++ con soporte C++17
- Librerías: pthread (incluida en sistemas POSIX)
- Build system: Make

## Compilación

```bash
# Limpiar compilaciones anteriores
make clean

# Compilar el proyecto
make

# Verificar que compiló correctamente
ls -lh bin/gsea
```

El ejecutable se generará en `bin/gsea`.

## Uso

### Sintaxis General

```bash
./bin/gsea [OPCIONES] --input <path> [--output <path>]
```

### Opciones Disponibles

| Flag Largo | Flag Corto | Descripción | Requerido |
|------------|-----------|-------------|-----------|
| `--compress` | `-c` | Comprime archivos | No |
| `--decompress` | `-d` | Descomprime archivos | No |
| `--encrypt` | `-e` | Encripta archivos | No |
| `--decrypt` | `-r` | Desencripta archivos | No |
| `--input <path>` | `-i <path>` | Ruta de entrada (archivo o directorio) | **Sí** |
| `--output <path>` | `-o <path>` | Ruta de salida | No |
| `--key <key>` | `-k <key>` | Clave para encriptación/desencriptación | Sí (si -e/-r) |
| `--comp-alg <alg>` | `-a <alg>` | Algoritmo de compresión: `rle` (default) o `diff` | No |
| `--enc-alg <alg>` | `-b <alg>` | Algoritmo de encriptación: `vigenere` (default) o `xor` | No |

### Algoritmos de Compresión

- **RLE (Run-Length Encoding)** - Por defecto: Codifica secuencias repetidas como `[count][byte]`. Efectivo para datos repetitivos como logs y texto.
- **Differential Encoding** (`diff`): Almacena diferencias entre bytes consecutivos. Efectivo para datos numéricos con cambios graduales.

### Algoritmos de Encriptación

- **Vigenère** - Por defecto: Cifrado que suma cada byte con la clave módulo 256. Más seguro que XOR.
- **XOR**: Operación XOR bit a bit con la clave. Extremadamente rápido pero menos seguro.

## Ejemplos de Uso

### 1. Compresión Básica

```bash
# Comprimir un archivo
./bin/gsea --compress --input archivo.txt --output archivo.rle

# O usando flags cortos
./bin/gsea -c -i archivo.txt -o archivo.rle
```

### 2. Descompresión

```bash
./bin/gsea --decompress --input archivo.rle --output archivo_restaurado.txt

# Verificar integridad
diff archivo.txt archivo_restaurado.txt
```

### 3. Encriptación

```bash
# Encriptar con Vigenère (por defecto)
./bin/gsea --encrypt --input archivo.txt --output archivo.enc -k "mi_clave_secreta"

# Encriptar con XOR
./bin/gsea --encrypt --input archivo.txt --output archivo.enc -k "clave" --enc-alg xor
```

### 4. Desencriptación

```bash
# Desencriptar (usar el mismo algoritmo que se usó para encriptar)
./bin/gsea --decrypt --input archivo.enc --output archivo.txt -k "mi_clave_secreta"

# Con XOR
./bin/gsea --decrypt --input archivo.enc --output archivo.txt -k "clave" --enc-alg xor
```

### 5. Compresión + Encriptación (Operación Combinada)

```bash
# Comprimir y encriptar en una sola operación
./bin/gsea --compress --encrypt --key "clave_secreta" \
           --input archivo.txt --output archivo.rle.enc

# O combinado
./bin/gsea -ce -k "clave_secreta" -i archivo.txt -o archivo.rle.enc
```

**Nota importante:** El orden es: primero comprime, luego encripta.

### 6. Desencriptación + Descompresión

```bash
# Desencriptar y descomprimir (orden inverso)
./bin/gsea --decrypt --decompress --key "clave_secreta" \
           --input archivo.rle.enc --output archivo_restaurado.txt

# O combinado
./bin/gsea -rd -k "clave_secreta" -i archivo.rle.enc -o archivo_restaurado.txt
```

**Nota importante:** El orden es: primero desencripta, luego descomprime.

### 7. Procesar Directorio Completo

```bash
# Comprimir todos los archivos en un directorio
./bin/gsea --compress --input directorio/ --output directorio_comprimido/

# El directorio de salida se crea automáticamente si no existe
# Cada archivo se procesa en un hilo separado para máximo paralelismo
```

### 8. Especificar Algoritmos

```bash
# Compresión con Differential Encoding
./bin/gsea --compress --comp-alg diff --input archivo.txt --output archivo.diff

# Encriptación XOR
./bin/gsea --encrypt --enc-alg xor --key "clave" \
           --input archivo.txt --output archivo.xor

# Combinación: Differential + XOR
./bin/gsea --compress --comp-alg diff --encrypt --enc-alg xor \
           --key "clave" --input archivo.txt --output archivo.diff.xor
```

## Casos de Uso Comunes

### Backup Comprimido y Encriptado

```bash
./bin/gsea -ce -k "backup_key_2024" -i documentos/ -o backup_encrypted/
```

### Comprimir Logs Antiguos

```bash
./bin/gsea -c --comp-alg rle -i logs/ -o logs_compressed/
```

### Encriptar Archivo Sensible

```bash
./bin/gsea -e --enc-alg vigenere -k "clave_fuerte_123" \
           -i secreto.txt -o secreto.enc
```

### Procesar Múltiples Archivos en Paralelo

```bash
./bin/gsea -c -i carpeta_con_100_archivos/ -o salida/
# Crea 100 hilos, uno por archivo, procesando en paralelo
```

## Arquitectura del Proyecto

```
GSEA/
├── bin/              # Ejecutable compilado
├── build/            # Archivos objeto (.o)
├── include/          # Headers (.h)
│   ├── cli.h
│   ├── file_manager.h
│   ├── utils.h
│   └── worker.h
├── src/              # Código fuente (.cpp)
│   ├── main.cpp      # Orquestador principal
│   ├── cli.cpp       # Parser de argumentos
│   ├── file_manager.cpp  # Gestión de archivos con syscalls
│   ├── worker.cpp    # Algoritmos de compresión/encriptación
│   └── utils.cpp     # Utilidades y logging thread-safe
├── tests/            # Pruebas automáticas
├── Makefile          # Sistema de compilación
└── README.md         # Este archivo
```

## Concurrencia

GSEA utiliza pthreads para procesar múltiples archivos en paralelo:

- **Un hilo por archivo:** Cada archivo se procesa en un hilo separado
- **Procesamiento paralelo real:** Múltiples archivos se procesan simultáneamente en diferentes cores
- **Logging thread-safe:** Mensajes protegidos con mutex para evitar intercalado
- **Escalabilidad:** Rendimiento mejora linealmente con número de cores

## Llamadas al Sistema

GSEA utiliza llamadas al sistema POSIX directas en lugar de funciones de alto nivel:

- `open()`, `read()`, `write()`, `close()` - Operaciones de archivos
- `stat()` - Información de archivos y directorios
- `opendir()`, `readdir()`, `closedir()` - Recorrido de directorios

Esto proporciona control total sobre las operaciones y demuestra conocimiento de APIs del sistema operativo.

## Notas Importantes

- **Orden de operaciones:** Al combinar compresión y encriptación, primero se comprime y luego se encripta. Al revertir, primero se desencripta y luego se descomprime.
- **Algoritmos consistentes:** El algoritmo usado para encriptar debe ser el mismo para desencriptar.
- **Claves:** Las claves se repiten cíclicamente si son más cortas que los datos.
- **Rendimiento:** El procesamiento paralelo mejora significativamente el tiempo en sistemas multi-core. Con 10 archivos en 4 cores: ~4x más rápido que secuencial.

## Pruebas

El proyecto incluye una suite de pruebas automáticas (22/22 pruebas pasadas):

- Compresión y descompresión RLE
- Compresión y descompresión Differential
- Encriptación y desencriptación Vigenère
- Encriptación y desencriptación XOR
- Operaciones combinadas
- Procesamiento de directorios
- Validación de errores

Para ejecutar las pruebas, consultar `GUIA_COMPILACION_Y_PRUEBAS.md`.

## Documentación Adicional

- **DOCUMENTO_TECNICO_OPTIMIZADO.tex:** Documento técnico completo con justificación de algoritmos, diseño de solución, estrategia de concurrencia y uso de syscalls.
- **GUIA_COMPILACION_Y_PRUEBAS.md:** Guía detallada de compilación, pruebas y ejemplos avanzados.

## Licencia

Este proyecto fue desarrollado como trabajo final para la asignatura de Sistemas Operativos.

## Autores

- Camilo Arbelaez
- Isabella Bejarano
- Juan Gonzalo De Los Rios
