# Copyright (c) ZeroC, Inc.

from .Exception import Exception as IceException


class UserException(IceException):
    """
    The base class for all user-defined exceptions.
    """

    def __str__(self):
        # Types derived from UserException are data classes. We override __str__to delegate to the generated __repr__,
        # since the built-in __str__ from BaseException only works when all arguments are passed to the constructor,
        # which is not the case for exceptions unmarshalled from the wire.
        return self.__repr__()


__all__ = ["UserException"]
