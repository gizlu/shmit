#!/bin/sh

# Do screenshoot of selected area/current window/fullscreen.
# Copy it to clipboard and save on disk with current time as filename
# License: gizlu 2021, Do what the fuck you want, just dont blame me for anything
# Dependencies: maim, xclip

filename=~/Pictures/screeny/$(date +%F-%T).png

case $1 in
    select)
        maim -s --format png $filename && xclip -sel clipboard -t image/png -i $filename
        ;;
    window)
        maim -i $(xdotool getactivewindow) $filename && xclip -sel clipboard -t image/png -i $filename
        ;;
    fullscreen)
        maim --format png $filename && xclip -sel clipboard -t image/png -i $filename
        ;;
    *)
        printf "Usage: screenshoot {select|window|fullscreen}\n"
        exit
        ;;
esac
