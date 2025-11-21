GSEA es una utilidad de línea de comandos escrita en C++ para comprimir/descomprimir y encriptar/desencriptar archivos y directorios usando llamadas al sistema POSIX y concurrencia (1 hilo por archivo). Está diseñada como proyecto para la asignatura Sistemas Operativos y sirve de base para implementar algoritmos propios de compresión y criptografía sin depender de librerías externas.

## Uso

```
gsea [--compress|--decompress|--encrypt|--decrypt] --input <path> --output <path> [-k key] [--enc-alg algorithm]
```

### Opciones

- `--compress` / `-c`: Comprime archivos usando RLE (Run-Length Encoding)
- `--decompress` / `-d`: Descomprime archivos comprimidos con RLE
- `--encrypt` / `-e`: Encripta archivos
- `--decrypt` / `-r`: Desencripta archivos
- `--input <path>` / `-i <path>`: Ruta de entrada (archivo o directorio)
- `--output <path>` / `-o <path>`: Ruta de salida
- `--key <key>` / `-k <key>`: Clave para encriptación/desencriptación (requerida para -e/-r)
- `--enc-alg <algorithm>` / `-b <algorithm>`: Algoritmo de encriptación a usar

### Algoritmos de Encriptación

- `vigenere` (por defecto): Cifrado Vigenère - suma módulo 256 de cada byte con la clave
- `xor`: Cifrado XOR simple - operación XOR bit a bit con la clave

Ejemplos:
```bash
# Encriptar con Vigenère (por defecto)
gsea --encrypt --input file.txt --output file.enc -k "mykey"

# Encriptar con XOR
gsea --encrypt --input file.txt --output file.enc -k "mykey" --enc-alg xor

# Desencriptar (usa el mismo algoritmo que se usó para encriptar)
gsea --decrypt --input file.enc --output file.txt -k "mykey" --enc-alg xor
```

### Notas

- Si no se especifica `--enc-alg`, se usa Vigenère por defecto
- El algoritmo debe ser el mismo para encriptar y desencriptar
- La clave se repite cíclicamente si es más corta que los datos