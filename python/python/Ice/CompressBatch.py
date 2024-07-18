# Copyright (c) ZeroC, Inc. All rights reserved.

from .EnumBase import EnumBase

class CompressBatch(EnumBase):
    """
        The batch compression option when flushing queued batch requests.
    Enumerators:
    Yes --  Compress the batch requests.
    No --  Don't compress the batch requests.
    BasedOnProxy --  Compress the batch requests if at least one request was made on a compressed proxy.
    """

    def __init__(self, _n, _v):
        EnumBase.__init__(self, _n, _v)

    def valueOf(self, _n):
        if _n in self._enumerators:
            return self._enumerators[_n]
        return None

    valueOf = classmethod(valueOf)

CompressBatch.Yes = CompressBatch("Yes", 0)
CompressBatch.No = CompressBatch("No", 1)
CompressBatch.BasedOnProxy = CompressBatch("BasedOnProxy", 2)
CompressBatch._enumerators = {
    0: CompressBatch.Yes,
    1: CompressBatch.No,
    2: CompressBatch.BasedOnProxy,
}
