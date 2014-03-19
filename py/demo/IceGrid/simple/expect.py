#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import sys, os

path = [ ".", "..", "../..", "../../..", "../../../.." ]
head = os.path.dirname(sys.argv[0])
if len(head) > 0:
    path = [os.path.join(head, p) for p in path]
path = [os.path.abspath(p) for p in path if os.path.exists(os.path.join(p, "demoscript")) ]
if len(path) == 0:
    raise RuntimeError("can't find toplevel directory!")
sys.path.append(path[0])

from demoscript import Util
from demoscript.IceGrid import simple

def rewrite(namein, nameout):
    fi = open(namein, "r")
    fo = open(nameout, "w")
    for l in fi:
        if sys.version_info[0] == 3 and not Util.isWin32():
            if l.find('exe="python"') != -1:
                l = l.replace('exe="python"', 'exe="python3"')
        if l.find('option') != -1:
            fo.write('<option>-u</option>')
        fo.write(l)
    fi.close()
    fo.close()

rewrite('application.xml', 'tmp_application.xml')
rewrite('application_with_template.xml', 'tmp_application_with_template.xml')
rewrite('application_with_replication.xml', 'tmp_application_with_replication.xml')

simple.run('Client.py', 'tmp_application')
