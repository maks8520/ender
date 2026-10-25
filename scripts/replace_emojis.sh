#!/bin/bash

FILES=$(find base_ui/src -name "*.cpp" -o -name "*.h")

for FILE in $FILES; do
  echo "Processing $FILE..."

  # Заменяем эмодзи на символы LVGL
  sed -i 's/🔥/LV_SYMBOL_CHARGE/g' "$FILE"
  sed -i 's/🛏/LV_SYMBOL_HOME/g' "$FILE"
  sed -i 's/❄/LV_SYMBOL_CHARGE/g' "$FILE"
  sed -i 's/📊/LV_SYMBOL_LIST/g' "$FILE"
  sed -i 's/⚙️/LV_SYMBOL_SETTINGS/g' "$FILE"
  sed -i 's/⚙/LV_SYMBOL_SETTINGS/g' "$FILE"
  sed -i 's/⏸/LV_SYMBOL_PAUSE/g' "$FILE"
  sed -i 's/⏹/LV_SYMBOL_STOP/g' "$FILE"
  sed -i 's/▶/LV_SYMBOL_PLAY/g' "$FILE"
  sed -i 's/🔌/LV_SYMBOL_WARNING/g' "$FILE"
  sed -i 's/⚠️/LV_SYMBOL_WARNING/g' "$FILE"
  sed -i 's/⚠/LV_SYMBOL_WARNING/g' "$FILE"
done

echo "Emoji replacement complete!"
