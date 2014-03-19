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
from demoscript.IceGrid import icebox

desc = 'application.xml'
if Util.isMono():
    fi = open(desc, "r")
    desc = 'tmp_application.xml'
    fo = open(desc, "w")
    for l in fi:
        if l.find('<icebox') != -1:
            l = '<icebox id="IceBox" exe="mono" activation="on-demand"><option>%s</option>' % Util.getIceBox("cs")
        fo.write(l)
    fi.close()
    fo.close()

icebox.run('client.exe', desc[0:len(desc)-4])
