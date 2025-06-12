% Copyright (c) ZeroC, Inc.

classdef EncapsEncoder11_InstanceData < handle
    methods
        function obj = EncapsEncoder11_InstanceData(previous)
            if ~isempty(previous)
                previous.next = obj;
            end
            obj.previous = previous;
            obj.next = [];
            obj.indirectionTable = {};
        end
    end
    properties
        % Instance attributes
        sliceType
        firstSlice
        % Slice attributes
        sliceFlags
        writeSlice    % Position of the slice data members
        sliceFlagsPos % Position of the slice flags
        indirectionTable
        indirectionMap dictionary = configureDictionary('int32', 'int32')
        previous
        next
    end
end
