#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2002
# ZeroC, Inc.
# Huntsville, AL, USA
#
# All Rights Reserved
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
populateOptions = " -I" + toplevel + "/schema -I" + testdir + " --db-dir " + dbdir;

print "starting populate...",
populatePipe = os.popen(populate + populateOptions)
print "ok"

for output in populatePipe.xreadlines():
    print output,

populateStatus = populatePipe.close()

if populateStatus:
    sys.exit(1)

validate = os.path.join(testdir, "validate")
validateOptions = " --db-dir " + dbdir;

print "starting validate...",
validatePipe = os.popen(validate + validateOptions)
print "ok"

for output in validatePipe.xreadlines():
    print output,

validateStatus = validatePipe.close()

if validateStatus:
    sys.exit(1)

sys.exit(0)
