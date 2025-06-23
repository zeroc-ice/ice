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
            %ICE_ID Returns the Slice type ID associated with this instance.
            %   Output Arguments
            %     id - The Slice type ID.
            %       character vector
            id = obj.unknownTypeId;
        end
    end
    properties(Access=private)
        unknownTypeId
    end
end
