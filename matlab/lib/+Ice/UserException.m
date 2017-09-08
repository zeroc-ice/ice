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
    end
    methods(Hidden=true)
        function obj = read_(obj, is)
            is.startException();
            if obj.usesClasses_()
                obj.valueTable_ = containers.Map('KeyType', 'char', 'ValueType', 'any');
            end
            obj = obj.readImpl_(is);
            is.endException();
        end
        function r = usesClasses_(obj)
            r = false;
        end
        function obj = resolveValues_(obj)
        end
        function setValueMember_(obj, k, v)
            obj.valueTable_(k) = v;
        end
    end
    methods(Abstract,Access=protected)
        obj = readImpl_(obj, is)
    end
    properties(Access=protected)
        valueTable_
    end
end
