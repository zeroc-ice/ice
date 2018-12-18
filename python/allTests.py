#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import os, sys
sys.path.append(os.path.join(os.path.dirname(__file__), "..", "scripts"))

from Util import runTestsWithPath

runTestsWithPath(__file__)
