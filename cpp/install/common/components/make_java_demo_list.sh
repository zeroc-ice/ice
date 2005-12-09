#!/bin/bash

echo "">$1

cat >$1 <<\_CMTFILE
# To create an explicit listing for the demos run the following command
# in the demo directory of the IceJ distribution.:
# find . -name "*" -type f | sed -e # '/\/generated\/\|\/classes\/\|\
#        .dummy/d' | sed -e 's/^\.\///' >> file
#
# If you want to use wild card searches instead of explicitly listing
# files, you can delete the filenames and uncomment the following lines:
#
# include=*.*
# exclude=**/.dummy
# exclude=**/classes/**
# exclude=**/generated/**
#
# Stop uncommenting here!
# Delete from here to EOF

_CMTFILE

find . -name "*" -type f |  sed -e '/\/generated\/\|\/classes\/\|.dummy/d' | sed -e 's/^\.\///' >> $1
