classdef OptionalFormat < uint8
    % OptionalFormat   Summary of OptionalFormat
    %
    % The optional type.
    %
    % An optional value is encoded with a specific optional format. This optional
    % format describes how the data is encoded and how it can be skipped by the
    % unmarshaling code if the optional is not known to the receiver.

    % Copyright (c) ZeroC, Inc. All rights reserved.

    % Don't use an enumeration as comparing enumerators with integral values is significantly slower.
    properties(Constant)
        F1 = uint8(0)
        F2 = uint8(1)
        F4 = uint8(2)
        F8 = uint8(3)
        Size = uint8(4)
        VSize = uint8(5)
        FSize = uint8(6)
        Class = uint8(7)
    end
end
