# Copyright (c) ZeroC, Inc.

from Util import CSharpMapping, Darwin, Mapping, TestSuite, platform

options = {}
# Disable IPv6 for .NET on macOS. TODO: probably obsolete, see #6436.
if isinstance(Mapping.getByPath(__name__), CSharpMapping) and isinstance(platform, Darwin):
    options = {"ipv6": [False]}

TestSuite(__name__, multihost=False, options=options)
