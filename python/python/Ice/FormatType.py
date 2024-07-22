# Copyright (c) ZeroC, Inc. All rights reserved.

class FormatType(object):
    """
    This enumeration describes the possible formats for classes and exceptions.
    """
    def __init__(self, val):
        assert val >= 0 and val < 3
        self.value = val

FormatType.DefaultFormat = FormatType(0)
FormatType.CompactFormat = FormatType(1)
FormatType.SlicedFormat = FormatType(2)
