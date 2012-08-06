#
# Convert Windows path to Unix path, also escape whitespace.
#
v=`cygpath -u "$1"`; v=${v//[[:blank:]]/\\ }; v=${v/(/\\(}; echo ${v/)/\\)}
