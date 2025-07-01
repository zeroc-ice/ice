classdef OptionalFormat < uint8
    %OPTIONALFORMAT The optional format type.
    %   An optional value is encoded with a specific optional format. This optional format describes how the data is
    %   marshaled and how it can be skipped by the unmarshaling code if the optional is not known to the receiver.
    %
    %   OptionalFormat Properties:
    %     F1 - Fixed-length format (1 byte).
    %     F2 - Fixed-length format (2 bytes).
    %     F4 - Fixed-length format (4 bytes).
    %     F8 - Fixed-length format (8 bytes).
    %     Size - Variable-length format with size.
    %     VSize - Variable-length format with variable size.
    %     FSize - Fixed-length format with size.
    %     Class - Class format.

    % Copyright (c) ZeroC, Inc.

    % Don't use an enumeration as comparing enumerators with integral values is significantly slower.
    properties (Constant)
        %F1 Fixed-length format (1 byte).
        F1 = uint8(0)

        %F2 Fixed-length format (2 bytes).
        F2 = uint8(1)

        %F4 Fixed-length format (4 bytes).
        F4 = uint8(2)

        %F8 Fixed-length format (8 bytes).
        F8 = uint8(3)

        %SIZE Variable-length format with size.
        Size = uint8(4)

        %VSIZE Variable-length format with variable size.
        VSize = uint8(5)

        %FSIZE Fixed-length format with size.
        FSize = uint8(6)

        %CLASS Class format, no longer supported as of Ice 3.8.
        Class = uint8(7)
    end
end
