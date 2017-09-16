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
        function id = ice_id(obj)
            id = obj.unknownTypeId;
        end
        function iceWrite_(obj, os)
            os.startValue(obj.slicedData);
            os.endValue();
        end
        function obj = iceRead_(obj, is)
            is.startValue();
            obj.slicedData = is.endValue(true);
        end
    end
    properties(Accses=private)
        unknownTypeId
        slicedData
    end
end
