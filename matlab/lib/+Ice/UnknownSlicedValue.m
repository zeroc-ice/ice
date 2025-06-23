classdef UnknownSlicedValue < Ice.Value
    %UNKNOWNSLICEDVALUE Represents an instance of an unknown class.
    %
    %   UnknownSlicedValue Methods:
    %     ice_id - Returns the Slice type ID associated with this instance.

    % Copyright (c) ZeroC, Inc.

    methods(Hidden)
        function obj = UnknownSlicedValue(unknownTypeId)
            obj.unknownTypeId = unknownTypeId;
        end
    end
    methods
        function id = ice_id(obj)
            id = obj.unknownTypeId;
        end
    end
    properties(Access=private)
        unknownTypeId
    end
end
