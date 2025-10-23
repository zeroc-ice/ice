# Copyright (c) ZeroC, Inc.

from __future__ import annotations

import asyncio
import os
import sys
from typing import TYPE_CHECKING, overload

import IcePy

from .asyncio.EventLoopAdapter import EventLoopAdapter as AsyncIOEventLoopAdapter
from .Communicator import Communicator
from .InitializationData import InitializationData
from .Properties import Properties

if TYPE_CHECKING:
    import asyncio

    from Ice import Communicator, Identity, InitializationData, Properties, ToStringMode

__name__ = "Ice"


@overload
def initialize(args: list[str] | None = None, eventLoop: asyncio.AbstractEventLoop | None = None) -> Communicator: ...


@overload
def initialize(*, initData: InitializationData | None = None) -> Communicator: ...


def initialize(
    args: list[str] | None = None,
    eventLoop: asyncio.AbstractEventLoop | None = None,
    initData: InitializationData | None = None,
) -> Communicator:
    """
    Creates a new communicator. This function is provided for backwards compatibility. New code should call the
    :class:`Communicator` constructor directly.

    Parameters
    ----------
    args : list of str, optional
        The command-line arguments.
    eventLoop : asyncio.AbstractEventLoop, optional
        An asyncio event loop used to run coroutines and wrap futures. This argument and the `initData` argument are
        mutually exclusive.
    initData : InitializationData, optional
        Options for the new communicator. This argument and the `args` argument are mutually exclusive.

    Returns
    -------
    Communicator
        The new communicator.
    """
    if args is not None or eventLoop is not None:
        return Communicator(args, eventLoop)
    else:
        return Communicator(initData=initData)


def identityToString(identity: Identity, toStringMode: ToStringMode | None = None) -> str:
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


def stringToIdentity(str: str) -> Identity:
    """
    Convert a string to an object identity.

    Parameters
    ----------
    str : str
        The string to convert.

    Returns
    -------
    Identity
        The converted object identity.

    Raises
    ------
    ParseException
        If the string cannot be converted to an object identity.
    """
    return IcePy.stringToIdentity(str)


def createProperties(args: list[str] | None = None, defaults: Properties | None = None) -> Properties:
    """
    Creates a new property set.

    This function creates a new Ice property set. You can optionally provide a command-line argument list (such as
    ``sys.argv``) and/or a dictionary of default property values.

    If an argument list is supplied, this function parses arguments starting with ``--`` and a known Ice prefix
    (e.g., ``Ice``, ``IceSSL``), and removes recognized arguments from the list.

    Parameters
    ----------
    args : list[str], optional
        A list of command-line arguments, such as ``sys.argv``. Arguments that match Ice runtime options are parsed
        into properties and removed from the list.
    defaults : dict[str, str], optional
        A dictionary representing default property values.

    Returns
    -------
    Properties
        A new Ice property set instance.

    Examples
    --------
    .. code-block:: python

        # Create a new empty property set.
        properties = Ice.createProperties()

        # Create a property set from command-line arguments.
        properties = Ice.createProperties(sys.argv)

        # Create a property set using default values.
        defaults = {"Ice.Trace.Protocol": "1"}
        properties = Ice.createProperties(defaults)

        # Combine command-line parsing with default values.
        defaults = {"Ice.Trace.Protocol": "1"}
        properties = Ice.createProperties(sys.argv, defaults)
    """
    properties = IcePy.createProperties(args, defaults)
    return Properties(properties)


def getSliceDir() -> str | None:
    """
    Returns the path to the directory where the Ice Slice files are installed.

    This helper function locates the installation directory for the Ice Slice files,
    typically used to configure include paths for Slice compilers.

    Returns
    -------
    str or None
        The absolute path to the directory containing the Ice Slice files, or ``None`` if the directory cannot be found.
    """

    # Get the parent of the directory containing this file (__init__.py).
    pyHome = os.path.join(os.path.dirname(__file__), "..")

    # Detect setup.py installation in site-packages. The slice files live one level above this file.
    dir = os.path.join(pyHome, "slice")
    if os.path.isdir(dir):
        return dir

    # For an installation from a source distribution the "slice" directory is a sibling of the "python"
    # directory.
    dir = os.path.join(pyHome, "..", "slice")
    if os.path.exists(dir):
        return os.path.normpath(dir)

    # In a source distribution, the "slice" directory is an extra level higher.
    dir = os.path.join(pyHome, "..", "..", "slice")
    if os.path.exists(dir):
        return os.path.normpath(dir)

    if sys.platform[:5] == "linux":
        # Check the default Linux location.
        dir = os.path.join("/", "usr", "share", "ice", "slice")
        if os.path.exists(dir):
            return dir

    elif sys.platform == "darwin":
        # Check the default macOS homebrew location.
        dir = os.path.join("/", "usr", "local", "share", "ice", "slice")
        if os.path.exists(dir):
            return dir

    return None


__all__ = ["initialize", "identityToString", "stringToIdentity", "createProperties", "getSliceDir"]
