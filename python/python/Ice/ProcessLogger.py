# Copyright (c) ZeroC, Inc.

import IcePy
from .Logger import Logger
from ._LoggerI import LoggerI

__name__ = "Ice"

def getProcessLogger():
    """Returns the default logger object."""
    logger = IcePy.getProcessLogger()
    if isinstance(logger, Logger):
        return logger
    else:
        return LoggerI(logger)

def setProcessLogger(logger):
    """Sets the default logger object."""
    IcePy.setProcessLogger(logger)

__all__ = ["getProcessLogger", "setProcessLogger"]
