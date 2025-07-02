# Copyright (c) ZeroC, Inc.

from .Logger import Logger

class LoggerI(Logger):
    def __init__(self, impl):
        self._impl = impl

    def print(self, message):
        return self._impl.print(message)

    def trace(self, category, message):
        return self._impl.trace(category, message)

    def warning(self, message):
        return self._impl.warning(message)

    def error(self, message):
        return self._impl.error(message)

    def getPrefix(self):
        return self._impl.getPrefix()

    def cloneWithPrefix(self, prefix):
        logger = self._impl.cloneWithPrefix(prefix)
        return LoggerI(logger)
