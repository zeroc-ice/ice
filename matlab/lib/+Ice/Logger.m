%{
**********************************************************************

Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.

This copy of Ice is licensed to you under the terms described in the
ICE_LICENSE file included in this distribution.

**********************************************************************
%}

classdef Logger < IceInternal.WrapperObject
    methods
        function obj = Logger(impl)
            if ~isa(impl, 'lib.pointer')
                throw(MException('Ice:ArgumentException', 'invalid argument'));
            end
            obj = obj@IceInternal.WrapperObject(impl);
        end
        function print(obj, message)
            obj.call_('print', message);
        end
        function trace(obj, category, message)
            obj.call_('trace', category, message);
        end
        function warning(obj, message)
            obj.call_('warning', message);
        end
        function error(obj, message)
            obj.call_('error', message);
        end
        function r = getPrefix(obj)
            r = obj.callWithResult_('getPrefix');
        end
        function r = cloneWithPrefix(obj, prefix)
            impl = libpointer('voidPtr');
            obj.call_('cloneWithPrefix', prefix, impl);
            if isNull(impl)
                r = obj;
            else
                r = Ice.Logger(impl);
            end
        end
    end
end
