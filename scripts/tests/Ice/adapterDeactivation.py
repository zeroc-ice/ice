# -*- coding: utf-8 -*-
#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

from Util import CSharpMapping, Darwin, TestSuite, platform, Mapping

# Disable IPv6 for macOS until https://github.com/dotnet/runtime/pull/108334 is merged in .NET 8
# See https://github.com/zeroc-ice/ice/issues/2061
ipv6 = not (isinstance(Mapping.getByPath(__name__), CSharpMapping) and isinstance(platform, Darwin))

TestSuite(__name__, multihost=False, options={"ipv6": [ipv6]})
