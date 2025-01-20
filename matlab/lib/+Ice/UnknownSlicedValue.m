classdef UnknownSlicedValue < Ice.Value
    % UnknownSlicedValue   Summary of UnknownSlicedValue
    %
    % UnknownSlicedValue holds an instance of an unknown Slice class type.
    % Call the constructor with the Slice type ID of the unknown value type.
    %

    %  Copyright (c) ZeroC, Inc.

    methods
        function obj = UnknownSlicedValue(unknownTypeId)
            obj.unknownTypeId = unknownTypeId;
        end

        function id = ice_id(obj)
            id = obj.unknownTypeId;
        end
    end
    properties(Access=private)
        unknownTypeId
    end
end
