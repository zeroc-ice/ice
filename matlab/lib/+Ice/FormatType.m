classdef FormatType < uint32
    % FormatType   Summary of FormatType
    %
    % This enumeration describes the possible formats for classes and exceptions.

    % Copyright (c) ZeroC, Inc. All rights reserved.

    % Don't use an enumeration as comparing enumerators with integral values is significantly slower.
    properties(Constant)
        % DefaultFormat   Indicates that no preference was specified.
        DefaultFormat = uint8(0)

        % CompactFormat   A minimal format that eliminates the possibility
        %   for slicing unrecognized types.
        CompactFormat = uint8(1)

        % SlicedFormat   Allow slicing and preserve slices for unknown types.
        SlicedFormat = uint8(2)
    end
end
