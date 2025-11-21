# ANÁLISIS COMPLETO DEL PROYECTO GSEA

## 1. ESTRUCTURA DEL PROYECTO

### Archivos de Código Fuente (.cpp)
- `src/main.cpp` - Punto de entrada principal del programa
- `src/cli.cpp` - Implementación del parser de argumentos de línea de comandos
- `src/file_manager.cpp` - Gestión de archivos y directorios
- `src/worker.cpp` - Implementación de los workers (hilos) que procesan archivos
- `src/utils.cpp` - Utilidades (logging, paths, safe read/write)
- `src/gsea.cpp` - **Implementación alternativa** con algoritmos RLE y XOR (no integrada con main.cpp)

### Archivos de Cabecera (.h)
- `include/cli.h` - Definición de estructura Options y declaración de parse_cli()
- `include/file_manager.h` - Declaraciones de funciones de gestión de archivos
- `include/worker.h` - Definición de WorkerArgs y worker_entry()
- `include/utils.h` - Declaraciones de utilidades
- `src/gsea.h` - **Cabecera alternativa** para funciones de compresión/encriptación (no integrada)

### Estructura de Directorios
```
EntregaFinal-SistemasOperativos/
├── build/              # Directorio para objetos compilados (.o)
├── bin/                # Directorio para el ejecutable (gsea)
├── docs/               # Documentación
├── include/            # Headers públicos
│   ├── cli.h
│   ├── file_manager.h
│   ├── utils.h
│   └── worker.h
├── src/                # Código fuente
│   ├── main.cpp
│   ├── cli.cpp
│   ├── file_manager.cpp
│   ├── worker.cpp
│   ├── utils.cpp
│   ├── gsea.cpp        # ⚠️ NO INTEGRADO con main.cpp
│   └── gsea.h          # ⚠️ NO INTEGRADO con main.cpp
├── tests/              # Tests
│   ├── data/
│   │   └── sample.txt
│   └── sample_data/
├── video/              # Videos/documentación
├── Makefile            # Script de compilación
└── README.md           # Documentación básica
```

### Script de Compilación
**✅ Makefile existe** en la raíz del proyecto.

**Configuración del Makefile:**
- Compilador: `g++`
- Estándar: `C++17`
- Flags: `-O2 -Wall -Wextra -pthread`
- Compila todos los `.cpp` en `src/` automáticamente
- Genera objetos en `build/`
- Ejecutable final: `bin/gsea`

**⚠️ PROBLEMA DETECTADO:** El Makefile NO incluye el directorio `include/` en los flags de compilación (`-Iinclude`), lo que podría causar problemas si los includes no son relativos correctamente.

---

## 2. FUNCIONALIDADES IMPLEMENTADAS

### Operaciones de Línea de Comandos

#### ✅ Parsing de Argumentos (cli.cpp)
El parser está **COMPLETO** y soporta:
- `--compress` / `-c` → Activa compresión
- `--decompress` / `-d` → Activa descompresión
- `--encrypt` / `-e` → Activa encriptación
- `--decrypt` / `-r` → Activa desencriptación (nota: usa 'r' en lugar de 'u')
- `--input <path>` / `-i` → Ruta de entrada (requerido)
- `--output <path>` / `-o` → Ruta de salida
- `--key <key>` / `-k` → Clave para encriptación/desencriptación
- `--comp-alg <alg>` / `-a` → Algoritmo de compresión (parsed pero no usado)
- `--enc-alg <alg>` / `-b` → Algoritmo de encriptación (parsed pero no usado)

**Estado:** ✅ **FUNCIONAL** - Usa `getopt_long()` para parsing robusto.

#### ❌ Operaciones Implementadas

**En `main.cpp` + `worker.cpp` (implementación principal):**
- ✅ **Estructura base completa** - El flujo de trabajo está implementado
- ❌ **Compresión (`-c`)**: Usa `placeholder_compress()` que **NO hace nada** (retorna datos sin modificar)
- ❌ **Descompresión (`-d`)**: Usa `placeholder_decompress()` que **NO hace nada** (retorna datos sin modificar)
- ⚠️ **Encriptación (`-e`)**: Usa XOR simple como placeholder (funcional pero básico)
- ⚠️ **Desencriptación (`-r`)**: Usa XOR simple (simétrico, funcional pero básico)

**En `gsea.cpp` (implementación alternativa NO INTEGRADA):**
- ✅ **Compresión RLE**: Implementación completa de Run-Length Encoding
- ✅ **Descompresión RLE**: Implementación completa
- ✅ **Encriptación XOR**: Implementación completa con clave
- ✅ **Desencriptación XOR**: Implementación completa (simétrica)

