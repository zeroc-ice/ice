%{
**********************************************************************

Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.

This copy of Ice is licensed to you under the terms described in the
ICE_LICENSE file included in this distribution.

**********************************************************************
%}

classdef (Abstract) Value < matlab.mixin.Copyable
    methods
        function ice_preMarshal(obj)
        end
        function ice_postUnmarshal(obj)
        end
        function r = ice_getSlicedData(obj)
            r = [];
        end
        function iceWrite_(obj, os)
            os.startValue([]);
            obj.iceWriteImpl_(os);
            os.endValue();
        end
        function obj = iceRead_(obj, is)
            is.startValue();
            obj = obj.iceReadImpl_(is);
            is.endValue(false);
        end
    end
    methods(Abstract)
        id = ice_id(obj)
    end
    methods(Access=protected)
        iceWriteImpl_(obj, os)
        obj = iceReadImpl_(obj, is)
    end
    methods(Static)
        function id = ice_staticId()
            id = '::Ice::Object';
        end
    end
    properties(Hidden)
        internal_ int32 = -1
    end
end
