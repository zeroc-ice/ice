%
% Copyright (c) ZeroC, Inc. All rights reserved.
%

classdef (Abstract) EncapsEncoder < handle
    methods
        function obj = EncapsEncoder(os, encaps)
            obj.os = os;
            obj.encaps = encaps;
            obj.typeIdMap = containers.Map('KeyType', 'char', 'ValueType', 'int32');
            obj.typeIdIndex = 0;
        end

        function r = writeOptional(obj, tag, format)
            r = false;
        end

        function writePendingValues(obj)
            %
            % Overridden for the 1.0 encoding, not necessary for subsequent encodings.
            %
        end
    end
    methods(Abstract)
        writeValue(obj, v)
        startInstance(obj, sliceType, slicedData)
        endInstance(obj)
        startSlice(obj, typeId, compactId, last)
        endSlice(obj)
    end
    methods(Access=protected)
        function r = registerTypeId(obj, typeId)
            %
            % The map raises an exception if the key isn't present.
            %
            try
                r = obj.typeIdMap(typeId);
            catch ex
                obj.typeIdIndex = obj.typeIdIndex + 1;
                obj.typeIdMap(typeId) = obj.typeIdIndex;
                r = -1;
            end
        end
    end
    properties(Access=protected)
        os
        encaps
    end
    properties(Access=private)
        typeIdMap
        typeIdIndex
    end
end
