# Copyright (c) ZeroC, Inc.

from typing import Self, final

import IcePy

from .Logger import Logger


@final
class LoggerI(Logger):
    def __init__(self, impl: IcePy.Logger):
        self._impl = impl

    def print(self, message: str):
        return self._impl.print(message)

    def trace(self, category: str, message: str):
        return self._impl.trace(category, message)

    def warning(self, message: str):
        return self._impl.warning(message)

    def error(self, message: str):
        return self._impl.error(message)

    def getPrefix(self) -> str:
        return self._impl.getPrefix()

    def cloneWithPrefix(self, prefix: str) -> Logger:
        logger: IcePy.Logger = self._impl.cloneWithPrefix(prefix)
        return LoggerI(logger)
