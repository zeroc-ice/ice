#!/bin/bash

echo "">$1

cat >$1 <<\_CMTFILE
# To create an explicit listing for the demos run the following command
# in the demo directory of the IcePy distribution:
# find . -name "*.py" -type f | sed -e 's/^\.\///' >> file
#
# If you want to use wild card searches instead of explicitly listing
# files, you can delete the filenames and uncomment the following lines:
#
# *.py
# include=**/*.py
#
# Stop uncommenting here
# Delete from here to EOF

_CMTFILE

find . -name "*.py" -type f | sed -e 's/^\.\///' >> $1
