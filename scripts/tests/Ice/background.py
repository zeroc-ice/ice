#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

from Util import TestSuite


TestSuite(__name__, libDirs=["testtransport"], options={"mx": [False]})
