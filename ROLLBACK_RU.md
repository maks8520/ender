# Инструкция по откату OpenKE

## Уровень 1: Откат к предыдущей кастомной версии
Используйте скрипт `rollback.sh` и выберите нужный бэкап:
```bash
bash scripts/rollback.sh
```

## Уровень 2: Откат к стоковому OpenKE
```bash
ssh root@<IP-адрес-принтера>
cp /usr/data/guppyify-backup/guppyscreen /usr/data/guppyscreen/guppyscreen
/etc/init.d/S99guppyscreen restart
```

## Уровень 3: Полный откат к заводской Creality OS
Запустите официальный uninstaller:
```bash
ssh root@<IP-адрес-принтера>
sh -c "$(wget --no-check-certificate -qO - https://raw.githubusercontent.com/coreflake1/guppyscreen/main/scripts/installer.sh)" uninstall
```
