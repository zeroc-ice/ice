#!/usr/bin/env python

import fileinput
import string

previous = ""

for line in fileinput.input():
    line = string.strip(line)

    if(previous):
        line = previous + " " + line

    if(line[-1] == "\\"):
        previous = line[:-2]
        continue
    else:
        previous = ""

    for s in string.split(line):
        if(s[0] != "/"):
            print s,

    print
            
