#!/bin/sh
# ==============================================================================
# Custom Touch UI Installer for Ender 3 V3 KE (Creality OS)
# ==============================================================================

set -e

yellow="\033[01;33m"
green="\033[01;32m"
red="\033[01;31m"
white="\033[m"

BACKUP_DIR="/usr/data/custom_ui_backup"
APP_DIR="/usr/data/guppyscreen"
BASE_UI_DIR="$(cd "$(dirname "$0")" && pwd)/base_ui"

printf "${green}=== Custom Touch UI Installer for Ender 3 V3 KE ===${white}\n"

# Check root
if [ "$(id -u)" -ne 0 ]; then
    printf "${red}ERROR: This script must be run as root.${white}\n"
    exit 1
fi

# Preflight disk space checks
REMAINING_ROOT_DISK=$(df -m / | tail -1 | awk '{print $4}')
if [ "$REMAINING_ROOT_DISK" -lt 15 ]; then
    printf "${red}CRITICAL: / partition disk space low (${REMAINING_ROOT_DISK}MB free). Aborting.${white}\n"
    exit 1
fi

REMAINING_DATA_DISK=$(df -m /usr/data | tail -1 | awk '{print $4}')
if [ "$REMAINING_DATA_DISK" -lt 100 ]; then
    printf "${red}CRITICAL: /usr/data disk space low (${REMAINING_DATA_DISK}MB free). Aborting.${white}\n"
    exit 1
fi

printf "${green}Setting up backup directory at $BACKUP_DIR...${white}\n"
mkdir -p "$BACKUP_DIR"

# Step 1: Backup Creality OS services
printf "${green}Backing up stock Creality OS display services...${white}\n"

if [ -f /etc/init.d/S99start_app ] && [ ! -f "$BACKUP_DIR/S99start_app" ]; then
    cp /etc/init.d/S99start_app "$BACKUP_DIR/S99start_app"
    printf "${green}  Backed up S99start_app${white}\n"
fi

if [ -f /etc/init.d/S12boot_display ] && [ ! -f "$BACKUP_DIR/S12boot_display" ]; then
    cp /etc/init.d/S12boot_display "$BACKUP_DIR/S12boot_display"
    printf "${green}  Backed up S12boot_display${white}\n"
fi

if [ -f /etc/init.d/S50dropbear ] && [ ! -f "$BACKUP_DIR/S50dropbear" ]; then
    cp /etc/init.d/S50dropbear "$BACKUP_DIR/S50dropbear"
    printf "${green}  Backed up S50dropbear${white}\n"
fi

if [ -f /usr/bin/Monitor ] && [ ! -f "$BACKUP_DIR/Monitor" ]; then
    cp /usr/bin/Monitor "$BACKUP_DIR/Monitor"
fi

if [ -f /usr/bin/display-server ] && [ ! -f "$BACKUP_DIR/display-server" ]; then
    cp /usr/bin/display-server "$BACKUP_DIR/display-server"
fi

# Step 2: Disable stock Creality OS UI services
printf "${green}Disabling Creality OS default display services...${white}\n"

killall -q Monitor || true
killall -q display-server || true
killall -q master-server || true

if [ -f /usr/bin/Monitor ]; then
    mv /usr/bin/Monitor /usr/bin/Monitor.disable 2>/dev/null || true
fi

if [ -f /usr/bin/display-server ]; then
    mv /usr/bin/display-server /usr/bin/display-server.disable 2>/dev/null || true
fi

if [ -f /etc/init.d/S99start_app ]; then
    rm -f /etc/init.d/S99start_app
fi

# Step 3: Install Custom Touch UI application files
printf "${green}Installing Custom Touch UI to $APP_DIR...${white}\n"
mkdir -p "$APP_DIR"
mkdir -p "$APP_DIR/thumbnails"

if [ -d "$BASE_UI_DIR/build/bin" ] && [ -f "$BASE_UI_DIR/build/bin/guppyscreen" ]; then
    cp "$BASE_UI_DIR/build/bin/guppyscreen" "$APP_DIR/guppyscreen"
    chmod +x "$APP_DIR/guppyscreen"
elif [ -f "$APP_DIR/guppyscreen" ]; then
    printf "${yellow}Using existing binary at $APP_DIR/guppyscreen${white}\n"
else
    printf "${yellow}Notice: Pre-compiled binary not found in $BASE_UI_DIR/build/bin/guppyscreen.${white}\n"
fi

if [ -d "$BASE_UI_DIR/assets" ]; then
    cp -r "$BASE_UI_DIR/assets" "$APP_DIR/" 2>/dev/null || true
fi

