#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import sys, traceback, Ice

Ice.loadSlice('Filesystem.ice')
import Filesystem

# Recursively display the contents of directory "dir"
# in tree fashion. For files, show the contents of
# each file. The "depth" parameter is the current
# nesting level (for indentation).

def listRecursive(dir, depth):
    indent = ''
    depth = depth + 1
    for i in range(depth):
        indent = indent + '\t'

    contents = dir.list()

    for node in contents:
        subdir = Filesystem.DirectoryPrx.checkedCast(node)
        file = Filesystem.FilePrx.uncheckedCast(node)
        sys.stdout.write(indent + node.name() + " ")
        if subdir:
            print("(directory):")
            listRecursive(subdir, depth)
        else:
            print("(file):")
            text = file.read()
            for line in text:
                print(indent + "\t" + line)

status = 0
ic = None
try:
    # Create a communicator
    #
    ice = Ice.initialize(sys.argv)

    # Create a proxy to the root directory
    #
    obj = ice.stringToProxy("RootDir:default -h localhost -p 10000")

    # Downcast the proxy to a Directory proxy
    #
    rootDir = Filesystem.DirectoryPrx.checkedCast(obj)

    # Recursively list the contents of the root directory
    #
    print("Contents of root directory:")
    listRecursive(rootDir, 0)
except:
    traceback.print_exc()
    status = 1

if ic:
    # Clean up
    #
    try:
        ic.destroy()
    except:
        traceback.print_exc()
        status = 1

sys.exit(status)
