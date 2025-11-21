# GUÍA COMPLETA DE COMPILACIÓN Y PRUEBAS - GSEA

## ÍNDICE
1. [Compilación](#1-compilación)
2. [Pruebas Básicas Manuales](#2-pruebas-básicas-manuales)
3. [Pruebas Automáticas](#3-pruebas-automáticas)
4. [Verificación de Requisitos](#4-verificación-de-requisitos)
5. [Pruebas de Casos Edge](#5-pruebas-de-casos-edge)
6. [Comandos de Limpieza](#6-comandos-de-limpieza)

---

## 1. COMPILACIÓN

### 1.1 Compilación desde cero

**Comando:**
```bash
cd /ruta/al/proyecto/EntregaFinal-SistemasOperativos
make clean
make
```

**Output esperado:**
```
rm -rf build bin/gsea
mkdir -p build
g++ -std=c++17 -O2 -Wall -Wextra -pthread -Iinclude -c src/main.cpp -o build/main.o
g++ -std=c++17 -O2 -Wall -Wextra -pthread -Iinclude -c src/cli.cpp -o build/cli.o
g++ -std=c++17 -O2 -Wall -Wextra -pthread -Iinclude -c src/file_manager.cpp -o build/file_manager.o
g++ -std=c++17 -O2 -Wall -Wextra -pthread -Iinclude -c src/worker.cpp -o build/worker.o
g++ -std=c++17 -O2 -Wall -Wextra -pthread -Iinclude -c src/utils.cpp -o build/utils.o
mkdir -p bin
g++ -std=c++17 -O2 -Wall -Wextra -pthread -Iinclude -o bin/gsea build/main.o build/cli.o build/file_manager.o build/worker.o build/utils.o
```

**Interpretación:**
- ✅ **Éxito**: No hay mensajes de error, se crea `bin/gsea`
- ❌ **Error**: Aparecen mensajes de error en rojo

### 1.2 Verificar que compiló correctamente

**Comando:**
```bash
ls -lh bin/gsea
file bin/gsea
```

**Output esperado:**
```
-rwxr-xr-x 1 user user 123K fecha hora bin/gsea
bin/gsea: ELF 64-bit LSB executable, x86-64, version 1 (SYSV), dynamically linked, ...
```

**Interpretación:**
- ✅ **Éxito**: El archivo existe y es ejecutable
- ❌ **Error**: "No such file or directory" significa que la compilación falló

**Comando alternativo (verificar versión):**
```bash
./bin/gsea --help 2>&1 | head -5
```

**Output esperado:**
```
[ERROR] input path required
```

**Interpretación:** El binario funciona (aunque muestre error por falta de argumentos).

### 1.3 Solución de errores de compilación

**Error común 1: "No such file or directory" en includes**
```bash
# Verificar que existe el directorio include
ls -la include/
# Debe mostrar: cli.h, file_manager.h, utils.h, worker.h
```

**Error común 2: "pthread not found"**
```bash
# En Ubuntu/Debian:
sudo apt-get install build-essential

# Verificar que pthread está disponible:
g++ -pthread --version
```

**Error común 3: "getopt.h not found" (Windows)**
- **Solución**: Este proyecto requiere Linux/Unix. Usa WSL, Docker, o máquina virtual Linux.

**Error común 4: Warnings de compilación**
- Los warnings no impiden la ejecución, pero revisa:
```bash
make clean
make 2>&1 | grep -i warning
```

---

## 2. PRUEBAS BÁSICAS MANUALES

### 2.1 Crear archivo de prueba

**Comando:**
```bash
mkdir -p tests/data
cat > tests/data/sample.txt << 'EOF'
Este es un archivo de prueba para GSEA.
Contiene texto repetido: AAAA BBBB CCCC DDDD
Y también números: 1234567890
El objetivo es probar compresión y encriptación.
Este archivo tiene suficiente contenido para ver resultados.
EOF
```

**Verificar creación:**
```bash
cat tests/data/sample.txt
wc -l tests/data/sample.txt
```

**Output esperado:**
```
Este es un archivo de prueba para GSEA.
...
5 tests/data/sample.txt
```

### 2.2 Prueba de SOLO compresión RLE

**Paso 1: Comprimir**
```bash
./bin/gsea --compress --input tests/data/sample.txt --output tests/data/sample.rle
```

**Output esperado:**
```
[INFO] Worker starting for file: tests/data/sample.txt
[INFO] File 'tests/data/sample.txt': Compressed 123 bytes to 89 bytes (rle)
[INFO] File 'tests/data/sample.txt': Successfully processed and written to tests/data/sample.rle
[INFO] All workers finished
```

**Interpretación:**
- ✅ **Éxito**: Muestra ratio de compresión (ej: 123 → 89 bytes)
- ❌ **Error**: Mensaje de error en rojo

**Paso 2: Verificar archivo comprimido**
```bash
ls -lh tests/data/sample.rle
file tests/data/sample.rle
```

**Output esperado:**
```
-rw-r--r-- 1 user user 89 fecha hora tests/data/sample.rle
tests/data/sample.rle: data
```

**Paso 3: Descomprimir**
```bash
./bin/gsea --decompress --input tests/data/sample.rle --output tests/data/sample_restored.txt
```

**Output esperado:**
```
[INFO] Worker starting for file: tests/data/sample.rle
[INFO] File 'tests/data/sample.rle': Decompressed 89 bytes to 123 bytes (rle)
[INFO] File 'tests/data/sample.rle': Successfully processed and written to tests/data/sample_restored.txt
```

**Paso 4: Verificar que funcionó (comparar archivos)**
```bash
diff tests/data/sample.txt tests/data/sample_restored.txt
echo $?
```

**Output esperado:**
```
(ninguna salida)
0
```

**Interpretación:**
- ✅ **Éxito**: `diff` no muestra diferencias y retorna código 0
- ❌ **Error**: `diff` muestra diferencias o retorna código != 0

**Verificación alternativa:**
```bash
md5sum tests/data/sample.txt tests/data/sample_restored.txt
```

**Output esperado (ambos deben ser iguales):**
```
a1b2c3d4e5f6...  tests/data/sample.txt
a1b2c3d4e5f6...  tests/data/sample_restored.txt
```

### 2.3 Prueba de SOLO encriptación Vigenère

**Paso 1: Encriptar**
```bash
./bin/gsea --encrypt --key "mi_clave_secreta" --input tests/data/sample.txt --output tests/data/sample.enc
```

**Output esperado:**
```
[INFO] Worker starting for file: tests/data/sample.txt
[INFO] File 'tests/data/sample.txt': Encrypted 123 bytes using vigenere cipher
[INFO] File 'tests/data/sample.txt': Successfully processed and written to tests/data/sample.enc
```

**Paso 2: Verificar archivo encriptado (debe ser diferente)**
```bash
diff tests/data/sample.txt tests/data/sample.enc
echo $?
```

**Output esperado:**
```
(mostrará diferencias - esto es correcto)
1
```

**Interpretación:**
- ✅ **Éxito**: Los archivos son diferentes (encriptación funcionó)
- ❌ **Error**: Si son idénticos, la encriptación no funcionó

**Paso 3: Desencriptar**
```bash
./bin/gsea --decrypt --key "mi_clave_secreta" --input tests/data/sample.enc --output tests/data/sample_decrypted.txt
```

**Output esperado:**
```
[INFO] Worker starting for file: tests/data/sample.enc
[INFO] File 'tests/data/sample.enc': Decrypted 123 bytes using vigenere cipher
[INFO] File 'tests/data/sample.enc': Successfully processed and written to tests/data/sample_decrypted.txt
```

**Paso 4: Verificar que funcionó**
```bash
diff tests/data/sample.txt tests/data/sample_decrypted.txt
echo $?
```

**Output esperado:**
```
(ninguna salida)
0
```

**Interpretación:**
- ✅ **Éxito**: Archivos idénticos después de encriptar/desencriptar
- ❌ **Error**: Si hay diferencias, la desencriptación falló

### 2.4 Prueba de compresión + encriptación combinadas

**Paso 1: Comprimir Y encriptar**
```bash
./bin/gsea --compress --encrypt --key "clave123" --input tests/data/sample.txt --output tests/data/sample.ce
```

**Output esperado:**
```
[INFO] Worker starting for file: tests/data/sample.txt
[INFO] File 'tests/data/sample.txt': Compressed 123 bytes to 89 bytes (rle)
[INFO] File 'tests/data/sample.txt': Encrypted 89 bytes using vigenere cipher
[INFO] File 'tests/data/sample.txt': Successfully processed and written to tests/data/sample.ce
```

**Interpretación:**
- ✅ **Éxito**: Muestra primero compresión, luego encriptación
- ✅ **Orden correcto**: Compresión → Encriptación

**Paso 2: Desencriptar Y descomprimir**
```bash
./bin/gsea --decrypt --decompress --key "clave123" --input tests/data/sample.ce --output tests/data/sample_restored_ce.txt
```

**Output esperado:**
```
[INFO] Worker starting for file: tests/data/sample.ce
[INFO] File 'tests/data/sample.ce': Decrypted 89 bytes using vigenere cipher
[INFO] File 'tests/data/sample.ce': Decompressed 89 bytes to 123 bytes (rle)
[INFO] File 'tests/data/sample.ce': Successfully processed and written to tests/data/sample_restored_ce.txt
```

**Interpretación:**
- ✅ **Éxito**: Muestra primero desencriptación, luego descompresión
- ✅ **Orden correcto**: Desencriptación → Descompresión

**Paso 3: Verificar**
```bash
diff tests/data/sample.txt tests/data/sample_restored_ce.txt
echo $?
```

**Output esperado:**
```
(ninguna salida)
0
```

### 2.5 Prueba con DIRECTORIO completo

**Paso 1: Crear directorio con múltiples archivos**
```bash
mkdir -p tests/data/dir_test
echo "Archivo 1" > tests/data/dir_test/file1.txt
echo "Archivo 2 con contenido diferente" > tests/data/dir_test/file2.txt
echo "Archivo 3" > tests/data/dir_test/file3.txt
cp tests/data/sample.txt tests/data/dir_test/sample_copy.txt
```

**Verificar:**
```bash
ls -la tests/data/dir_test/
```

**Output esperado:**
```
total 20
drwxr-xr-x 2 user user 4096 fecha hora .
drwxr-xr-x 3 user user 4096 fecha hora ..
-rw-r--r-- 1 user user   10 fecha hora file1.txt
-rw-r--r-- 1 user user   35 fecha hora file2.txt
-rw-r--r-- 1 user user   10 fecha hora file3.txt
-rw-r--r-- 1 user user  123 fecha hora sample_copy.txt
```

**Paso 2: Procesar todo el directorio**
```bash
./bin/gsea --compress --input tests/data/dir_test/ --output tests/data/dir_test_out/
```

**Output esperado:**
```
[INFO] Found 4 file(s) to process
[INFO] Worker starting for file: tests/data/dir_test/file1.txt
[INFO] Worker starting for file: tests/data/dir_test/file2.txt
[INFO] Worker starting for file: tests/data/dir_test/file3.txt
[INFO] Worker starting for file: tests/data/dir_test/sample_copy.txt
[INFO] File 'tests/data/dir_test/file1.txt': Compressed X bytes to Y bytes (rle)
[INFO] File 'tests/data/dir_test/file2.txt': Compressed X bytes to Y bytes (rle)
[INFO] File 'tests/data/dir_test/file3.txt': Compressed X bytes to Y bytes (rle)
[INFO] File 'tests/data/dir_test/sample_copy.txt': Compressed X bytes to Y bytes (rle)
[INFO] Processing complete: 4 file(s) processed successfully, 0 file(s) failed
```

**Interpretación:**
- ✅ **Éxito**: Muestra "X file(s) processed successfully"
- ✅ **Concurrencia**: Múltiples "Worker starting" aparecen casi simultáneamente

**Paso 3: Verificar que todos los archivos se procesaron**
```bash
ls -la tests/data/dir_test_out/
```

**Output esperado:**
```
-rw-r--r-- 1 user user  X fecha hora file1.txt
-rw-r--r-- 1 user user  X fecha hora file2.txt
-rw-r--r-- 1 user user  X fecha hora file3.txt
-rw-r--r-- 1 user user  X fecha hora sample_copy.txt
```

**Verificar que son archivos comprimidos:**
```bash
file tests/data/dir_test_out/*
```

**Output esperado:**
```
tests/data/dir_test_out/file1.txt: data
tests/data/dir_test_out/file2.txt: data
...
```

### 2.6 Prueba de CONCURRENCIA

**Paso 1: Crear múltiples archivos grandes**
```bash
mkdir -p tests/data/concurrency_test
for i in {1..10}; do
    head -c 10000 /dev/urandom > tests/data/concurrency_test/file$i.bin
done
```

**Paso 2: Procesar y medir tiempo**
```bash
time ./bin/gsea --compress --input tests/data/concurrency_test/ --output tests/data/concurrency_test_out/
```

**Output esperado:**
```
[INFO] Found 10 file(s) to process
[INFO] Worker starting for file: tests/data/concurrency_test/file1.bin
[INFO] Worker starting for file: tests/data/concurrency_test/file2.bin
...
[INFO] Processing complete: 10 file(s) processed successfully, 0 file(s) failed

real    0m0.123s
user    0m0.456s
sys     0m0.012s
```

**Interpretación:**
- ✅ **Concurrencia**: `user` time > `real` time indica procesamiento paralelo
- ✅ **Múltiples hilos**: Los logs aparecen en orden no secuencial

**Paso 3: Verificar con htop/ps**
```bash
# En otra terminal, mientras se ejecuta:
htop -p $(pgrep gsea)
# O:
ps aux | grep gsea
```

**Output esperado:**
```
user  12345  0.5  0.1  gsea (múltiples procesos/hilos)
```

**Paso 4: Ver logs de procesamiento paralelo**
```bash
./bin/gsea --compress --input tests/data/concurrency_test/ --output tests/data/concurrency_test_out/ 2>&1 | grep "Worker starting" | wc -l
```

**Output esperado:**
```
10
```

**Interpretación:**
- ✅ **Éxito**: Muestra 10 "Worker starting" (uno por archivo)

---

## 3. PRUEBAS AUTOMÁTICAS

### 3.1 Ejecutar script de pruebas automáticas

**Comando:**
```bash
chmod +x tests/run_tests.sh
./tests/run_tests.sh
```

**O sin hacer ejecutable:**
```bash
bash tests/run_tests.sh
```

### 3.2 Output esperado si todo está bien

```
[INFO] Compilando el proyecto...
[✓] Compilación exitosa
[INFO] Iniciando pruebas...

==========================================
[INFO] PRUEBA 1: Compresión y descompresión RLE
[INFO] Ejecutando: Compresión RLE
[✓] Compresión RLE - PASÓ
[INFO] Ejecutando: Descompresión RLE
[✓] Descompresión RLE - PASÓ
[INFO] Ejecutando: Verificación RLE (diff)
[✓] Verificación RLE (diff) - PASÓ

==========================================
[INFO] PRUEBA 2: Compresión y descompresión Differential
...

==========================================
[INFO] RESUMEN DE PRUEBAS:
  Pruebas pasadas: 15
  Pruebas fallidas: 0

[✓] ✅ Todas las pruebas pasaron (15 pruebas)
```

### 3.3 Archivos generados por el script

**Durante las pruebas:**
```bash
tests/data/test.txt          # Archivo de prueba original
tests/data/test.rle          # Archivo comprimido RLE
tests/data/test_restored.txt # Archivo descomprimido
tests/data/test.enc          # Archivo encriptado
tests/data/test.ce           # Archivo comprimido + encriptado
tests/data/dir_test/         # Directorio de prueba
tests/data/dir_test_out/     # Directorio de salida
```

**Después de las pruebas:**
- ✅ Todos los archivos temporales son eliminados automáticamente

**Verificar limpieza:**
```bash
ls tests/data/
```

**Output esperado:**
```
sample.txt  (solo archivos que creaste manualmente)
```

---

## 4. VERIFICACIÓN DE REQUISITOS

### 4.1 Confirmar uso de llamadas al sistema POSIX (no stdio.h)

**Comando:**
```bash
grep -r "fopen\|fread\|fwrite\|fclose" src/ include/
```

**Output esperado:**
```
(ninguna salida)
```

**Interpretación:**
- ✅ **Éxito**: No encuentra funciones de stdio.h
- ❌ **Error**: Si encuentra, está usando stdio.h en lugar de POSIX

**Verificar que usa POSIX:**
```bash
grep -r "open\|read\|write\|close" src/file_manager.cpp | head -5
```

**Output esperado:**
```cpp
int fd = open(path.c_str(), O_RDONLY);
while ((r = read(fd, buf, CHUNK)) > 0) {
ssize_t w = write(fd, ptr + written, total - written);
close(fd);
```

**Interpretación:**
- ✅ **Éxito**: Usa `open()`, `read()`, `write()`, `close()` (POSIX)
- ❌ **Error**: Si usa `fopen()`, `fread()`, etc. (stdio.h)

**Verificar includes:**
```bash
grep "#include" src/file_manager.cpp
```

**Output esperado:**
```cpp
#include <fcntl.h>    # Para open()
#include <unistd.h>   # Para read(), write(), close()
#include <sys/stat.h> # Para stat()
#include <dirent.h>   # Para opendir(), readdir()
```

### 4.2 Confirmar uso de pthreads para concurrencia

**Comando:**
```bash
grep -r "pthread" src/ include/ Makefile
```

**Output esperado:**
```cpp
// src/main.cpp
#include <pthread.h>
pthread_create(&threads[i], nullptr, worker_entry, args[i]);
pthread_join(threads[i], nullptr);

// src/utils.cpp
#include <pthread.h>
pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_lock(&log_mutex);

// Makefile
CXXFLAGS := ... -pthread
```

**Interpretación:**
- ✅ **Éxito**: Usa `pthread_create()`, `pthread_join()`, mutex
- ❌ **Error**: Si usa `std::thread` o `fork()`

**Verificar enlazado:**
```bash
ldd bin/gsea | grep pthread
```

**Output esperado:**
```
libpthread.so.0 => /lib/x86_64-linux-gnu/libpthread.so.0
```

**O:**
```bash
nm bin/gsea | grep pthread
```

**Output esperado:**
```
U pthread_create
U pthread_join
U pthread_mutex_lock
```

### 4.3 Confirmar que los algoritmos son propios (no librerías externas)

**Comando:**
```bash
grep -r "zlib\|openssl\|crypto\|compression" src/ include/ Makefile
```

**Output esperado:**
```
(ninguna salida o solo comentarios)
```

**Verificar implementaciones propias:**
```bash
grep -A 5 "compress_rle\|encrypt_vigenere" src/worker.cpp
```

**Output esperado:**
```cpp
static std::vector<uint8_t> compress_rle(const std::vector<uint8_t> &in) {
    // Implementación propia aquí
    ...
}
```

**Interpretación:**
- ✅ **Éxito**: Funciones `static` con implementación visible en el código
- ❌ **Error**: Si llama a funciones de librerías externas

**Verificar dependencias del binario:**
```bash
ldd bin/gsea
```

**Output esperado:**
```
linux-vdso.so.1
libpthread.so.0
libstdc++.so.6
libgcc_s.so.1
libc.so.6
```

**Interpretación:**
- ✅ **Éxito**: Solo librerías estándar del sistema (pthread, stdc++, libc)
- ❌ **Error**: Si aparece `libz.so`, `libcrypto.so`, etc.

---

## 5. PRUEBAS DE CASOS EDGE

### 5.1 Archivo vacío

**Comando:**
```bash
touch tests/data/empty.txt
./bin/gsea --compress --input tests/data/empty.txt --output tests/data/empty.rle
```

**Output esperado:**
```
[INFO] Worker starting for file: tests/data/empty.txt
[INFO] File 'tests/data/empty.txt' is empty, skipping processing
[INFO] File 'tests/data/empty.txt': Successfully processed and written to tests/data/empty.rle
```

**Verificar:**
```bash
ls -lh tests/data/empty.rle
```

**Output esperado:**
```
-rw-r--r-- 1 user user 0 fecha hora tests/data/empty.rle
```

**Interpretación:**
- ✅ **Éxito**: Maneja archivo vacío sin errores
- ❌ **Error**: Si muestra error o crashea

### 5.2 Archivo muy grande (>10MB)

**Comando:**
```bash
dd if=/dev/urandom of=tests/data/large.bin bs=1M count=15
./bin/gsea --compress --input tests/data/large.bin --output tests/data/large.rle
```

**Output esperado:**
```
[INFO] Worker starting for file: tests/data/large.bin
[INFO] File 'tests/data/large.bin': Compressed 15728640 bytes to X bytes (rle)
[INFO] File 'tests/data/large.bin': Successfully processed and written to tests/data/large.rle
```

**Verificar memoria:**
```bash
# Mientras se ejecuta, en otra terminal:
free -h
```

**Interpretación:**
- ✅ **Éxito**: Procesa archivo grande sin problemas
- ❌ **Error**: Si se queda sin memoria o crashea

**Limpieza:**
```bash
rm tests/data/large.bin tests/data/large.rle
```

### 5.3 Directorio vacío

**Comando:**
```bash
mkdir -p tests/data/empty_dir
./bin/gsea --compress --input tests/data/empty_dir/ --output tests/data/empty_dir_out/
```

**Output esperado:**
```
[INFO] Found 0 file(s) to process
[ERROR] No input files found for path: tests/data/empty_dir/
```

**Interpretación:**
- ✅ **Éxito**: Maneja directorio vacío con mensaje de error apropiado
- ❌ **Error**: Si crashea o no muestra mensaje

### 5.4 Clave incorrecta en desencriptación

**Paso 1: Encriptar con una clave**
```bash
./bin/gsea --encrypt --key "clave_correcta" --input tests/data/sample.txt --output tests/data/sample_wrong.enc
```

**Paso 2: Intentar desencriptar con clave incorrecta**
```bash
./bin/gsea --decrypt --key "clave_incorrecta" --input tests/data/sample_wrong.enc --output tests/data/sample_wrong_decrypted.txt
```

**Output esperado:**
```
[INFO] Worker starting for file: tests/data/sample_wrong.enc
[INFO] File 'tests/data/sample_wrong.enc': Decrypted 123 bytes using vigenere cipher
[INFO] File 'tests/data/sample_wrong.enc': Successfully processed and written to tests/data/sample_wrong_decrypted.txt
```

**Verificar que el resultado es incorrecto:**
```bash
diff tests/data/sample.txt tests/data/sample_wrong_decrypted.txt
echo $?
```

**Output esperado:**
```
(diferencias mostradas)
1
```

**Interpretación:**
- ✅ **Comportamiento esperado**: Desencripta sin error, pero el resultado es incorrecto
- ⚠️ **Nota**: Vigenère no valida la clave, solo desencripta. Esto es normal.

### 5.5 Archivo ya comprimido

**Paso 1: Comprimir archivo**
```bash
./bin/gsea --compress --input tests/data/sample.txt --output tests/data/sample.rle
```

**Paso 2: Intentar comprimir el archivo comprimido**
```bash
./bin/gsea --compress --input tests/data/sample.rle --output tests/data/sample.rle.rle
```

**Output esperado:**
```
[INFO] Worker starting for file: tests/data/sample.rle
[INFO] File 'tests/data/sample.rle': Compressed X bytes to Y bytes (rle)
```

**Interpretación:**
- ✅ **Éxito**: Permite comprimir archivo ya comprimido (puede comprimir más o menos)
- ⚠️ **Nota**: No hay validación de formato, esto es comportamiento esperado

---

## 6. COMANDOS DE LIMPIEZA

### 6.1 Limpiar archivos compilados

**Comando:**
```bash
make clean
```

**Output esperado:**
```
rm -rf build bin/gsea
```

**Verificar:**
```bash
ls -la build/ bin/gsea 2>&1
```

**Output esperado:**
```
ls: cannot access 'build/': No such file or directory
ls: cannot access 'bin/gsea': No such file or directory
```

**Limpieza completa (incluyendo binarios):**
```bash
make clean
rm -rf bin/
```

### 6.2 Limpiar archivos de prueba

**Limpiar archivos de prueba individuales:**
```bash
rm -f tests/data/*.rle tests/data/*.enc tests/data/*.ce
rm -f tests/data/*_restored*.txt
rm -f tests/data/*_decrypted*.txt
```

**Limpiar directorios de prueba:**
```bash
rm -rf tests/data/dir_test tests/data/dir_test_out
rm -rf tests/data/concurrency_test tests/data/concurrency_test_out
```

**Limpieza completa de tests/data (mantener sample.txt):**
```bash
find tests/data/ -type f ! -name "sample.txt" -delete
find tests/data/ -type d -empty -delete
```

**Limpieza total de tests/data:**
```bash
rm -rf tests/data/*
```

**Verificar limpieza:**
```bash
ls -la tests/data/
```

**Output esperado:**
```
total 8
drwxr-xr-x 2 user user 4096 fecha hora .
drwxr-xr-x 3 user user 4096 fecha hora ..
```

### 6.3 Script de limpieza completa

**Crear script de limpieza:**
```bash
cat > clean_all.sh << 'EOF'
#!/bin/bash
echo "Limpiando archivos compilados..."
make clean
rm -rf bin/

echo "Limpiando archivos de prueba..."
rm -rf tests/data/*.rle tests/data/*.enc tests/data/*.ce
rm -rf tests/data/*_restored*.txt tests/data/*_decrypted*.txt
rm -rf tests/data/dir_test* tests/data/concurrency_test*
rm -rf tests/data/empty* tests/data/large* tests/data/test*

echo "Limpieza completa."
EOF

chmod +x clean_all.sh
```

**Ejecutar:**
```bash
./clean_all.sh
```

---

## RESUMEN DE COMANDOS RÁPIDOS

### Compilación
```bash
make clean && make
```

### Prueba básica completa
```bash
./bin/gsea --compress --input tests/data/sample.txt --output tests/data/sample.rle
./bin/gsea --decompress --input tests/data/sample.rle --output tests/data/sample_restored.txt
diff tests/data/sample.txt tests/data/sample_restored.txt
```

### Pruebas automáticas
```bash
bash tests/run_tests.sh
```

### Limpieza
```bash
make clean
rm -rf tests/data/*.rle tests/data/*.enc
```

---

**Última actualización:** $(date)
**Versión del proyecto:** Basado en estructura actual

