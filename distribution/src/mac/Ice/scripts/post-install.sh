#!/bin/sh

if [[ -d "/Library/Developer/Ice-@ver@" ]]; then
    if [[ -L "/Library/Developer/Ice-@mmver@" ]]; then
        rm -f /Library/Developer/Ice-@mmver@
    fi
    ln -s "/Library/Developer/Ice-@ver@" "/Library/Developer/Ice-@mmver@"
fi

echo "/Library/Developer/Ice-@ver@/bin" > /etc/paths.d/ice.path
echo "/Library/Developer/Ice-@ver@/bin" > /etc/manpaths.d/ice.path