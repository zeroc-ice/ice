# Copyright (c) ZeroC, Inc. All rights reserved.

from .ImplicitContext import ImplicitContext

class ImplicitContextI(ImplicitContext):
    def __init__(self, impl):
        self._impl = impl

    def setContext(self, ctx):
        self._impl.setContext(ctx)

    def getContext(self):
        return self._impl.getContext()

    def containsKey(self, key):
        return self._impl.containsKey(key)

    def get(self, key):
        return self._impl.get(key)

    def put(self, key, value):
        return self._impl.put(key, value)

    def remove(self, key):
        return self._impl.remove(key)
