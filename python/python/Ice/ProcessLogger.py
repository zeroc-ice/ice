# Copyright (c) ZeroC, Inc.

import IcePy

from ._LoggerI import LoggerI
from .Logger import Logger

__name__ = "Ice"


def getProcessLogger() -> Logger:
    """Returns the default logger object."""
    logger = IcePy.getProcessLogger()
    if isinstance(logger, Logger):
        return logger
    else:
        return LoggerI(logger)


def setProcessLogger(logger: Logger):
    """Sets the default logger object."""
    IcePy.setProcessLogger(logger)


__all__ = ["getProcessLogger", "setProcessLogger"]
