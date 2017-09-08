%{
**********************************************************************

Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.

This copy of Ice is licensed to you under the terms described in the
ICE_LICENSE file included in this distribution.

**********************************************************************
%}

classdef (Abstract) EncapsDecoder < handle
    methods
        function obj = EncapsDecoder(is, encaps)
            obj.is = is;
            obj.encaps = encaps;
            obj.typeIdMap = containers.Map('KeyType', 'int32', 'ValueType', 'char');
            obj.typeIdIndex = 0;
        end
        function r = readOptional(tag, format)
            r = false;
        end
    end
    methods(Abstract)
        throwException(obj)
        startInstance(obj, sliceType)
        endInstance(obj) % TODO: SlicedData
        r = startSlice(obj)
        endSlice(obj)
        skipSlice(obj)
    end
    methods(Access=protected)
        function r = readTypeId(obj, isIndex)
            if isIndex
                index = obj.is.readSize();
                if obj.typeIdMap.isKey(index)
                    r = obj.typeIdMap(index);
                else
                    throw(Ice.UnmarshalOutOfBoundsException());
                end
            else
                r = obj.is.readString();
                obj.typeIdIndex = obj.typeIdIndex + 1;
                obj.typeIdMap(obj.typeIdIndex) = r;
            end
        end
    end
    properties(Access=protected)
        is
        encaps
    end
    properties(Access=private)
        typeIdMap
        typeIdIndex
    end
end
