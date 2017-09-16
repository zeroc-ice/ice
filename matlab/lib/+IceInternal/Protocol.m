%{
**********************************************************************

Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.

This copy of Ice is licensed to you under the terms described in the
ICE_LICENSE file included in this distribution.

**********************************************************************
%}

classdef Protocol
    properties(Constant)
        FLAG_HAS_TYPE_ID_STRING    = bitshift(1, 0)
        FLAG_HAS_TYPE_ID_INDEX     = bitshift(1, 1)
        FLAG_HAS_TYPE_ID_COMPACT   = bitor(bitshift(1, 0), bitshift(1, 1))
        FLAG_HAS_OPTIONAL_MEMBERS  = bitshift(1, 2)
        FLAG_HAS_INDIRECTION_TABLE = bitshift(1, 3)
        FLAG_HAS_SLICE_SIZE        = bitshift(1, 4)
        FLAG_IS_LAST_SLICE         = bitshift(1, 5)

        OPTIONAL_END_MARKER = hex2dec('ff')

        Encoding_1_0 = Ice.EncodingVersion(1, 0)
        Encoding_1_1 = Ice.EncodingVersion(1, 1)
    end
end
