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

def rewrite(fi, fo):
    for l in fi:
        if l.find('exe="./server.exe"') != -1:
            l = l.replace('exe="./server.exe"', 'exe="mono"') + "<option>./server.exe</option>"
        fo.write(l)
    fi.close()
    fo.close()
desc = 'application.xml'
if Util.isMono():
    desc = 'tmp_application.xml'
    fi = open("application.xml", "r")
    fo = open("tmp_application.xml", "w")
    rewrite(fi, fo)
    fi = open("application_with_template.xml", "r")
    fo = open("tmp_application_with_template.xml", "w")
    rewrite(fi, fo)
    fi = open("application_with_replication.xml", "r")
    fo = open("tmp_application_with_replication.xml", "w")
    rewrite(fi, fo)

simple.run('client.exe', desc[0:len(desc)-4])