**⚠️ PROBLEMA CRÍTICO:** `gsea.cpp` tiene implementaciones reales pero **NO se está usando** en `main.cpp`. El programa principal usa placeholders que no hacen compresión real.

### Algoritmos de Compresión

#### Implementación Principal (worker.cpp)
- ❌ **Ninguno implementado** - Solo placeholders que retornan datos sin modificar

#### Implementación Alternativa (gsea.cpp - NO INTEGRADA)
- ✅ **RLE (Run-Length Encoding)**: 
  - Codifica secuencias repetidas como (count, byte)
  - Máximo 255 repeticiones por run (se divide si es mayor)
  - Implementación completa y funcional

### Algoritmos de Encriptación

#### Implementación Principal (worker.cpp)
- ⚠️ **XOR simple**: 
  - Aplica XOR byte a byte con la clave
  - La clave se repite cíclicamente
  - Funcional pero muy básico (no es seguro para producción)

#### Implementación Alternativa (gsea.cpp - NO INTEGRADA)
- ✅ **XOR simple**: Misma implementación que la principal

**❌ NO HAY:** AES, Vigenère, u otros algoritmos más robustos mencionados en los TODOs.

---

## 3. LLAMADAS AL SISTEMA

### ✅ Se están usando LLAMADAS DIRECTAS AL SISTEMA (POSIX)

**NO se usa `stdio.h` (fopen, fread, fwrite, fclose).**  
**SÍ se usan llamadas POSIX directas:**

#### Llamadas al Sistema Utilizadas:

**En `file_manager.cpp`:**
- `stat()` - Obtener información de archivos/directorios
- `open()` - Abrir archivos (O_RDONLY, O_WRONLY | O_CREAT | O_TRUNC)
- `read()` - Leer datos de archivos
- `write()` - Escribir datos a archivos
- `close()` - Cerrar descriptores de archivo
- `opendir()` - Abrir directorios
- `readdir()` - Leer entradas de directorio
- `closedir()` - Cerrar directorios

**En `gsea.cpp`:**
- `open()` - Abrir archivos
- `read()` - Leer datos
- `write()` - Escribir datos
- `close()` - Cerrar descriptores
- `stat()` - Verificar tipos de archivo
- `opendir()` - Abrir directorios
- `readdir()` - Leer entradas
- `closedir()` - Cerrar directorios

**En `utils.cpp`:**
- `read()` - En `safe_read_loop()`
- `write()` - En `safe_write_loop()`

**Headers POSIX incluidos:**
- `<fcntl.h>` - Para flags O_RDONLY, O_WRONLY, O_CREAT, O_TRUNC
- `<unistd.h>` - Para read(), write(), close()
- `<sys/stat.h>` - Para stat(), S_ISDIR(), S_ISREG()
- `<dirent.h>` - Para opendir(), readdir(), closedir()
- `<errno.h>` - Para manejo de errores

**✅ CUMPLE CON EL REQUISITO:** El proyecto usa llamadas directas al sistema POSIX en lugar de stdio.h.

---

## 4. CONCURRENCIA

### ✅ CONCURRENCIA IMPLEMENTADA

#### Librería Utilizada

**En `main.cpp` (implementación principal):**
- ✅ **pthreads** (`<pthread.h>`)
- Se crea **un hilo por archivo** encontrado
- Usa `pthread_create()` y `pthread_join()`

**En `gsea.cpp` (implementación alternativa NO INTEGRADA):**
- ✅ **std::thread** (C++11)
- También crea un hilo por archivo
- Usa `std::thread` y `join()`

#### Manejo de Múltiples Archivos

**En `main.cpp`:**
1. `list_input_files()` obtiene todos los archivos (recursivo si es directorio)
2. Se crea un vector de `pthread_t` con un hilo por archivo
3. Cada hilo ejecuta `worker_entry()` con sus propios `WorkerArgs`
4. Todos los hilos se esperan con `pthread_join()`
5. Procesamiento **paralelo** de múltiples archivos

**En `gsea.cpp`:**
1. `list_files_recursive()` obtiene archivos recursivamente
2. Se crea un vector de `std::thread`
3. Cada hilo procesa un archivo independientemente
4. Todos los hilos se esperan con `join()`

#### Sincronización

**En `utils.cpp`:**
- ✅ **Mutex para logging**: `pthread_mutex_t log_mutex`
- Las funciones `log_info()` y `log_error()` son **thread-safe**
- Protegen las operaciones de escritura a stdout/stderr

