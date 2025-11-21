#!/bin/bash

# Script de testing automático para GSEA
# Ejecuta pruebas de compresión, encriptación y operaciones combinadas

# No usar set -e porque queremos manejar errores manualmente en las pruebas

# Colores para output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Contador de pruebas
TESTS_PASSED=0
TESTS_FAILED=0
FAILED_TESTS=()

# Función para imprimir mensajes
print_info() {
    echo -e "${YELLOW}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[✓]${NC} $1"
}

print_error() {
    echo -e "${RED}[✗]${NC} $1"
}

# Función para ejecutar una prueba
run_test() {
    local test_name="$1"
    local test_command="$2"
    
    print_info "Ejecutando: $test_name"
    
    if eval "$test_command"; then
        print_success "$test_name - PASÓ"
        ((TESTS_PASSED++))
        return 0
    else
        print_error "$test_name - FALLÓ"
        ((TESTS_FAILED++))
        FAILED_TESTS+=("$test_name")
        return 1
    fi
}

# Función para limpiar archivos de prueba
cleanup_test_files() {
    print_info "Limpiando archivos de prueba..."
    rm -f tests/data/test.txt
    rm -f tests/data/test.rle
    rm -f tests/data/test_restored.txt
    rm -f tests/data/test.enc
    rm -f tests/data/test_restored2.txt
    rm -f tests/data/test.ce
    rm -f tests/data/test_restored3.txt
    rm -rf tests/data/dir_test
    rm -rf tests/data/dir_test_out
}

# Limpiar archivos de pruebas anteriores
cleanup_test_files

# Compilar el proyecto
print_info "Compilando el proyecto..."
if make clean && make; then
    print_success "Compilación exitosa"
else
    print_error "Error en la compilación"
    exit 1
fi

# Verificar que el binario existe
if [ ! -f "./bin/gsea" ]; then
    print_error "El binario ./bin/gsea no existe después de la compilación"
    exit 1
fi

print_info "Iniciando pruebas..."
echo ""

# Preparar directorio de datos de prueba
mkdir -p tests/data

# Crear archivo de prueba con contenido conocido
print_info "Creando archivo de prueba..."
cat > tests/data/test.txt << 'EOF'
Este es un archivo de prueba para GSEA.
Contiene texto repetido: AAAA BBBB CCCC
Y también variaciones: 1234567890
El objetivo es probar compresión y encriptación.
EOF

# PRUEBA 1: Compresión y descompresión RLE
echo "=========================================="
print_info "PRUEBA 1: Compresión y descompresión RLE"
run_test "Compresión RLE" "./bin/gsea --compress --input tests/data/test.txt --output tests/data/test.rle"
run_test "Descompresión RLE" "./bin/gsea --decompress --input tests/data/test.rle --output tests/data/test_restored.txt"
run_test "Verificación RLE (diff)" "diff tests/data/test.txt tests/data/test_restored.txt"
echo ""

# PRUEBA 2: Compresión y descompresión Differential
echo "=========================================="
print_info "PRUEBA 2: Compresión y descompresión Differential"
run_test "Compresión Differential" "./bin/gsea --compress --comp-alg diff --input tests/data/test.txt --output tests/data/test.diff"
run_test "Descompresión Differential" "./bin/gsea --decompress --comp-alg diff --input tests/data/test.diff --output tests/data/test_restored_diff.txt"
run_test "Verificación Differential (diff)" "diff tests/data/test.txt tests/data/test_restored_diff.txt"
rm -f tests/data/test.diff tests/data/test_restored_diff.txt
echo ""

# PRUEBA 3: Encriptación y desencriptación Vigenère
echo "=========================================="
print_info "PRUEBA 3: Encriptación y desencriptación Vigenère"
run_test "Encriptación Vigenère" "./bin/gsea --encrypt --key 'secreto123' --input tests/data/test.txt --output tests/data/test.enc"
run_test "Desencriptación Vigenère" "./bin/gsea --decrypt --key 'secreto123' --input tests/data/test.enc --output tests/data/test_restored2.txt"
run_test "Verificación Vigenère (diff)" "diff tests/data/test.txt tests/data/test_restored2.txt"
echo ""

# PRUEBA 4: Encriptación y desencriptación XOR
echo "=========================================="
print_info "PRUEBA 4: Encriptación y desencriptación XOR"
run_test "Encriptación XOR" "./bin/gsea --encrypt --enc-alg xor --key 'secreto123' --input tests/data/test.txt --output tests/data/test_xor.enc"
run_test "Desencriptación XOR" "./bin/gsea --decrypt --enc-alg xor --key 'secreto123' --input tests/data/test_xor.enc --output tests/data/test_restored_xor.txt"
run_test "Verificación XOR (diff)" "diff tests/data/test.txt tests/data/test_restored_xor.txt"
rm -f tests/data/test_xor.enc tests/data/test_restored_xor.txt
echo ""

