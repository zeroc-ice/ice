# Copyright (c) ZeroC, Inc.

from Util import CSharpMapping, Darwin, TestSuite, platform, Mapping

options = {}
# Disable IPv6 for .NET on macOS until https://github.com/dotnet/runtime/pull/108334 is merged in .NET 8
# See https://github.com/zeroc-ice/ice/issues/2061
if isinstance(Mapping.getByPath(__name__), CSharpMapping) and isinstance(platform, Darwin):
    options = {"ipv6": [False]}

TestSuite(__name__, multihost=False, options=options)
