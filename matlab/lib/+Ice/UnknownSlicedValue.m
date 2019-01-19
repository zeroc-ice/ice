classdef UnknownSlicedValue < Ice.Value
    % UnknownSlicedValue   Summary of UnknownSlicedValue
    %
    % UnknownSlicedValue holds an instance of an unknown Slice class type.
    % Call the constructor with the Slice type ID of the unknown value type.
    %
    % UnknownSlicedValue Methods:
    %   ice_getSlicedData - Obtain the SlicedData object that encapsulates
    %     the value's marshaled state.

    % Copyright (c) ZeroC, Inc. All rights reserved.

    methods
        function obj = UnknownSlicedValue(unknownTypeId)
            obj.unknownTypeId = unknownTypeId;
        end
        % ice_getSlicedData - Obtain the SlicedData object that encapsulates
        %   the value's marshaled state.
        %
        % Returns (Ice.SlicedData) - The value's marshaled state.

        function r = ice_getSlicedData(obj)
            r = obj.slicedData;
        end
        function iceWrite(obj, os)
            os.startValue(obj.slicedData);
            os.endValue();
        end
        function obj = iceRead(obj, is)
            is.startValue();
            obj.slicedData = is.endValue(true);
        end
        function id = ice_id(obj)
            id = obj.unknownTypeId;
        end
    end
    methods(Access=protected)
        function iceWriteImpl(obj, os)
        end
        function obj = iceReadImpl(obj, is)
        end
    end
    properties(Access=private)
        unknownTypeId
        slicedData
    end
end
