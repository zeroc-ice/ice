# Copyright (c) ZeroC, Inc.

class EnumBase:
    def __init__(self, _n, _v):
        self._name = _n
        self._value = _v

    def __hash__(self):
        return self._value

    def __lt__(self, other):
        if isinstance(other, self.__class__):
            return self._value < other._value
        elif other is None:
            return False
        return NotImplemented

    def __le__(self, other):
        if isinstance(other, self.__class__):
            return self._value <= other._value
        elif other is None:
            return False
        return NotImplemented

    def __eq__(self, other):
        if isinstance(other, self.__class__):
            return self._value == other._value
        elif other is None:
            return False
        return NotImplemented

    def __ne__(self, other):
        if isinstance(other, self.__class__):
            return self._value != other._value
        elif other is None:
            return False
        return NotImplemented

    def __gt__(self, other):
        if isinstance(other, self.__class__):
            return self._value > other._value
        elif other is None:
            return False
        return NotImplemented

    def __ge__(self, other):
        if isinstance(other, self.__class__):
            return self._value >= other._value
        elif other is None:
            return False
        return NotImplemented

    def _getName(self):
        return self._name

    def _getValue(self):
        return self._value

    def __str__(self):
        return repr(self)

    name = property(_getName)
    value = property(_getValue)
