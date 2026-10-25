#!/bin/bash
set -e

# Создаем директорию для шрифтов
mkdir -p base_ui/src/fonts

# Скачиваем Montserrat (поддерживает кириллицу) если отсутствует
if [ ! -f base_ui/src/fonts/Montserrat-Regular.ttf ]; then
  wget -O base_ui/src/fonts/Montserrat-Regular.ttf \
    "https://raw.githubusercontent.com/JulietaUla/Montserrat/master/fonts/ttf/Montserrat-Regular.ttf"
fi

# Диапазоны Unicode для кириллицы и нужных символов
# Basic Latin + Cyrillic + Degree symbol + General punctuation + Arrows
RANGES="0x20-0x7F,0x400-0x45F,0x490-0x49D,0x00B0-0x00B1,0x2000-0x206F,0x2190-0x2199"

# Генерируем шрифты разных размеров
for SIZE in 14 16 18 20 24 28; do
  echo "Generating Montserrat ${SIZE} with Cyrillic..."

  npx lv_font_conv \
    --font base_ui/src/fonts/Montserrat-Regular.ttf \
    --size ${SIZE} \
    --bpp 4 \
    --range ${RANGES} \
    --format lvgl \
    -o base_ui/src/fonts/montserrat_cyr_${SIZE}.c

  # Создаем заголовочный файл
  cat > base_ui/src/fonts/montserrat_cyr_${SIZE}.h << EOF
#ifndef MONTSERRAT_CYR_${SIZE}_H
#define MONTSERRAT_CYR_${SIZE}_H

#include "lvgl/lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const lv_font_t montserrat_cyr_${SIZE};

#ifdef __cplusplus
}
#endif

#endif
EOF
done

echo "All Cyrillic fonts generated successfully!"
