#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import sys, os

try:
    import demoscript
except ImportError:
    for toplevel in [".", "..", "../..", "../../..", "../../../.."]:
        toplevel = os.path.normpath(toplevel)
        if os.path.exists(os.path.join(toplevel, "demoscript")):
            break 
    else:
        raise "can't find toplevel directory!"
    sys.path.append(os.path.join(toplevel))
    import demoscript

import demoscript.Util
demoscript.Util.defaultLanguage = "C++"
import demoscript.IceGrid.icebox

desc = 'application.xml'
if demoscript.Util.mode == 'debug':
    fi = open(desc, "r")
    desc = 'tmp_application.xml'
    fo = open(desc, "w")
    for l in fi:
        if l.find('exe="icebox"') != -1:
            l = l.replace('exe="icebox"', 'exe="iceboxd.exe"')
        fo.write(l)
    fi.close()
    fo.close()

directory = os.path.dirname(os.path.abspath(__file__))
demoscript.Util.addLdPath(directory)

demoscript.IceGrid.icebox.run('./client', desc[0:len(desc)-4])
