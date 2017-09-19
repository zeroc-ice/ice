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
        function obj = read_(obj, is)
            is.startException();
            obj = obj.readImpl_(is);
            is.endException(false);
        end
        function obj = preUnmarshal_(obj)
            %
            % Overridden by subclasses that have class members.
            %
        end
        function obj = postUnmarshal_(obj)
            %
            % Overridden by subclasses that have class members.
            %
        end
    end
    methods(Abstract,Access=protected)
        obj = readImpl_(obj, is)
    end
end
