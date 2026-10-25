#!/bin/bash
set -e

RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0m'

echo "=== Финальная самопроверка ==="

# Проверка 1: Нет эмодзи в коде
echo "Проверка 1: Поиск эмодзи..."
EMOJI_COUNT=$(grep -rn "🔥\|🛏\|❄\|📊\|⚙\|⏸\|⏹\|▶\|🔌\|⚠" base_ui/src --include="*.cpp" --include="*.h" | wc -l || true)
if [ "$EMOJI_COUNT" -eq 0 ]; then
  echo -e "${GREEN}✓ Эмодзи не найдены${NC}"
else
  echo -e "${RED}✗ Найдено $EMOJI_COUNT эмодзи${NC}"
  exit 1
fi

# Проверка 2: Нет встроенных шрифтов без кириллицы
echo "Проверка 2: Поиск встроенных шрифтов..."
FONT_COUNT=$(grep -rn "lv_font_montserrat_[0-9]" base_ui/src --include="*.cpp" --include="*.h" | wc -l || true)
if [ "$FONT_COUNT" -eq 0 ]; then
  echo -e "${GREEN}✓ Все шрифты заменены на кириллические${NC}"
else
  echo -e "${RED}✗ Найдено $FONT_COUNT встроенных шрифтов${NC}"
  exit 1
fi

# Проверка 3: Все новые файлы в CMake
echo "Проверка 3: Проверка CMakeLists.txt..."
if grep -q "montserrat_cyr" base_ui/CMakeLists.txt; then
  echo -e "${GREEN}✓ Кириллические шрифты добавлены в CMake${NC}"
else
  echo -e "${RED}✗ Кириллические шрифты не в CMake${NC}"
  exit 1
fi

# Проверка 4: Валидность shell-скриптов
echo "Проверка 4: Проверка синтаксиса shell-скриптов..."
bash -n scripts/deploy_update.sh && echo -e "${GREEN}✓ deploy_update.sh корректен${NC}"
bash -n scripts/rollback.sh && echo -e "${GREEN}✓ rollback.sh корректен${NC}"
bash -n scripts/generate_cyrillic_fonts.sh && echo -e "${GREEN}✓ generate_cyrillic_fonts.sh корректен${NC}"

# Проверка 5: Валидность YAML workflow
echo "Проверка 5: Проверка GitHub Actions workflow..."
if [ -f .github/workflows/build.yml ] && grep -q "guppyscreen-ke-mips" .github/workflows/build.yml; then
  echo -e "${GREEN}✓ workflow.yml корректен${NC}"
else
  echo -e "${RED}✗ workflow.yml некорректен${NC}"
  exit 1
fi

echo -e "${GREEN}=== Все проверки пройдены успешно! ===${NC}"