# PRUEBA 5: Compresión + Encriptación, luego Desencriptación + Descompresión
echo "=========================================="
print_info "PRUEBA 5: Compresión RLE + Encriptación Vigenère (combinado)"
run_test "Compresión + Encriptación" "./bin/gsea --compress --encrypt --key 'clave' --input tests/data/test.txt --output tests/data/test.ce"
run_test "Desencriptación + Descompresión" "./bin/gsea --decrypt --decompress --key 'clave' --input tests/data/test.ce --output tests/data/test_restored3.txt"
run_test "Verificación combinado (diff)" "diff tests/data/test.txt tests/data/test_restored3.txt"
echo ""

# PRUEBA 6: Compresión Differential + Encriptación XOR
echo "=========================================="
print_info "PRUEBA 6: Compresión Differential + Encriptación XOR (combinado)"
run_test "Compresión Diff + Encriptación XOR" "./bin/gsea --compress --comp-alg diff --encrypt --enc-alg xor --key 'clave' --input tests/data/test.txt --output tests/data/test.diff_xor"
run_test "Desencriptación XOR + Descompresión Diff" "./bin/gsea --decrypt --enc-alg xor --decompress --comp-alg diff --key 'clave' --input tests/data/test.diff_xor --output tests/data/test_restored4.txt"
run_test "Verificación combinado Diff+XOR (diff)" "diff tests/data/test.txt tests/data/test_restored4.txt"
rm -f tests/data/test.diff_xor tests/data/test_restored4.txt
echo ""

# PRUEBA 7: Procesamiento de directorio completo
echo "=========================================="
print_info "PRUEBA 7: Procesamiento de directorio completo"
mkdir -p tests/data/dir_test
cp tests/data/test.txt tests/data/dir_test/file1.txt
cp tests/data/test.txt tests/data/dir_test/file2.txt
echo "Archivo adicional" > tests/data/dir_test/file3.txt

run_test "Procesar directorio (compresión)" "./bin/gsea --compress --input tests/data/dir_test/ --output tests/data/dir_test_out/"
if [ -f "tests/data/dir_test_out/file1.txt" ] && [ -f "tests/data/dir_test_out/file2.txt" ] && [ -f "tests/data/dir_test_out/file3.txt" ]; then
    print_success "Todos los archivos del directorio fueron procesados"
    ((TESTS_PASSED++))
else
    print_error "No todos los archivos del directorio fueron procesados"
    ((TESTS_FAILED++))
    FAILED_TESTS+=("Procesamiento de directorio")
fi
echo ""

# PRUEBA 8: Validación de errores
echo "=========================================="
print_info "PRUEBA 8: Validación de errores"
# Probar sin clave cuando se requiere
if ./bin/gsea --encrypt --input tests/data/test.txt --output tests/data/test_no_key.enc 2>&1 | grep -q "requires a key"; then
    print_success "Validación de clave requerida - PASÓ"
    ((TESTS_PASSED++))
else
    print_error "Validación de clave requerida - FALLÓ"
    ((TESTS_FAILED++))
    FAILED_TESTS+=("Validación de clave")
fi

# Probar algoritmo desconocido
if ./bin/gsea --compress --comp-alg unknown --input tests/data/test.txt --output tests/data/test_unknown.xxx 2>&1 | grep -q "Unknown compression algorithm"; then
    print_success "Validación de algoritmo desconocido - PASÓ"
    ((TESTS_PASSED++))
else
    print_error "Validación de algoritmo desconocido - FALLÓ"
    ((TESTS_FAILED++))
    FAILED_TESTS+=("Validación de algoritmo")
fi

rm -f tests/data/test_no_key.enc tests/data/test_unknown.xxx
echo ""

# Resumen final
echo "=========================================="
echo ""
print_info "RESUMEN DE PRUEBAS:"
echo "  Pruebas pasadas: $TESTS_PASSED"
echo "  Pruebas fallidas: $TESTS_FAILED"
echo ""

if [ $TESTS_FAILED -eq 0 ]; then
    print_success "✅ Todas las pruebas pasaron ($TESTS_PASSED pruebas)"
    echo ""
    cleanup_test_files
    exit 0
else
    print_error "❌ Algunas pruebas fallaron:"
    for test in "${FAILED_TESTS[@]}"; do
        echo "  - $test"
    done
    echo ""
    cleanup_test_files
    exit 1
fi

