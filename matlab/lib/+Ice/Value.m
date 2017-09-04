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
        function id = ice_id(obj)
            id = ice_staticId()
        end
        function iceWrite_(self, os)
            os.startValue([]);
            self.iceWriteImpl_(os);
            os.endValue();
        end
        function self = iceRead_(self, is)
            is.startValue();
            self = self.iceReadImpl_(is);
            is.endValue(false);
        end
    end
    methods(Abstract)
        iceWriteImpl_(self, os)
        self = iceReadImpl_(self, is)
    end
    methods(Static)
        function id = ice_staticId()
            id = '::Ice::Object'
        end
    end
end
