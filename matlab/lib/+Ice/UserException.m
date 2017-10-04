%{
**********************************************************************

Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.

This copy of Ice is licensed to you under the terms described in the
ICE_LICENSE file included in this distribution.

**********************************************************************
%}

classdef (Abstract) UserException < Ice.Exception
    methods
        function obj = UserException(id, msg)
            obj = obj@Ice.Exception(id, msg)
        end
        function r = ice_getSlicedData(obj)
            r = [];
        end
    end
    methods(Hidden=true)
        function obj = iceRead(obj, is)
            is.startException();
            obj = obj.iceReadImpl(is);
            is.endException(false);
        end
        function obj = icePostUnmarshal(obj)
            %
            % Overridden by subclasses that have class members.
            %
        end
    end
    methods(Abstract,Access=protected)
        obj = iceReadImpl(obj, is)
    end
end
