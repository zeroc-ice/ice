# Copyright (c) ZeroC, Inc.

class Logger:
    """
    The Ice message logger.

    Applications can provide their own logger by implementing this interface and installing it in a communicator.
    """

    def __init__(self):
        if type(self) is Logger:
            raise RuntimeError("Ice.Logger is an abstract class")

    def _print(self, message):
        """
        Print a message.

        The message is printed literally, without any decorations such as executable name or timestamp.

        Parameters
        ----------
        message : str
            The message to log.
        """
        raise NotImplementedError("method '_print' not implemented")

    def trace(self, category, message):
        """
        Log a trace message.

        Parameters
        ----------
        category : str
            The trace category.
        message : str
            The trace message to log.
        """
        raise NotImplementedError("method 'trace' not implemented")

    def warning(self, message):
        """
        Log a warning message.

        Parameters
        ----------
        message : str
            The warning message to log.
        """
        raise NotImplementedError("method 'warning' not implemented")

    def error(self, message):
        """
        Log an error message.

        Parameters
        ----------
        message : str
            The error message to log.
        """
        raise NotImplementedError("method 'error' not implemented")

    def getPrefix(self):
        """
        Get this logger's prefix.

        Returns
        -------
        str
            The prefix of this logger.
        """
        raise NotImplementedError("method 'getPrefix' not implemented")

    def cloneWithPrefix(self, prefix):
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
        raise NotImplementedError("method 'cloneWithPrefix' not implemented")