if [ -d "$BASE_UI_DIR/themes" ]; then
    cp -r "$BASE_UI_DIR/themes" "$APP_DIR/" 2>/dev/null || true
fi

if [ -d "$BASE_UI_DIR/scripts" ]; then
    cp -r "$BASE_UI_DIR/scripts" "$APP_DIR/" 2>/dev/null || true
fi

# Create default guppyconfig.json if not present
if [ ! -f "$APP_DIR/guppyconfig.json" ]; then
    cat > "$APP_DIR/guppyconfig.json" << 'EOF'
{
  "default_printer": "k1",
  "display_rotate": 0,
  "display_sleep_sec": 600,
  "camera_enabled": true,
  "camera_fps": 12,
  "camera_url": "http://127.0.0.1:8080/?action=stream",
  "invert_touch_x": false,
  "invert_touch_y": false,
  "swap_touch_xy": false,
  "touch_calibrated": false,
  "log_path": "/usr/data/printer_data/logs/guppyscreen.log",
  "printers": {
    "k1": {
      "default_macros": {
        "cooldown": "SET_HEATER_TEMPERATURE HEATER=extruder TARGET=0\nSET_HEATER_TEMPERATURE HEATER=heater_bed TARGET=0",
        "load_filament": "_GUPPY_LOAD_MATERIAL",
        "unload_filament": "_GUPPY_QUIT_MATERIAL"
      },
      "log_level": "info",
      "moonraker_api_key": false,
      "moonraker_host": "127.0.0.1",
      "moonraker_port": 7125
    }
  },
  "thumbnail_path": "/usr/data/printer_data/thumbnails"
}
EOF
    printf "${green}Created default guppyconfig.json${white}\n"
fi

# Step 4: Install Init Service
printf "${green}Installing Custom Touch UI init service...${white}\n"

cat > /etc/init.d/S99guppyscreen << 'EOF'
#!/bin/sh
# Service for Custom Touch UI

APP_DIR=/usr/data/guppyscreen
PROG=$APP_DIR/guppyscreen
PID_FILE=/var/run/guppyscreen.pid

start() {
    echo "Starting Custom Touch UI..."
    if [ ! -x "$PROG" ]; then
        echo "Error: $PROG not executable"
        exit 1
    fi
    start-stop-daemon -S -q -b -m -p "$PID_FILE" -d "$APP_DIR" --exec "$PROG"
}

stop() {
    echo "Stopping Custom Touch UI..."
    start-stop-daemon -K -q -p "$PID_FILE" 2>/dev/null || killall -q guppyscreen || true
    rm -f "$PID_FILE"
}

restart() {
    stop
    sleep 1
    start
}

case "$1" in
  start) start ;;
  stop) stop ;;
  restart) restart ;;
  *) echo "Usage: $0 {start|stop|restart}"; exit 1 ;;
esac
EOF

chmod +x /etc/init.d/S99guppyscreen

# Step 4b: Verify MJPEG / Camera streamer dependencies
printf "${green}Checking camera stream dependencies (mjpg-streamer / libjpeg-turbo)...${white}\n"
if [ -f /usr/bin/opkg ]; then
    opkg update >/dev/null 2>&1 || true
    opkg install libjpeg-turbo ffmpeg >/dev/null 2>&1 || true
fi
if [ ! -f /usr/bin/curl ] && [ ! -f /tmp/curl ]; then
    printf "${yellow}Notice: libcurl/curl utility recommended for camera streaming.${white}\n"
fi

# Step 5: Check Moonraker service status
printf "${green}Verifying Moonraker service on port 7125...${white}\n"
if [ -f /etc/init.d/S56moonraker_service ]; then
    /etc/init.d/S56moonraker_service start >/dev/null 2>&1 || true
fi

# Step 6: Start Custom Touch UI service
printf "${green}Starting Custom Touch UI service...${white}\n"
/etc/init.d/S99guppyscreen restart || true

IP_ADDR=$(ip route get 1 2>/dev/null | awk 'NR==1{for(i=1;i<=NF;i++) if ($i=="src") {print $(i+1); exit}}')

printf "\n${green}=====================================================${white}\n"
printf "${green} Installation complete!${white}\n"
printf "${green} Custom Touch UI is configured and running.${white}\n"
if [ -n "$IP_ADDR" ]; then
    printf "${green} Printer IP Address: http://${IP_ADDR}/${white}\n"
fi
printf "${yellow} To rollback to stock Creality OS UI, execute:${white}\n"
printf "${yellow}   sh /usr/data/custom_ui/rollback.sh (or ./rollback.sh)${white}\n"
printf "${green}=====================================================${white}\n"
