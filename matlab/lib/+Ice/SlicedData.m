classdef SlicedData < handle
    % SlicedData   Summary of SlicedData
    %
    % Holds class slices that cannot be unmarshaled because their types are not known locally.
    %
    % SlicedData Properties:
    %   slices - The details of each slice, in order of most-derived to least-derived.

    % Copyright (c) ZeroC, Inc.

    properties(SetAccess=immutable)
        slices (1, :) cell % cell array of SliceInfo objects
    end
    methods
        function obj = SlicedData(slices)
            obj.slices = slices;
        end
    end
end
