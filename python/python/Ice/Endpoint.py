# Copyright (c) ZeroC, Inc. All rights reserved.

class Endpoint(object):
    """
    The user-level interface to an endpoint.
    """

    def __init__(self):
        if type(self) == Endpoint:
            raise RuntimeError("Ice.Endpoint is an abstract class")

    def toString(self):
        """
            Return a string representation of the endpoint.
        Returns: The string representation of the endpoint.
        """
        raise NotImplementedError("method 'toString' not implemented")

    def getInfo(self):
        """
            Returns the endpoint information.
        Returns: The endpoint information class.
        """
        raise NotImplementedError("method 'getInfo' not implemented")
