# Copyright (c) ZeroC, Inc. All rights reserved.

import IcePy
from .Logger import Logger
from .LoggerI import LoggerI

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
