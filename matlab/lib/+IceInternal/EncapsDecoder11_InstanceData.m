classdef EncapsDecoder11_InstanceData < handle
    % Copyright (c) ZeroC, Inc.

    methods
        function obj = EncapsDecoder11_InstanceData(p)
            if ~isempty(p)
                obj.previous = p;
                p.next = obj;
            end
            obj.slices = {};
            obj.indirectionTables = {};
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
        typeIdIndex
        compactId
        indirectPatchList dictionary = dictionary % unconfigured dictionary
        previous
        next
    end
end
