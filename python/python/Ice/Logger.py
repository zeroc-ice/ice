# Copyright (c) ZeroC, Inc. All rights reserved.

class Logger(object):
    """
    The Ice message logger. Applications can provide their own logger by implementing this interface and installing it
    in a communicator.
    """

    def __init__(self):
        if type(self) is Logger:
            raise RuntimeError("Ice.Logger is an abstract class")

    def _print(self, message):
        """
            Print a message. The message is printed literally, without any decorations such as executable name or time
            stamp.
        Arguments:
        message -- The message to log.
        """
        raise NotImplementedError("method '_print' not implemented")

    def trace(self, category, message):
        """
            Log a trace message.
        Arguments:
        category -- The trace category.
        message -- The trace message to log.
        """
        raise NotImplementedError("method 'trace' not implemented")

    def warning(self, message):
        """
            Log a warning message.
        Arguments:
        message -- The warning message to log.
        """
        raise NotImplementedError("method 'warning' not implemented")

    def error(self, message):
        """
            Log an error message.
        Arguments:
        message -- The error message to log.
        """
        raise NotImplementedError("method 'error' not implemented")

    def getPrefix(self):
        """
            Returns this logger's prefix.
        Returns: The prefix.
        """
        raise NotImplementedError("method 'getPrefix' not implemented")

    def cloneWithPrefix(self, prefix):
        """
            Returns a clone of the logger with a new prefix.
        Arguments:
        prefix -- The new prefix for the logger.
        Returns: A logger instance.
        """
        raise NotImplementedError("method 'cloneWithPrefix' not implemented")
