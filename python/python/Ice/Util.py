# -*- coding: utf-8 -*-
#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

import os
import sys
import IcePy
from .PropertiesI import PropertiesI
from .CommunicatorI import CommunicatorI


def initialize(args=None, data=None):
    """Initializes a new communicator. The optional arguments represent
    an argument list (such as sys.argv) and an instance of InitializationData.
    You can invoke this function as follows:

    Ice.initialize()
    Ice.initialize(args)
    Ice.initialize(data)
    Ice.initialize(args, data)

    If you supply an argument list, the function removes those arguments from
    the list that were recognized by the Ice run time."""
    communicator = IcePy.Communicator(args, data)
    return CommunicatorI(communicator)



def identityToString(id, toStringMode=None):
    return IcePy.identityToString(id, toStringMode)


def stringToIdentity(str):
    return IcePy.stringToIdentity(str)


def createProperties(args=None, defaults=None):
    """Creates a new property set. The optional arguments represent
    an argument list (such as sys.argv) and a property set that supplies
    default values. You can invoke this function as follows:

    Ice.createProperties()
    Ice.createProperties(args)
    Ice.createProperties(defaults)
    Ice.createProperties(args, defaults)

    If you supply an argument list, the function removes those arguments
    from the list that were recognized by the Ice run time."""

    properties = IcePy.createProperties(args, defaults)
    return PropertiesI(properties)


def getSliceDir():
    """Convenience function for locating the directory containing the Slice files."""

    #
    # Get the parent of the directory containing this file (__init__.py).
    #
    pyHome = os.path.join(os.path.dirname(__file__), "..")

    #
    # Detect setup.py installation in site-packages. The slice
    # files live one level above this file.
    #
    dir = os.path.join(pyHome, "slice")
    if os.path.isdir(dir):
        return dir

    #
    # For an installation from a source distribution, a binary tarball, or a
    # Windows installer, the "slice" directory is a sibling of the "python"
    # directory.
    #
    dir = os.path.join(pyHome, "..", "slice")
    if os.path.exists(dir):
        return os.path.normpath(dir)

    #
    # In a source distribution, the "slice" directory is an extra level higher.
    # directory.
    #
    dir = os.path.join(pyHome, "..", "..", "slice")
    if os.path.exists(dir):
        return os.path.normpath(dir)

    if sys.platform[:5] == "linux":
        #
        # Check the default Linux location.
        #
        dir = os.path.join("/", "usr", "share", "ice", "slice")
        if os.path.exists(dir):
            return dir

    elif sys.platform == "darwin":
        #
        # Check the default macOS homebrew location.
        #
        dir = os.path.join("/", "usr", "local", "share", "ice", "slice")
        if os.path.exists(dir):
            return dir

    return None


# TODO refactor generated code to use builtins directly for example
# import builtins as __builtins, __builtins.type(xxx)

#
# Used by generated code. Defining these in the Ice module means the generated code
# can avoid the need to qualify the type() and hash() functions with their module
# names. Since the functions are in the __builtin__ module (for Python 2.x) and the
# builtins module (for Python 3.x), it's easier to define them here.
#


def getType(o):
    return type(o)


#
# Used by generated code. Defining this in the Ice module means the generated code
# can avoid the need to qualify the hash() function with its module name. Since
# the function is in the __builtin__ module (for Python 2.x) and the builtins
# module (for Python 3.x), it's easier to define it here.
#


def getHash(o):
    return hash(o)
