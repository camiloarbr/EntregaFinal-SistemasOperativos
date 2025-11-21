#!/bin/bash

echo "=== Verificación de Compresión RLE ==="
echo ""

echo "1. Tamaños de archivos:"
ls -lh tests/data/sample.txt tests/data/sample.rle
echo ""

echo "2. Contenido del archivo original:"
cat tests/data/sample.txt
echo ""

echo "3. Descomprimiendo archivo..."
./bin/gsea --decompress --input tests/data/sample.rle --output tests/data/sample_restored.txt

echo ""
echo "4. Comparando archivos original y restaurado:"
if diff tests/data/sample.txt tests/data/sample_restored.txt > /dev/null; then
    echo "✅ ÉXITO: Los archivos son idénticos"
    echo ""
    echo "5. Contenido del archivo restaurado:"
    cat tests/data/sample_restored.txt
else
    echo "❌ ERROR: Los archivos son diferentes"
    echo ""
    echo "Diferencias:"
    diff tests/data/sample.txt tests/data/sample_restored.txt
fi

echo ""
echo "6. Verificando formato RLE (debe ser par número de bytes):"
RLE_SIZE=$(stat -c%s tests/data/sample.rle 2>/dev/null || stat -f%z tests/data/sample.rle 2>/dev/null)
if [ $((RLE_SIZE % 2)) -eq 0 ]; then
    echo "✅ Formato correcto: $RLE_SIZE bytes (par)"
else
    echo "❌ Formato incorrecto: $RLE_SIZE bytes (impar, debería ser par)"
fi

