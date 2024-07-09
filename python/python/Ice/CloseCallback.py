# Copyright (c) ZeroC, Inc. All rights reserved.

class CloseCallback(object):
    """
    An application can implement this interface to receive notifications when a connection closes.
    """

    def __init__(self):
        if type(self) == CloseCallback:
            raise RuntimeError("Ice.CloseCallback is an abstract class")

    def closed(self, con):
        """
            This method is called by the connection when the connection is closed. If the callback needs more information
            about the closure, it can call Connection#throwException.
        Arguments:
        con -- The connection that closed.
        """
        raise NotImplementedError("method 'closed' not implemented")
