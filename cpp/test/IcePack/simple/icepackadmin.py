#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2001
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
    if os.path.exists(os.path.join(toplevel, "config", "TestUtil.py")):
        break
else:
    raise "can't find toplevel directory!"

icePackAdmin = os.path.join(toplevel, "bin", "icepackadmin")
os.system(icePackAdmin + r' "--Ice.Adapter.Admin.Endpoints=tcp -p 12347 -t 5000"')

sys.exit(1)

