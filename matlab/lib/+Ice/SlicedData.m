classdef SlicedData < handle
    %SLICEDDATA Holds class slices that cannot be unmarshaled because their types are not known locally.
    %
    %   SlicedData Properties:
    %     slices - The details of each slice, in order of most-derived to least-derived.

    % Copyright (c) ZeroC, Inc.

    properties(SetAccess=immutable)
        %SLICES The details of each slice, in order of most-derived to least-derived.
        %   cell array of SliceInfo objects
        slices (1, :) cell
    end
    methods(Hidden)
        function obj = SlicedData(slices)
            obj.slices = slices;
        end
    end
end
