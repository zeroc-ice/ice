# Copyright (c) ZeroC, Inc.


from enum import Enum


class CompressBatch(Enum):
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

    Yes = 0
    No = 1
    BasedOnProxy = 2
