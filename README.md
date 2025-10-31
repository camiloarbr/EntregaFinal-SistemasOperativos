# GSEA — Utilidad de Gestión Segura y Eficiente de Archivos

Proyecto para la asignatura Sistemas Operativos.
Objetivo: crear una CLI en C++ que comprima/descomprima y encripte/desencripte archivos y directorios usando llamadas al sistema POSIX y concurrencia (1 hilo por archivo). No usar librerías externas para compresión/encriptación.

Requisitos de desarrollo
- C++17
- Entorno POSIX (Linux, WSL, macOS). El código usa syscalls POSIX (`open`, `read`, `write`, `opendir`, `readdir`, etc.).

Estructura inicial
- `src/` — código fuente
- `include/` — cabeceras públicas
- `Makefile` — reglas de compilación
- `tests/` — casos de prueba básicos

Cómo compilar

En un entorno POSIX con g++ instalado:

```sh
make
```

Cómo usar (ejemplos)

- Comprimir un archivo:
  ./gsea compress -i input.txt -o input.txt.rle

- Descomprimir:
  ./gsea decompress -i input.txt.rle -o input_out.txt

- Encriptar con clave (simple XOR):
  ./gsea encrypt -i file -o file.enc -k mykey

- Desencriptar:
  ./gsea decrypt -i file.enc -o file.dec -k mykey

- Para operar recursivamente sobre un directorio (se lanza 1 hilo por archivo):
  ./gsea compress -i somedir -o somedir.rle

Notas
- Implementaciones de compresión/encriptación iniciales son sencillas (RLE y XOR) para cumplir la restricción de no usar bibliotecas externas. Son suficientes para la entrega y fáciles de ampliar.
- Tests básicos en `tests/`. Ejecutar `make test` después de `make`.

Siguientes pasos
1. Implementar mejoras en los algoritmos (por ejemplo, Huffman para compresión segura/eficiente).
2. Añadir más tests y casos límite.

Licencia: código de ejemplo para uso académico.
