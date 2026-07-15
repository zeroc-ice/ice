classdef UnknownSlicedValue < Ice.Value
    %UNKNOWNSLICEDVALUE Represents an instance of an unknown class.
    %
    %   UnknownSlicedValue Methods:
    %     ice_id - Returns the Slice type ID of this unknown value's most-derived slice, or a compact ID
    %       formatted as a decimal string when that slice was encoded with a compact ID.

    % Copyright (c) ZeroC, Inc.

    methods (Hidden)
        function obj = UnknownSlicedValue(unknownTypeId)
            obj.unknownTypeId = unknownTypeId;
        end
    end
    methods
        function id = ice_id(obj)
            %ICE_ID Returns the Slice type ID of this unknown value's most-derived slice. When that slice was
            %   encoded with a compact ID, the return value is the compact ID formatted as a decimal string.
            %
            %   Output Arguments
            %     id - The Slice type ID, or a compact ID formatted as a decimal string.
            %       character vector
            id = obj.unknownTypeId;
        end
    end
    properties (Access = private)
        unknownTypeId
    end
end
