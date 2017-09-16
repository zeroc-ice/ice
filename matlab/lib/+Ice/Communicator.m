%{
**********************************************************************

Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.

This copy of Ice is licensed to you under the terms described in the
ICE_LICENSE file included in this distribution.

**********************************************************************
%}

classdef Communicator < IceInternal.WrapperObject
    methods
        function obj = Communicator(impl, vfm)
            obj = obj@IceInternal.WrapperObject(impl);
            obj.valueFactoryManager = vfm;
        end
        function r = identityToString(obj, id)
            r = obj.callWithResult_('identityToString', id);
        end
        function r = stringToProxy(obj, str)
            impl = libpointer('voidPtr');
            obj.call_('stringToProxy', str, impl);
            if isNull(impl)
                r = [];
            else
                r = Ice.ObjectPrx(impl, obj);
            end
        end
        function r = propertyToProxy(obj, prop)
            impl = libpointer('voidPtr');
            obj.call_('propertyToProxy', prop, impl);
            if isNull(impl)
                r = [];
            else
                r = Ice.ObjectPrx(impl, obj);
            end
        end
        function r = proxyToProperty(obj, proxy, prop)
            if isempty(proxy)
                r = containers.Map('KeyType', 'char', 'ValueType', 'char');
            elseif ~isa(proxy, 'Ice.ObjectPrx')
                throw(MException('Ice:ArgumentException', 'expecting a proxy'));
            else
                r = obj.callWithResult_('proxyToProperty', proxy.impl_, prop);
            end
        end
        function r = proxyToString(obj, proxy)
            if isempty(proxy)
                r = '';
            elseif ~isa(proxy, 'Ice.ObjectPrx')
                throw(MException('Ice:ArgumentException', 'expecting a proxy'));
            else
                r = obj.callWithResult_('proxyToString', proxy.impl_);
            end
        end
        function r = getProperties(obj)
            impl = libpointer('voidPtr');
            obj.call_('getProperties', impl);
            r = Ice.Properties(impl);
        end
        function r = getValueFactoryManager(obj)
            r = obj.valueFactoryManager;
        end
        function destroy(obj)
            obj.call_('destroy');
        end
    end
    properties(Access=private)
        valueFactoryManager
    end
end
