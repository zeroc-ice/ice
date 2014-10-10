#!/bin/sh

#
# This script uninstalls all Ice @ver@ Third Party Packages currently installed on this system.
#

showHelp ()
{
    echo "Ice @ver@ Third Party Packages uninstall script"
    echo "Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved."
    echo "usage: "
    echo "  \"sudo $0\" -- uninstall all Ice @ver@ Third Party Packages installed on this system."
}

#
# Uninstall a package
#
uninstallPackage ()
{
    PACKAGE=$1

    if [[ ! -f "/var/db/receipts/$PACKAGE.bom" ]]; then
        echo "$PACKAGE - not installed, skipping"
        return 0
    fi

    if [[ ! -f "/var/db/receipts/$PACKAGE.plist" ]]; then
        echo "$PACKAGE - not installed, skipping"
        return 0
    fi

    VOLUME=""
    LOCATION=""
    VERSION=""
    pkgutil --pkg-info $PACKAGE |
    {
        while IFS=' ' read -r TOKEN VALUE; do
            if [[ "$TOKEN" == "volume:" ]]; then
                VOLUME=$VALUE
            elif [[ "$TOKEN" == "location:" ]]; then
                LOCATION=$VALUE
            elif [[ "$TOKEN" == "version:" ]]; then
                VERSION=$VALUE
            fi
        done

        if [[ "$PACKAGE" == "com.zeroc.ice@ver@-thirdparty" ]]; then
            BASE_PATH=$VOLUME$LOCATION
            if [[ "$BASE_PATH" ]]; then
                rm -rf "$BASE_PATH"
            fi
        fi

        pkgutil --forget $PACKAGE

        echo "$PACKAGE - uninstalled from $BASE_PATH"

        return 0
    }
}

#
# Parse command line arguments.
#
while true; do
    case "$1" in
      -h|--help|-\?) showHelp; exit 0;;
      -*) echo "invalid option: $1" 1>&2; showHelp; exit 1;;
      *)  break;;
    esac
done

#
# Display a error if not running as root.
#
if [ "$(id -u)" != "0" ]; then
    echo "ERROR: This script must be executed with root permissions -- prefix command with 'sudo'" 1>&2
    showHelp
    exit 1
fi


#
# Ask the user for comfirmation.
#


ok=0
confirmed="no"
answer=""

while [[ $ok -eq 0 ]]
do
    echo "Uninstall all Ice @ver@ Third Party Packages installed on this sytem? Yes/No"
    read -p "$*" answer
    if [[ ! "$answer" ]]; then
        answer="no"
    else
        answer=$(tr '[:upper:]' '[:lower:]' <<<$answer)
    fi

    if [[ "$answer" == 'y' || "$answer" == 'yes' || "$answer" == 'n' || "$answer" == 'no' ]]; then
        ok=1
    fi

    if [[ $ok -eq 0 ]]; then
        echo "Valid answers are: 'yes', 'y', 'no', 'n'"
    fi
done

if [[ "$answer" == 'y' || "$answer" == 'yes' ]]; then
    confirmed="yes"
else
    confirmed="no"
fi

if [[ "$confirmed" == "no" ]]; then
    echo "Uninstallation cancelled"
    exit 0
fi

uninstallPackage "com.zeroc.ice@ver@-thirdparty"

echo "Ice @ver@ Third Party Packages uninstallation completed successfully"
