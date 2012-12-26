#!/bin/sh

if [[ -d "/Library/Developer/Ice-3.5b" ]]; then
    if [[ -L "/Library/Developer/Ice-3.5" ]]; then
        rm -f /Library/Developer/Ice-3.5
    fi
    ln -s "/Library/Developer/IceT-3.5b" "/Library/Developer/Ice-3.5"
fi
