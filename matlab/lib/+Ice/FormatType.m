classdef FormatType < uint8
    % FormatType   Summary of FormatType
    %
    % This enumeration describes the possible formats for classes and exceptions.

    % Copyright (c) ZeroC, Inc.

    % Don't use an enumeration as comparing enumerators with integral values is significantly slower.
    properties(Constant)
        % CompactFormat   A minimal format that eliminates the possibility
        %   for slicing unrecognized types.
        CompactFormat = uint8(0)

        % SlicedFormat   Allow slicing and preserve slices for unknown types.
        SlicedFormat = uint8(1)
    end
end
