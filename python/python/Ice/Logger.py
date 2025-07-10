# Copyright (c) ZeroC, Inc.

from abc import ABC, abstractmethod


class Logger(ABC):
    """
    The Ice message logger.

    Applications can provide their own logger by implementing this interface and installing it in a communicator.
    """

    @abstractmethod
    def print(self, message: str):
        """
        Print a message.

        The message is printed literally, without any decorations such as executable name or timestamp.

        Parameters
        ----------
        message : str
            The message to log.
        """
        pass

    # An alias for print, provided for backwards compatibility.
    # New code should call print() directly.
    def _print(self, message: str):
        self.print(message)

    @abstractmethod
    def trace(self, category: str, message: str):
        """
        Log a trace message.

        Parameters
        ----------
        category : str
            The trace category.
        message : str
            The trace message to log.
        """
        pass

    @abstractmethod
    def warning(self, message: str):
        """
        Log a warning message.

        Parameters
        ----------
        message : str
            The warning message to log.
        """
        pass

    @abstractmethod
    def error(self, message: str):
        """
        Log an error message.

        Parameters
        ----------
        message : str
            The error message to log.
        """
        pass

    @abstractmethod
    def getPrefix(self) -> str:
        """
        Get this logger's prefix.

        Returns
        -------
        str
            The prefix of this logger.
        """
        pass

    @abstractmethod
    def cloneWithPrefix(self, prefix: str) -> "Logger":
        """
        Return a clone of the logger with a new prefix.

        Parameters
        ----------
        prefix : str
            The new prefix for the logger.

        Returns
        -------
        Logger
            A new logger instance with the specified prefix.
        """
        pass
