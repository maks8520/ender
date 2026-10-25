#!/bin/bash
set -e

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

echo -e "${GREEN}=== Откат OpenKE к предыдущей версии ===${NC}"

if [ "$EUID" -ne 0 ]; then
  echo -e "${RED}Ошибка: скрипт должен запускаться от root${NC}"
  exit 1
fi

BACKUP_DIR="/usr/data/guppyify-backup"

echo "Доступные бэкапы:"
ls -lh "$BACKUP_DIR"/guppyscreen.* 2>/dev/null || echo "Бэкапы не найдены"

echo -e "${YELLOW}Введите имя файла бэкапа для отката (или 'cancel' для отмены):${NC}"
read -r BACKUP_FILE

if [ "$BACKUP_FILE" = "cancel" ] || [ -z "$BACKUP_FILE" ]; then
  echo "Отмена"
  exit 0
fi

if [ ! -f "$BACKUP_DIR/$BACKUP_FILE" ]; then
  echo -e "${RED}Ошибка: файл $BACKUP_FILE не найден${NC}"
  exit 1
fi

echo -e "${YELLOW}Остановка сервиса...${NC}"
/etc/init.d/S99guppyscreen stop 2>/dev/null || true
sleep 2

echo -e "${YELLOW}Восстановление бэкапа...${NC}"
cp "$BACKUP_DIR/$BACKUP_FILE" /usr/data/guppyscreen/guppyscreen
chmod +x /usr/data/guppyscreen/guppyscreen

echo -e "${YELLOW}Запуск сервиса...${NC}"
/etc/init.d/S99guppyscreen start

echo -e "${GREEN}✓ Откат выполнен успешно!${NC}"
