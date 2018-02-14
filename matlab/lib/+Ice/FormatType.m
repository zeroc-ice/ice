classdef FormatType < uint32
    % FormatType   Summary of FormatType
    %
    % This enumeration describes the possible formats for classes and exceptions.

    % Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.

    enumeration
        % DefaultFormat   Indicates that no preference was specified.
        DefaultFormat (0)

        % CompactFormat   A minimal format that eliminates the possibility
        %   for slicing unrecognized types.
        CompactFormat (1)

        % SlicedFormat   Allow slicing and preserve slices for unknown types.
        SlicedFormat (2)
    end
end
