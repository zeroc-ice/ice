# Copyright (c) ZeroC, Inc.

class FormatType:
    """
    This enumeration describes the possible formats for classes and exceptions.
    """

    def __init__(self, val):
        assert val >= 0 and val < 3
        self.value = val

FormatType.CompactFormat = FormatType(0)
FormatType.SlicedFormat = FormatType(1)
