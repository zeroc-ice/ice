classdef OptionalFormat < uint32
    % OptionalFormat   Summary of OptionalFormat
    %
    % The optional type.
    %
    % An optional value is encoded with a specific optional format. This optional
    % format describes how the data is encoded and how it can be skipped by the
    % unmarshaling code if the optional is not known to the receiver.

    % Copyright (c) ZeroC, Inc. All rights reserved.

    enumeration
        F1 (0)
        F2 (1)
        F4 (2)
        F8 (3)
        Size (4)
        VSize (5)
        FSize (6)
        Class (7)
    end
end
