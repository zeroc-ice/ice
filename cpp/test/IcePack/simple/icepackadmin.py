#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2001
# Mutable Realms, Inc.
# Huntsville, AL, USA
#
# All Rights Reserved
#
# **********************************************************************

import os, sys

for toplevel in [".", "..", "../..", "../../..", "../../../.."]:
    if os.path.exists(os.path.join(toplevel, "config", "TestUtil.py")):
        break
else:
    raise "can't find toplevel directory!"

icePackAdmin = os.path.join(toplevel, "bin", "icepackadmin")
os.system(icePackAdmin + r' "--Ice.Adapter.Admin.Endpoints=tcp -p 12347 -t 5000"')

sys.exit(1)

