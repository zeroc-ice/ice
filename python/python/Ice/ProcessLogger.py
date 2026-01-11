# Copyright (c) ZeroC, Inc.

import IcePy

from ._LoggerI import LoggerI
from .Logger import Logger

__name__ = "Ice"


def getProcessLogger() -> Logger:
    """
    Gets the per-process logger. This logger is used by all communicators that do not have their own specific logger
    configured at the time the communicator is created.

    Returns
    -------
    Ice.Logger
        The current per-process logger instance.
    """
    logger = IcePy.getProcessLogger()
    if isinstance(logger, Logger):
        return logger
    else:
        return LoggerI(logger)


def setProcessLogger(logger: Logger):
    """
    Sets the per-process logger. This logger is used by all communicators that do not have their own specific logger
    configured at the time the communicator is created.

    Parameters
    ----------
    logger : Ice.Logger
        The new per-process logger instance.
    """
    IcePy.setProcessLogger(logger)


__all__ = ["getProcessLogger", "setProcessLogger"]
