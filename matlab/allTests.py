#!/usr/bin/env python3

# Copyright (c) ZeroC, Inc.

import os
import sys

sys.path.append(os.path.join(os.path.dirname(__file__), "..", "scripts"))

from Util import runTestsWithPath

runTestsWithPath(__file__)
