#!/usr/bin/env ruby
# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

require 'Ice'
Ice::loadSlice('Filesystem.ice')

# Recursively print the contents of directory "dir"
# in tree fashion. For files, show the contents of
# each file. The "depth" parameter is the current
# nesting level (for indentation).

def listRecursive(dir, depth)
    indent = ''
    depth = depth + 1
    for i in (0...depth)
        indent += "\t"
    end

    contents = dir.list()

    for node in contents
        subdir = Filesystem::DirectoryPrx::checkedCast(node)
        file = Filesystem::FilePrx::uncheckedCast(node)
        print indent + node.name()
        if subdir
            puts "(directory):"
            listRecursive(subdir, depth)
        else
            puts "(file):"
            text = file.read()
            for line in text
                puts indent + "\t" + line
            end
        end
    end
end

status = 0
ic = nil
begin
    # Create a communicator
    #
    ic = Ice::initialize(ARGV)

    # Create a proxy for the root directory
    #
    obj = ic.stringToProxy("RootDir:default -p 10000")

    # Down-cast the proxy to a Directory proxy
    #
    rootDir = Filesystem::DirectoryPrx::checkedCast(obj)

    # Recursively list the contents of the root directory
    #
    puts "Contents of root directory:"
    listRecursive(rootDir, 0)
rescue => ex
    puts ex
    print ex.backtrace.join("\n")
    status = 1
end

if ic
    # Clean up
    #
    begin
        ic.destroy()
    rescue => ex
        puts ex
        print ex.backtrace.join("\n")
        status = 1
    end
end

exit(status)
