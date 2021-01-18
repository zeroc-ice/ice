# -*- coding: utf-8 -*-
#
# Copyright (c) ZeroC, Inc. All rights reserved.
#


# This test requires asyncio methods which are only available with Python 3.7
# We want to check the version of the python interpreter used to run the tests
# not the version of the python interpreter running this script see --python option

version = subprocess.check_output([currentConfig.python,
                                   "-c",
                                   "import sys; print(f\"{sys.version_info[0]}.{sys.version_info[1]}\")"], text=True)
version = tuple(int(num) for num in version.split("."))

if version >= (3, 7):
    TestSuite(__name__)
