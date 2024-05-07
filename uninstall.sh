#!/bin/bash

#Help text
display_help() {
    echo
    echo "   --aasdk          uninstall aasdk"
    echo "   --openauto       uninstall openauto "
    echo "   --gstreamer      uninstall gstreamer "
    echo "   --dash           uninstall dash "
    echo "   --h264bitstream  uninstall h264bitstream"
    echo "   --pulseaudio     uninstall pulseaudio"
    echo "   --bluez          uninstall bluez"
    echo "   --ofono          uninstall ofono"
    echo "   --all            uninstall all components "
    echo
}

script_path=$(dirname "$(realpath -s "$0")")

#check to see if there are any arguments supplied, if none are supplied run full install
if [ $# -gt 0 ]; then
  #initialize all arguments to false before looping to find which are available
  aasdk=false
  gstreamer=false
  openauto=false
  dash=false
  h264bitstream=false
  pulseaudio=false
  bluez=false
  ofono=false
    while [ "$1" != "" ]; do
        case $1 in
            --aasdk )           aasdk=true
                                    ;;
            --gstreamer )       gstreamer=true
                                    ;;
            --openauto )       openauto=true
                                    ;;
            --dash )           dash=true
                                    ;;
            --h264bitstream )  h264bitstream=true
                                    ;;
            --pulseaudio )     pulseaudio=true
                                    ;;
            --bluez )          bluez=true
                                    ;;
            --ofono )          ofono=true
                                    ;;
            -h | --help )           display_help
                                    exit
                                    ;;
            * )                     display_help
                                    exit 1
        esac
        shift
    done
else
    echo -e Full uninstall running'\n'
    aasdk=true
    gstreamer=true
    openauto=true
    dash=true
    h264bitstream=true
    pulseaudio=true
    bluez=true
    ofono=true
fi

if [ $pulseaudio = true ] && [ -d "$script_path/pulseaudio" ]; then
    echo "Uninstalling pulseaudio"
    cd pulseaudio
    sudo make uninstall
    cd ..
    sudo rm -rv pulseaudio
    rm uninstall
fi

if [ $ofono = true ] && [[ $(dpkg -l ofono 2> /dev/null) ]]; then
    echo "Uninstalling ofono"
    sudo apt purge -y ofono
    echo "Removing pulse config enhancemnets"
    sudo sed -i 's/load-module module-bluetooth-discover headset=ofono/load-module module-bluetooth-discover/g' /etc/pulse/default.pa
    sudo sed -i '/### Echo cancel and noise reduction/,/.endif/d' /etc/pulse/default.pa
fi

if [ $bluez = true ] && [ -d "$script_path/bluez-5.63" ]; then
    echo "Uninstalling bluez-5.63"
    cd bluez-5.63
    sudo make uninstall
    cd ..
    rm -rv bluez-5.63
fi

if [ $aasdk = true ] && [ -d "$script_path/aasdk/build" ]; then
    echo "Removing aasdk"
    cd aasdk/build
    xargs sudo rm < install_manifest.txt
    cd ../..
    sudo rm -rv aasdk
fi

if [ $h264bitstream = true ] && [ -d "$script_path/h264bitstream" ]; then
    echo "Uninstalling h264bitstream"
    cd h264bitstream
    sudo make uninstall
    cd ..
    sudo rm -rv h264bitstream
fi

if [ $gstreamer = true ] && [ -d "$script_path/qt-gstreamer/build" ]; then
    echo "Uninstalling qt-gstreamer"
    cd qt-gstreamer/build
    sudo make uninstall
    cd ../..
    sudo rm -rv qt-gstreamer
fi

if [ $openauto = true ] && [ -d "$script_path/openauto/build" ]; then
    echo "Removing openauto"
    cd openauto/build
    xargs sudo rm < install_manifest.txt
    cd ../..
    sudo rm -rv openauto
fi

if [ $dash = true ] && [ -d "$script_path/bin" ]; then
    echo "Removing dash binary"
    rm -rv bin build
    if [ -f "$HOME/Desktop/dash.desktop" ]; then
        echo "Removing desktop shortcut"
        rm -v $HOME/Desktop/dash.desktop
    fi
    if [ -f "/etc/systemd/system/dash.service" ]; then
        echo "Removing Systemd daemon"
        sudo systemctl stop dash.service || true
        sudo systemctl disable dash.service || true
        sudo systemctl unmask dash.service || true
    fi
    if [ -f $HOME/run_dash.sh ]; then
        echo "Removing xinit runner and bashrc line"
        rm -v $HOME/run_dash.sh
        sed -i '/### xinit/,/fi/d' $HOME/.bashrc
    fi
fi