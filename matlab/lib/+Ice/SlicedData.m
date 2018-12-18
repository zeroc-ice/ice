classdef SlicedData < handle
    % SlicedData   Summary of SlicedData
    %
    % SlicedData holds the slices of unknown types.
    %
    % SlicedData Properties:
    %   slices - The details of each slice, in order of most-derived to
    %     least-derived.

    % Copyright (c) 2003-present ZeroC, Inc. All rights reserved.

    properties(SetAccess=private)
        % The details of each slice, in order of most-derived to least-derived.
        slices
    end
    methods
        function obj = SlicedData(slices)
            obj.slices = slices;
        end
    end
end
