classdef (Hidden) Protocol
    % Copyright (c) ZeroC, Inc.

    properties (Constant)
        FLAG_HAS_TYPE_ID_STRING    = bitshift(uint8(1), 0)
        FLAG_HAS_TYPE_ID_INDEX     = bitshift(uint8(1), 1)
        FLAG_HAS_TYPE_ID_COMPACT   = bitor(bitshift(uint8(1), 0), bitshift(uint8(1), 1))
        FLAG_HAS_OPTIONAL_MEMBERS  = bitshift(uint8(1), 2)
        FLAG_HAS_INDIRECTION_TABLE = bitshift(uint8(1), 3)
        FLAG_HAS_SLICE_SIZE        = bitshift(uint8(1), 4)
        FLAG_IS_LAST_SLICE         = bitshift(uint8(1), 5)

        OPTIONAL_END_MARKER        = uint8(255)

        Encoding_1_0 = Ice.EncodingVersion(1, 0)
        Encoding_1_1 = Ice.EncodingVersion(1, 1)
        CurrentEncoding = Ice.EncodingVersion(1, 1)
    end
end
