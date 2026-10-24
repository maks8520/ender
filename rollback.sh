#!/bin/sh
# ==============================================================================
# Custom Touch UI Rollback / Recovery Script for Ender 3 V3 KE (Creality OS)
# ==============================================================================

set -e

yellow="\033[01;33m"
green="\033[01;32m"
red="\033[01;31m"
white="\033[m"

BACKUP_DIR="/usr/data/custom_ui_backup"

printf "${green}=== Custom Touch UI Rollback / Recovery ===${white}\n"

# Check root
if [ "$(id -u)" -ne 0 ]; then
    printf "${red}ERROR: This script must be run as root.${white}\n"
    exit 1
fi

# Step 1: Stop and remove custom UI service
printf "${green}Stopping Custom Touch UI service...${white}\n"

if [ -f /etc/init.d/S99guppyscreen ]; then
    /etc/init.d/S99guppyscreen stop 2>/dev/null || true
    rm -f /etc/init.d/S99guppyscreen
fi

killall -q guppyscreen 2>/dev/null || true

# Step 2: Restore stock binaries
printf "${green}Restoring stock Creality OS display binaries...${white}\n"

if [ -f /usr/bin/Monitor.disable ] && [ ! -f /usr/bin/Monitor ]; then
    mv /usr/bin/Monitor.disable /usr/bin/Monitor
    printf "${green}  Restored /usr/bin/Monitor${white}\n"
fi

if [ -f /usr/bin/display-server.disable ] && [ ! -f /usr/bin/display-server ]; then
    mv /usr/bin/display-server.disable /usr/bin/display-server
    printf "${green}  Restored /usr/bin/display-server${white}\n"
fi

# Step 3: Restore stock init scripts from backup
printf "${green}Restoring stock Creality OS init scripts...${white}\n"

if [ -f "$BACKUP_DIR/S99start_app" ]; then
    cp "$BACKUP_DIR/S99start_app" /etc/init.d/S99start_app
    chmod +x /etc/init.d/S99start_app
    printf "${green}  Restored /etc/init.d/S99start_app${white}\n"
elif [ -f /usr/bin/display-server ] && [ ! -f /etc/init.d/S99start_app ]; then
    # Fallback init script for start_app if backup missing
    cat > /etc/init.d/S99start_app << 'EOF'
#!/bin/sh
start() {
    [ -x /usr/bin/display-server ] && /usr/bin/display-server &
    [ -x /usr/bin/Monitor ] && /usr/bin/Monitor &
}
stop() {
    killall -q display-server Monitor || true
}
case "$1" in
    start) start ;;
    stop) stop ;;
    restart) stop; sleep 1; start ;;
    *) echo "Usage: $0 {start|stop|restart}" ;;
esac
EOF
    chmod +x /etc/init.d/S99start_app
    printf "${yellow}  Created default /etc/init.d/S99start_app${white}\n"
fi

if [ -f "$BACKUP_DIR/S12boot_display" ]; then
    cp "$BACKUP_DIR/S12boot_display" /etc/init.d/S12boot_display
    chmod +x /etc/init.d/S12boot_display
    printf "${green}  Restored /etc/init.d/S12boot_display${white}\n"
fi

if [ -f "$BACKUP_DIR/S50dropbear" ]; then
    cp "$BACKUP_DIR/S50dropbear" /etc/init.d/S50dropbear
    chmod +x /etc/init.d/S50dropbear
    printf "${green}  Restored /etc/init.d/S50dropbear${white}\n"
fi

if [ -f /etc/init.d/DISABLED_S97webrtc ] && [ ! -f /etc/init.d/S97webrtc ]; then
    mv /etc/init.d/DISABLED_S97webrtc /etc/init.d/S97webrtc
    printf "${green}  Restored /etc/init.d/S97webrtc${white}\n"
fi

# Step 4: Restart stock Creality OS services
printf "${green}Restarting Creality OS factory services...${white}\n"

if [ -f /etc/init.d/S99start_app ]; then
    /etc/init.d/S99start_app start 2>/dev/null || true
fi

printf "\n${green}=====================================================${white}\n"
printf "${green} Rollback complete!${white}\n"
printf "${green} Stock Creality OS display services have been restored.${white}\n"
printf "${green}=====================================================${white}\n"
