#!/bin/bash

echo "">$1

cat >$1 <<\_CMTFILE
# To create an explicit listing for the demos run the following command
# in the demo directory of the ice distribution.:
# find . -name "*" -type f | sed -e '/Debug\|Release\|.dsp$\|.dsw$\|.exe$\
#        \|.ilk$\|\.idb$\|.plg$\|.depend\|Makefile$\|.pdb$\|.ncb$\|.suo$\|\.dummy/d' | \
#        sed -e '/^Freeze\/backup/d' | sed -e 's/^\.\///' >> file
# find . -name "*" -type d | grep \/db |  sed -e 's/^\.\///' | sed -e '/^Freeze\/backup/d'  >> file
#
# If you want to use wild card searches instead of explicitly listing
# files, you can delete the filenames and uncomment the following lines:
#
# README
# include=**/*.*
# exclude=**/*.dsp
# exclude=**/*.dsw
# exclude=**/*.ncb
# exclude=**/*.suo
# exclude=**/*.exe
# exclude=**/*.ilk
# exclude=**/*.idb
# exclude=**/*.pdb
# exclude=**/*.plg
# exclude=**/.depend
# exclude=**/.dummy
# exclude=**/Makefile
# exclude=**/Debug/**
# exclude=**/Release/**
# exclude=Freeze/backup/**
#
# Stop uncommenting here!
# Delete from here to EOF

_CMTFILE

find . -name "*" -type f | sed -e '/Debug\|Release\|.dsp$\|.dsw$\|.exe$\|.ilk$\|.plg$\|.depend\|Makefile$\|\.idb$\|.pdb$\|.ncb$\|.suo$\|\.dummy/d' | sed -e '/^[\.\/]*Freeze\/backup/d' | sed -e '/.*vcproj[.].*/d' | sed -e 's/^\.\///' >> $1
find . -name "*" -type d | grep \/db |  sed -e 's/^\.\///' | sed -e '/^Freeze\/backup/d'  >> $1
