
from .Current import Current

class CurrentI(Current):

    def __init__(self, impl):
        self._impl = impl

    @property
    def adapter(self):
        return self._impl.adapter

    @property
    def con(self):
        return self._impl.con

    @property
    def id(self):
        return self._impl.id

    @property
    def facet(self):
        return self._impl.facet

    @property
    def operation(self):
        return self._impl.operation

    @property
    def mode(self):
        return self._impl.mode

    @property
    def ctx(self):
        return self._impl.ctx

    @property
    def requestId(self):
        return self._impl.requestId

    @property
    def encoding(self):
        return self._impl.encoding

    __module__ = "Ice"
