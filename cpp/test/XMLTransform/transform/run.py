#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003
# ZeroC, Inc.
# Billerica, MA, USA
#
# All Rights Reserved.
#
# Ice is free software; you can redistribute it and/or modify it under
# the terms of the GNU General Public License version 2 as published by
# the Free Software Foundation.
#
# **********************************************************************

import os, sys

for toplevel in [".", "..", "../..", "../../..", "../../../.."]:
    toplevel = os.path.normpath(toplevel)
    if os.path.exists(os.path.join(toplevel, "config", "TestUtil.py")):
        break
else:
    raise "can't find toplevel directory!"

sys.path.append(os.path.join(toplevel, "config"))
import TestUtil

name = os.path.join("XMLTransform", "transform")
testdir = os.path.join(toplevel, "test", name)

dbdir = os.path.join(testdir, "db")
TestUtil.cleanDbDir(dbdir)

populate = os.path.join(testdir, "populate")
populateOptions = TestUtil.clientOptions + " -I" + toplevel + "/schema -I" + testdir + " --db-dir " + dbdir;

print "starting populate...",
populatePipe = os.popen(populate + populateOptions)
print "ok"

for output in populatePipe.xreadlines():
    print output,

populateStatus = populatePipe.close()

if populateStatus:
    sys.exit(1)

validate = os.path.join(testdir, "validate")
validateOptions = TestUtil.clientOptions + " --db-dir " + dbdir;

print "starting validate...",
validatePipe = os.popen(validate + validateOptions)
print "ok"

for output in validatePipe.xreadlines():
    print output,

validateStatus = validatePipe.close()

if validateStatus:
    sys.exit(1)

sys.exit(0)
