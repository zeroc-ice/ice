%{
**********************************************************************

Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.

This copy of Ice is licensed to you under the terms described in the
ICE_LICENSE file included in this distribution.

**********************************************************************
%}

%
% SlicedData holds the slices of unknown types.
%
classdef SlicedData < handle
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
