# Copyright (c) ZeroC, Inc.

from .EnumBase import EnumBase

class CompressBatch(EnumBase):
    """
    The batch compression option when flushing queued batch requests.

    Enumerators:

    - Yes:
        Compress the batch requests.

    - No:
        Don't compress the batch requests.

    - BasedOnProxy:
        Compress the batch requests if at least one request was made on a compressed proxy.
    """

    def __init__(self, _n, _v):
        EnumBase.__init__(self, _n, _v)

    def valueOf(self, value):
        """
        Get the enumerator corresponding to the given value.

        Parameters
        ----------
        value : int
            The enumerator's value.

        Returns
        -------
        CompressBatch or None
            The enumerator corresponding to the given name, or None if no such enumerator exists.
        """
        return self._enumerators[value] if value in self._enumerators else None

    valueOf = classmethod(valueOf)

CompressBatch.Yes = CompressBatch("Yes", 0)
CompressBatch.No = CompressBatch("No", 1)
CompressBatch.BasedOnProxy = CompressBatch("BasedOnProxy", 2)
CompressBatch._enumerators = {
    0: CompressBatch.Yes,
    1: CompressBatch.No,
    2: CompressBatch.BasedOnProxy,
}
