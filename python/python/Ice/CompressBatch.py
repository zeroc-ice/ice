# Copyright (c) ZeroC, Inc.


from enum import Enum


class CompressBatch(Enum):
    """
    Represents batch compression options for flushing queued batch requests.
    """

    Yes = 0
    """
    Compress the batch requests.
    """

    No = 1
    """
    Don't compress the batch requests.
    """

    BasedOnProxy = 2
    """
    Compress the batch requests if at least one request was made on a compressed proxy.
    """
