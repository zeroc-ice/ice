#!/bin/bash 

ICEVER=$1

PRODUCTCODE="{`uuidgen`}"
UPGRADECODE="{`uuidgen`}"

VERARRAY=(`echo "$ICEVER" | sed -e 's/\./ /g'`)

sed " 
s/\(<row><td>ProductName<\/td><td>Ice \)[0-9]\+.[0-9]\+.[0-9]\+\(.*\)/\1$ICEVER\2/
s/\(<row><td>ProductVersion<\/td><td>\)[0-9]\+.[0-9]\+.[0-9]\+\(.*\)/\1$ICEVER\2/
s/\(<row><td>ProductCode<\/td><td>\).[-0-9A-Za-z]\+.\(.*\)/\1$PRODUCTCODE\2/
s/\(<row><td>UpgradeCode<\/td><td>\).[-0-9A-Za-z]\+.\(.*\)/\1$UPGRADECODE\2/
s/\(<subject>Ice \)[0-9]\+.[0-9]\+[0-9]\+\(.*\)/\1$ICEVER\2/
s/\(<row><td>ICE_INSTALL_DIR.*WindowsVolume<\/td><td>ICE-\)[0-9]\+..[0-9]\+.Ice-[0-9]\+.[0-9]\+.[0-9]\+\(.*\)/\1${VERARRAY[0]}_~${VERARRAY[2]}\|Ice-$ICEVER\2/
s/\(<row><td>ZEROC.*MSIPackageFileName<\/td><td>Ice-\)[0-9]\+.[0-9]\+.[0-9]\+\(.*\)/\1$ICEVER\2/
"
