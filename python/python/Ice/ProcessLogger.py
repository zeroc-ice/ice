# Copyright (c) ZeroC, Inc. All rights reserved.

from IcePy import getProcessLogger as _getProcessLogger, setProcessLogger as _setProcessLogger
from .Logger import Logger
from .LoggerI import LoggerI

def getProcessLogger():
    """Returns the default logger object."""
    logger = _getProcessLogger()
    if isinstance(logger, Logger):
        return logger
    else:
        return LoggerI(logger)


def setProcessLogger(logger):
    """Sets the default logger object."""
    _setProcessLogger(logger)
