# Copyright (c) ZeroC, Inc. All rights reserved.

import os
import sys
import IcePy
from .PropertiesI import PropertiesI
from .CommunicatorI import CommunicatorI

__name__ = "Ice"


def initialize(args=None, data=None):
    """
    Initializes a new communicator.

    This method initializes a new Ice communicator. The optional arguments represent
    an argument list (such as sys.argv) and an instance of InitializationData.
    You can invoke this function as follows:

    - `Ice.initialize()`
    - `Ice.initialize(args)`
    - `Ice.initialize(data)`
    - `Ice.initialize(args, data)`

    If you supply an argument list, the function removes those arguments from the list that were
    recognized by the Ice run time.

    Parameters
    ----------
    args : list, optional
        An argument list, such as sys.argv.
    data : InitializationData, optional
        An instance of InitializationData.

    Returns
    -------
    Communicator
        A new communicator instance.
    """
    communicator = IcePy.Communicator(args, data)
    return CommunicatorI(communicator)

def identityToString(id, toStringMode=None):
    return IcePy.identityToString(id, toStringMode)


def stringToIdentity(str):
    return IcePy.stringToIdentity(str)


def createProperties(args=None, defaults=None):
    """
    Creates a new property set.

    This method creates a new set of properties. The optional arguments represent
    an argument list (such as sys.argv) and a property set that supplies default values.
    You can invoke this function as follows:

    - Ice.createProperties()
    - Ice.createProperties(args)
    - Ice.createProperties(defaults)
    - Ice.createProperties(args, defaults)

    If you supply an argument list, the function removes those arguments from the list that were
    recognized by the Ice runtime.

    Parameters
    ----------
    args : list, optional
        An argument list, such as sys.argv.
    defaults : dict, optional
        A property set that supplies default values.

    Returns
    -------
    Properties
        A new property set instance.
    """
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
