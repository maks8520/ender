# Инструкция по кросс-компиляции для MIPS (Ingenic X2000 / Ender 3 V3 KE)

## Аппаратный контекст
Процессор Creality Ender 3 V3 KE — Ingenic XBurst2 X2000 (**MIPS32 / mipsel**).
Сборка кастомного бинарника `guppyscreen` должна выполняться в мусль-тулчейне MIPS (`mipsel-buildroot-linux-musl-` / `mipsel-linux-gnu`).

---

## Кросс-компиляция через официальный Docker-образ

Для гарантированной воспроизводимости бинарника используется официальный Docker-образ из репозитория: `ghcr.io/coreflake1/guppydev:latest`.

### Команда сборки:

```bash
docker run --rm -it -v "$PWD/base_ui":/work -w /work ghcr.io/coreflake1/guppydev:latest bash scripts/build-mips.sh
```

После выполнения сборки готовый исполняемый файл будет располагаться по пути:
`base_ui/build/bin/guppyscreen`

---

## Установка собранного бинарника на принтер

1. Скопируйте файл на принтер по SSH:
   ```bash
   scp base_ui/build/bin/guppyscreen root@<IP_ПРИНТЕРА>:/usr/data/guppyscreen/guppyscreen
   ```
2. Сделайте файл исполняемым и перезапустите службу:
   ```bash
   ssh root@<IP_ПРИНТЕРА> "chmod +x /usr/data/guppyscreen/guppyscreen && /etc/init.d/S99guppyscreen restart"
   ```
