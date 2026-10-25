#!/bin/bash
set -e

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

echo -e "${GREEN}=== Безопасное обновление бинарника OpenKE ===${NC}"

if [ "$EUID" -ne 0 ]; then
  echo -e "${RED}Ошибка: скрипт должен запускаться от root${NC}"
  exit 1
fi

echo "Поиск USB-флешки..."
MOUNT_POINTS=$(ls /mnt/ 2>/dev/null | grep -E "usb|UDISK|media" || true)

if [ -z "$MOUNT_POINTS" ]; then
  echo -e "${RED}Ошибка: USB-флешка не найдена в /mnt/${NC}"
  exit 1
fi

MOUNT_POINT="/mnt/$(echo "$MOUNT_POINTS" | head -n1)"
echo -e "${GREEN}Найдена флешка: $MOUNT_POINT${NC}"

if [ ! -f "$MOUNT_POINT/guppyscreen" ]; then
  echo -e "${RED}Ошибка: файл guppyscreen не найден на флешке${NC}"
  exit 1
fi

BACKUP_DIR="/usr/data/guppyify-backup"
mkdir -p "$BACKUP_DIR"
BACKUP_FILE="$BACKUP_DIR/guppyscreen.prev-custom-$(date +%Y%m%d-%H%M%S)"

echo -e "${YELLOW}Создание бэкапа текущего бинарника...${NC}"
if [ -f /usr/data/guppyscreen/guppyscreen ]; then
  cp /usr/data/guppyscreen/guppyscreen "$BACKUP_FILE"
  echo -e "${GREEN}Бэкап создан: $BACKUP_FILE${NC}"
fi

echo -e "${YELLOW}Остановка сервиса S99guppyscreen...${NC}"
/etc/init.d/S99guppyscreen stop 2>/dev/null || true
sleep 2

echo -e "${YELLOW}Замена бинарника...${NC}"
mkdir -p /usr/data/guppyscreen
cp "$MOUNT_POINT/guppyscreen" /usr/data/guppyscreen/guppyscreen
chmod +x /usr/data/guppyscreen/guppyscreen

echo -e "${YELLOW}Запуск сервиса...${NC}"
/etc/init.d/S99guppyscreen start
sleep 5

echo "Проверка работоспособности..."
if pgrep -x guppyscreen > /dev/null; then
  echo -e "${GREEN}✓ Сервис успешно запущен!${NC}"
  echo -e "${GREEN}✓ Обновление завершено успешно!${NC}"
  echo -e "${YELLOW}Для отката используйте: cp $BACKUP_FILE /usr/data/guppyscreen/guppyscreen && /etc/init.d/S99guppyscreen restart${NC}"
else
  echo -e "${RED}✗ Сервис не запустился! Выполняется автооткат...${NC}"
  if [ -f "$BACKUP_FILE" ]; then
    cp "$BACKUP_FILE" /usr/data/guppyscreen/guppyscreen
    /etc/init.d/S99guppyscreen restart
  fi
  echo -e "${RED}✗ Автооткат выполнен.${NC}"
  exit 1
fi
