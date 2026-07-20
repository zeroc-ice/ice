classdef FormatType < uint8
    %FORMATTYPE Specifies the format for marshaling classes and exceptions with the Slice 1.1 encoding.
    %
    %   FormatType Properties:
    %     CompactFormat - Favors compactness, but does not support slicing-off unknown slices during unmarshaling.
    %     SlicedFormat - Allows slicing-off unknown slices during unmarshaling, at the cost of some extra space in the
    %       marshaled data.

    % Copyright (c) ZeroC, Inc.

    % Don't use an enumeration as comparing enumerators with integral values is significantly slower.
    properties (Constant)
        %COMPACTFORMAT Favors compactness, but does not support slicing-off unknown slices during unmarshaling.
        CompactFormat = uint8(0)

        %SLICEDFORMAT Allows slicing-off unknown slices during unmarshaling, at the cost of some extra space in the
        %   marshaled data.
        SlicedFormat = uint8(1)
    end
end
