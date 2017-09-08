%{
**********************************************************************

Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.

This copy of Ice is licensed to you under the terms described in the
ICE_LICENSE file included in this distribution.

**********************************************************************
%}

classdef InstanceData < handle
    methods
        function obj = InstanceData(p)
            if ~isempty(p)
                obj.previous = p;
                p.next = obj;
            end
            obj.sliceFlags = uint8(0);
        end
    end
    properties
        sliceType
        skipFirstSlice
        slices
        indirectionTables
        sliceFlags
        sliceSize
        typeId
        compactId
        indirectPatchList
        previous
        next
    end
end
