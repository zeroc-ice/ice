# Copyright (c) ZeroC, Inc.


import asyncio
import os
import sys
import IcePy
from .InitializationData import InitializationData
from .Properties import Properties
from .Communicator import Communicator
from .LocalExceptions import InitializationException
from .asyncio.EventLoopAdapter import EventLoopAdapter

__name__ = "Ice"


def initialize(args=None, initData=None, configFile=None, eventLoop=None):
    """
    Creates a new communicator.

    Parameters
    ----------
    args : list of str, optional
        The command-line arguments. This function parses arguments starting with `--` and one of the
        reserved prefixes (Ice, IceSSL etc.) as properties for the new communicator. If there is an argument starting
        with `--Ice.Config`, this function loads the specified configuration file. When the same property is set in a
        configuration file and through a command-line argument, the command-line setting takes precedence.
    initData : InitializationData, optional
        Options for the new communicator. This argument and the `configFile` argument are mutually exclusive.
    configFile : str, optional
        The path to a configuration file. This argument and the `initData` argument are mutually exclusive.
    eventLoop : asyncio.AbstractEventLoop, optional
        An asyncio event loop used to run coroutines and wrap futures. If provided, a new event loop adapter is created
        and configured with the communicator. This adapter is responsible for executing coroutines returned by Ice
        asynchronous dispatch methods and for wrapping Ice futures (from Ice Async APIs) into asyncio futures.
        This argument and the `initData` argument are mutually exclusive. If the `initData` argument is provided, the
        event loop adapter can be set using the InitializationData.eventLoopAdapter member.

    Returns
    -------
    Communicator
        The new communicator.

    .. code-block:: python

    with Ice.initialize(sys.argv, eventLoop=asyncio.get_running_loop()) as communicator:
        greeter = VisitorCenter.GreeterPrx(communicator, "greeter:tcp -h localhost -p 4061")
        await greeter.greetAsync()
    """

    if args and not isinstance(args, list):
        raise InitializationException("args must be a list of strings")

    if initData and not isinstance(initData, InitializationData):
        raise InitializationException("initData must be an instance of Ice.InitializationData")

    if initData and configFile:
        raise InitializationException("Both initData and configFile arguments cannot be specified")

    if initData and eventLoop:
        raise InitializationException("Both initData and eventLoop arguments cannot be specified")

    eventLoopAdapter = initData.eventLoopAdapter if initData else None
    if eventLoop:

        if not isinstance(eventLoop, asyncio.AbstractEventLoop):
            raise InitializationException("The event loop must be an instance of asyncio.AbstractEventLoop")
        eventLoopAdapter = EventLoopAdapter(eventLoop)

    communicator = IcePy.Communicator(args, initData, configFile)
    return Communicator(communicator, eventLoopAdapter)

def identityToString(identity, toStringMode=None):
    """
    Convert an object identity to a string.

    Parameters
    ----------
    identity : Ice.Identity
        The object identity to convert.
    toStringMode : Ice.ToStringMode, optional
        Specifies if and how non-printable ASCII characters are escaped in the result.

    Returns
    -------
    str
        The string representation of the object identity.
    """
    return IcePy.identityToString(identity, toStringMode)


def stringToIdentity(str):
    """
    Convert a string to an object identity.

    Parameters
    ----------
    str : str
        The string to convert.

    Returns
    -------
    Ice.Identity
        The converted object identity.

    Raises
    ------
    ParseException
        If the string cannot be converted to an object identity.
    """
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
    return Properties(properties)


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

__all__ = [ "initialize", "identityToString", "stringToIdentity", "createProperties", "getSliceDir" ]
