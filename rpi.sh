#!/bin/bash

###
# Helper scripts for setting various parameters and config for 
# Raspberry Pi units running Raspberry Pi OS.
###

display_help() {
    echo "Raspberry Pi Dash additional install helpers Version 0.3"
    echo "Usage: $0 [option...]" >&2
    echo
    echo "   -arb, --addrulebrightness        Add udev rules for brightness"
    echo "   -mem, --memorysplit              Set memory split"
    echo "   -gl,  --gldriver                 Set GL driver"
    echo "   -krn, --krnbt                    Set krnbt flag"
    echo "   -h, --help                       Show help of script"
    echo
    echo "Example: Add touchscreen brightness rule on your RPI."
    echo "   rpi -arb"
    echo
    echo "Example: Set memory split on your RPI."
    echo "   rpi -mem 128"
    echo
    echo "Example: Set GL driver on your RPI."
    echo "   rpi -gl [G2|G1]"
    echo "   KMS (G2) / Fake KMS (G1)"
    echo
    echo "Example: Set krnbt flag on your RPI."
    echo "   rpi -krn"
    echo
    exit 1
}

add_brightness_udev_rule() {
  FILE=/etc/udev/rules.d/52-dashbrightness.rules
  if [[ ! -f "$FILE" ]]; then
     # udev rules to allow write access to all users for Raspberry Pi 7" Touch Screen
     echo "SUBSYSTEM==\"backlight\", RUN+=\"/bin/chmod 666 /sys/class/backlight/%k/brightness\"" | sudo tee $FILE
     if [[ $? -eq 0 ]]; then
         echo -e "Permissions created\n"
     else
         echo -e "Unable to create permissions\n"
     fi
  else
     echo -e "Rules exists\n"
  fi
}

set_memory_split() {
  sudo raspi-config nonint do_memory_split $2
  if [[ $? -eq 0 ]]; then
     echo -e "Memory set to 128mb\n"
  else
     echo "Setting memory failed with error code $? please set manually"
     exit 1
  fi
}

set_opengl() {
  sudo raspi-config nonint do_gldriver $2
  if [[ $? -eq 0 ]]; then
     echo -e "OpenGL set ok\n"
  else
     echo "Setting openGL failed with error code $? please set manually"
     exit 1
  fi
}

enable_krnbt() {
  echo "enabling krnbt to speed up boot and improve stability"
  echo "dtparam=krnbt" >> /boot/config.txt
}

# Check if Raspberry Pi OS is active, otherwise kill script
if [ ! -f /etc/rpi-issue ]
then
 echo "This script works only for Raspberry Pi OS"
 exit 1;
fi

# Main Menu
while :
do
    case "$1" in
        -arb | --addrulebrightness)
            add_brightness_udev_rule
            exit 0
          ;;
        -mem | --memorysplit)
            if [ $# -ne 0 ]; then
              set_memory_split $2
              exit 0
            fi
          ;;
        -gl | --gldriver)
            if [ $# -ne 0 ]; then
              set_opengl $2
              exit 0
            fi
          ;;
        -krn | --krnbt)
            enable_krnbt
            exit 0
          ;;          
        -h | --help)
            display_help  # Call your function
            exit 0
          ;;
        "")  # If $1 is blank, run display_help
            display_help
            exit 0
          ;;
        --) # End of all options
            shift
            break
          ;;
        -*)
            echo "Error: Unknown option: $1" >&2
            ## or call function display_help
            exit 1
          ;;
        *)  # No more options
            break
          ;;
    esac
done