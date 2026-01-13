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
    Creates a new communicator.

    This function is provided for backwards compatibility.
    New code should call the :class:`Communicator` constructor directly.

    Parameters
    ----------
    args : list[str] | None, optional
        The command-line arguments.
    eventLoop : asyncio.AbstractEventLoop | None, optional
        An asyncio event loop used to run coroutines and wrap futures.
    initData : InitializationData | None, optional
        Options for the new communicator.
        This argument is mutually exclusive with both the ``args`` and ``eventLoop`` argument.

    Returns
    -------
    Communicator
        The new communicator.
    """
    return Communicator(initData=initData) if initData is not None else Communicator(args, eventLoop)


def identityToString(identity: Identity, toStringMode: ToStringMode | None = None) -> str:
    """
    Converts an Identity into a string using the specified mode.

    Parameters
    ----------
    identity : Ice.Identity
        The identity.
    toStringMode : Ice.ToStringMode | None, optional
        Specifies how to handle non-ASCII characters and non-printable ASCII characters.
        The default is :const:`Ice.ToStringMode.Unicode`.

    Returns
    -------
    str
        The stringified identity.
    """
    return IcePy.identityToString(identity, toStringMode)


def stringToIdentity(str: str) -> Identity:
    """
    Converts a stringified identity into an Identity.

    Parameters
    ----------
    str : str
        The stringified identity.

    Returns
    -------
    Identity
        An Identity created from the provided string.

    Raises
    ------
    ParseException
        If the string cannot be converted to an object identity.
    """
    return IcePy.stringToIdentity(str)


def createProperties(args: list[str] | None = None, defaults: Properties | None = None) -> Properties:
    """
    Creates a property set initialized from command-line arguments and a default property set.

    This function is provided for backwards compatibility.
    New code should call the :class:`Properties` constructor directly.

    Parameters
    ----------
    args : list[str] | None, optional
        The command-line arguments.
    defaults : Properties | None, optional
        Default values for the new property set.

    Returns
    -------
    Properties
        A new property set.
    """
    return Properties(args, defaults)


def getSliceDir() -> str | None:
    """
    This helper function locates the installation directory for the Ice Slice files, and returns its path.
    This path is typically used to configure include paths for Slice compilers.

    Returns
    -------
    str | None
        The absolute path of the directory containing the Ice Slice files, or ``None`` if the directory cannot be found.
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
