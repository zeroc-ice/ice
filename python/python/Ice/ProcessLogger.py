# Copyright (c) ZeroC, Inc.

import IcePy

from ._LoggerI import LoggerI
from .Logger import Logger

__name__ = "Ice"


def getProcessLogger() -> Logger:
    """
    Gets the per-process logger.

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
    Sets the per-process logger. Communicators created after this call use this logger unless a logger is set in
    :class:`InitializationData` or configured through logger properties such as ``Ice.LogFile``.

    Parameters
    ----------
    logger : Ice.Logger
        The new per-process logger instance.
    """
    IcePy.setProcessLogger(logger)


__all__ = ["getProcessLogger", "setProcessLogger"]
