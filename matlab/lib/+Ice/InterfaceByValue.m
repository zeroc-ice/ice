%{
**********************************************************************

Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.

This copy of Ice is licensed to you under the terms described in the
ICE_LICENSE file included in this distribution.

**********************************************************************
%}

classdef InterfaceByValue < Ice.Value
    methods
        function obj = InterfaceByValue(id)
            obj.id = id;
        end
        function id = ice_id(obj)
            id = obj.id;
        end
        function iceWriteImpl_(obj, os)
            os.startSlice(obj.ice_id(), -1, true);
            os.endSlice();
        end
        function obj = iceReadImpl_(obj, is)
            is.startSlice();
            is.endSlice();
        end
    end
    properties(Access=private)
        id
    end
end
