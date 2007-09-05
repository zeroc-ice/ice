#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
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
demoscript.Util.defaultLanguage = "Python"
import demoscript.IceGrid.simple

def rewrite(namein, nameout):
    fi = open(namein, "r")
    fo = open(nameout, "w")
    for l in fi:
        if l.find('option') != -1:
            fo.write('<option>-u</option>')
        fo.write(l)
    fi.close()
    fo.close()

rewrite('application.xml', 'tmp_application.xml')
rewrite('application_with_template.xml', 'tmp_application_with_template.xml')
rewrite('application_with_replication.xml', 'tmp_application_with_replication.xml')

demoscript.IceGrid.simple.run('Client.py', 'tmp_application')
