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

def rewrite(fi, fo, javaCmd):
    for l in fi:
        if l.find('exe="java"') != -1:
            l = l.replace('exe="java"', 'exe=%s' % javaCmd)
        fo.write(l)
    fi.close()
    fo.close()

desc = 'application.xml'
if Util.javaCmd != "java":
    desc = 'tmp_application.xml'
    fi = open("application.xml", "r")
    fo = open("tmp_application.xml", "w")
    rewrite(fi, fo, Util.javaCmd)
    fi = open("application_with_template.xml", "r")
    fo = open("tmp_application_with_template.xml", "w")
    rewrite(fi, fo, Util.javaCmd)
    fi = open("application_with_replication.xml", "r")
    fo = open("tmp_application_with_replication.xml", "w")
    rewrite(fi, fo, Util.javaCmd)

simple.run('java -jar build/libs/client.jar', desc[0:len(desc)-4])

os.remove("tmp_application.xml")
os.remove("tmp_application_with_template.xml")
os.remove("tmp_application_with_replication.xml")
