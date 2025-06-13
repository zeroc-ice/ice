% Copyright (c) ZeroC, Inc.

classdef (Abstract) EncapsEncoder < handle
    methods
        function obj = EncapsEncoder(os, encaps)
            obj.os = os;
            obj.encaps = encaps;
            obj.typeIdIndex = 0;
        end

        function r = writeOptional(~, ~, ~)
            r = false;
        end

        function writePendingValues(~)
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
            if isKey(obj.typeIdMap, typeId)
                r = obj.typeIdMap(typeId);
            else
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
        typeIdMap dictionary = configureDictionary('char', 'int32')
        typeIdIndex
    end
end
