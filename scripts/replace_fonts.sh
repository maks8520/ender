#!/bin/bash

# Находим все .cpp и .h файлы
FILES=$(find base_ui/src -name "*.cpp" -o -name "*.h")

for FILE in $FILES; do
  echo "Processing $FILE..."

  # Заменяем все встроенные шрифты на кириллические
  sed -i 's/lv_font_montserrat_10/montserrat_cyr_14/g' "$FILE"
  sed -i 's/lv_font_montserrat_12/montserrat_cyr_14/g' "$FILE"
  sed -i 's/lv_font_montserrat_14/montserrat_cyr_14/g' "$FILE"
  sed -i 's/lv_font_montserrat_16/montserrat_cyr_16/g' "$FILE"
  sed -i 's/lv_font_montserrat_18/montserrat_cyr_18/g' "$FILE"
  sed -i 's/lv_font_montserrat_20/montserrat_cyr_20/g' "$FILE"
  sed -i 's/lv_font_montserrat_22/montserrat_cyr_20/g' "$FILE"
  sed -i 's/lv_font_montserrat_24/montserrat_cyr_24/g' "$FILE"
  sed -i 's/lv_font_montserrat_26/montserrat_cyr_24/g' "$FILE"
  sed -i 's/lv_font_montserrat_28/montserrat_cyr_28/g' "$FILE"
  sed -i 's/lv_font_montserrat_30/montserrat_cyr_28/g' "$FILE"
  sed -i 's/lv_font_montserrat_32/montserrat_cyr_28/g' "$FILE"
  sed -i 's/lv_font_montserrat_34/montserrat_cyr_28/g' "$FILE"
  sed -i 's/lv_font_montserrat_36/montserrat_cyr_28/g' "$FILE"
  sed -i 's/lv_font_montserrat_38/montserrat_cyr_28/g' "$FILE"
  sed -i 's/lv_font_montserrat_40/montserrat_cyr_28/g' "$FILE"
  sed -i 's/lv_font_montserrat_42/montserrat_cyr_28/g' "$FILE"
  sed -i 's/lv_font_montserrat_44/montserrat_cyr_28/g' "$FILE"
  sed -i 's/lv_font_montserrat_46/montserrat_cyr_28/g' "$FILE"
  sed -i 's/lv_font_montserrat_48/montserrat_cyr_28/g' "$FILE"
done

echo "Font replacement complete!"
