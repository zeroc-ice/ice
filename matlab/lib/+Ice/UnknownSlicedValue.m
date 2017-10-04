%{
**********************************************************************

Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.

This copy of Ice is licensed to you under the terms described in the
ICE_LICENSE file included in this distribution.

**********************************************************************
%}

%
% UnknownSlicedValue holds an instance of an unknown Slice class type.
%
classdef UnknownSlicedValue < Ice.Value
    methods
        function obj = UnknownSlicedValue(unknownTypeId)
            obj.unknownTypeId = unknownTypeId;
        end
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
