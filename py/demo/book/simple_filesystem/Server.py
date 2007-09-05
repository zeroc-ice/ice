#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import sys, threading, Ice

Ice.loadSlice('Filesystem.ice')
import Filesystem

class DirectoryI(Filesystem.Directory):
    def __init__(self, name, parent):
        self._name = name
        self._parent = parent
        self._contents = []

        # Create an identity. The parent has the fixed identity "RootDir"
        #
        if self._parent:
            myID = Ice.stringToIdentity(Ice.generateUUID())
        else:
            myID = Ice.stringToIdentity("RootDir")

        # Add the identity to the object adapter
        #
        self._adapter.add(self, myID)

        # Create a proxy for the new node and add it as a child to the parent
        thisNode = Filesystem.NodePrx.uncheckedCast(self._adapter.createProxy(myID))

        if self._parent:
            self._parent.addChild(thisNode)

    # Slice Node::name() operation

    def name(self, current=None):
        return self._name

    # Slice Directory::list() operation

    def list(self, current=None):
        return self._contents

    # addChild is called by the child in order to add
    # itself to the _contents member of the parent

    def addChild(self, child):
        self._contents.append(child)

    _adpater = None

class FileI(Filesystem.File):
    def __init__(self, name, parent):
        self._name = name
        self._parent = parent
        self.lines = []

        assert(self._parent != None)

        # Create an identity
        #
        myID = Ice.stringToIdentity(Ice.generateUUID())

        # Add the identity to the object adapter
        #
        self._adapter.add(self, myID)

        # Create a proxy for the new node and add it as a child to the parent
        #
        thisNode = Filesystem.NodePrx.uncheckedCast(self._adapter.createProxy(myID))
        self._parent.addChild(thisNode)

    # Slice Node::name() operation

    def name(self, current=None):
        return self._name

    # Slice File::reas() operation

    def read(self, current=None):
        return self._lines

    # Slice File::write() operation

    def write(self, text, current=None):
        self._lines = text

    _adapter = None

class Server(Ice.Application):
    def run(self, args):
        # Terminate cleanly on receipt of a signal
        #
        self.shutdownOnInterrupt()

        # Create an object adapter (stored in the _adapter static members)
        #
        adapter = self.communicator().createObjectAdapterWithEndpoints("SimpleFileSystem", "default -p 10000")
        DirectoryI._adapter = adapter
        FileI._adapter = adapter

        # Create the root directory (with name "/" and no parent)
        #
        root = DirectoryI("/", None)

        # Create a file called "README" in the root directory
        #
        file = FileI("README", root)
        text = [ "This file system contains a collection of poetry." ]
        try:
            file.write(text)
        except Filesystem.GenericError, e:
            print e.reason

        # Create a directory called "Coleridge" in the root directory
        #
        coleridge = DirectoryI("Coleridge", root)

        # Create a file called "Kubla_Khan" in the Coleridge directory
        #
        file = FileI("Kubla_Khan", coleridge)
        text = [ "In Xanadu did Kubla Khan",
                 "A stately pleasure-dome decree:",
                 "Where Alph, the sacred river, ran",
                 "Through caverns measureless to man",
                 "Down to a sunless sea." ]
        try:
            file.write(text)
        except Filesystem.GenericError, e:
            print e.reason

        # All objects are created, allow client requests now
        #
        adapter.activate()

        # Wait until we are done
        #
        self.communicator().waitForShutdown()

        if self.interrupted():
            print self.appName() + ": terminating"

        return 0

app = Server()
sys.exit(app.main(sys.argv))