**⚠️ NOTA:** No hay sincronización adicional porque cada hilo trabaja con archivos independientes (no hay recursos compartidos más allá del logging).

#### Configuración del Makefile
- ✅ Incluye flag `-pthread` en CXXFLAGS para enlazar la librería pthread

**✅ CUMPLE CON EL REQUISITO:** Concurrencia implementada con 1 hilo por archivo.

---

## 5. ESTADO DE COMPILACIÓN

### Análisis del Makefile

**Configuración:**
```makefile
CXX := g++
CXXFLAGS := -std=c++17 -O2 -Wall -Wextra -pthread
SRCS := $(wildcard src/*.cpp)
OBJS := $(SRCS:src/%.cpp=build/%.o)
BIN := bin/gsea
```

**Problemas Potenciales Detectados:**

1. **❌ Falta `-Iinclude`**: El Makefile no incluye el directorio de headers en los flags de compilación. Los archivos usan rutas relativas (`../include/`), lo que funciona pero no es ideal.

2. **⚠️ Compila TODOS los .cpp**: Incluye `gsea.cpp` aunque no se use en `main.cpp`, lo que podría causar:
   - Símbolos duplicados si hay funciones con el mismo nombre
   - Aumento innecesario del tamaño del binario

3. **✅ Flags de compilación adecuados**: 
   - `-Wall -Wextra` para warnings
   - `-O2` para optimización
   - `-pthread` para threading

### Verificación de Compilación

**No se pudo compilar en Windows** (el entorno es PowerShell, no bash/Linux), pero el código está diseñado para Linux/Unix (usa POSIX).

**Análisis Estático de Errores Potenciales:**

1. **Includes relativos**: Funcionan porque los archivos están en `src/` y los headers en `include/`, pero sería mejor usar `-Iinclude`.

2. **Código duplicado**: 
   - `gsea.cpp` tiene funciones similares a `file_manager.cpp` (list_files_recursive vs list_input_files)
   - Podría haber conflictos de nombres

3. **Manejo de errores**: Muchos TODOs en el código indican manejo de errores incompleto:
   - `file_manager.cpp`: "TODO: better error handling/reporting"
   - `file_manager.cpp`: "TODO: propagate/handle errno"
   - `worker.cpp`: "TODO: implement RLE/Huffman/LZW"

### Warnings Probables

Si se compilara con `-Wall -Wextra`, posibles warnings:
- Variables no usadas (si `gsea.cpp` no se integra)
- Comparaciones signed/unsigned (ssize_t vs size_t)
- Posibles conversiones implícitas

### Estado General

**✅ ESTRUCTURA DE COMPILACIÓN:** Correcta, pero mejorable  
**⚠️ INTEGRACIÓN:** `gsea.cpp` no está integrado con `main.cpp`  
**❌ FUNCIONALIDAD:** Compresión real no implementada en la versión principal  

---

## RESUMEN EJECUTIVO

### ✅ Lo que FUNCIONA:
1. Estructura del proyecto bien organizada
2. Parsing de argumentos completo y funcional
3. Gestión de archivos y directorios (recursivo)
4. Llamadas directas al sistema POSIX (open, read, write, close, stat, opendir, etc.)
5. Concurrencia con pthreads (1 hilo por archivo)
6. Logging thread-safe
7. Encriptación XOR básica funcional

### ❌ Lo que NO FUNCIONA o está INCOMPLETO:
1. **Compresión real NO implementada** en la versión principal (solo placeholders)
2. **Descompresión real NO implementada** en la versión principal
3. `gsea.cpp` tiene implementaciones reales pero **NO está integrado** con `main.cpp`
4. Algoritmos avanzados (LZW, Huffman, AES) mencionados en TODOs pero no implementados
5. Manejo de errores incompleto (muchos TODOs)
6. Makefile no incluye `-Iinclude` (aunque funciona con rutas relativas)

### 🔧 Recomendaciones:
1. **Integrar `gsea.cpp` con `main.cpp`** o reemplazar los placeholders en `worker.cpp`
2. Agregar `-Iinclude` al Makefile
3. Completar el manejo de errores
4. Decidir si mantener `gsea.cpp` o eliminarlo si no se usa
5. Implementar algoritmos de compresión más avanzados (LZW, Huffman) si es requerido

---

**Fecha de Análisis:** $(date)  
**Versión del Proyecto:** Basado en estructura actual del repositorio

