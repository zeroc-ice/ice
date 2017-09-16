%{
**********************************************************************

Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.

This copy of Ice is licensed to you under the terms described in the
ICE_LICENSE file included in this distribution.

**********************************************************************
%}

classdef Communicator < IceInternal.WrapperObject
    methods
        function obj = Communicator(impl, initData)
            obj = obj@IceInternal.WrapperObject(impl);
            obj.initData = initData;
        end
        function destroy(obj)
            obj.call_('destroy');
        end
        function f = destroyAsync(obj)
            future = libpointer('voidPtr');
            obj.call_('destroyAsync', future);
            assert(~isNull(future));
            f = Ice.Future(future, 'destroy', 0, 'Ice_SimpleFuture', []);
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
        function r = proxyToString(obj, proxy)
            if isempty(proxy)
                r = '';
            elseif ~isa(proxy, 'Ice.ObjectPrx')
                throw(MException('Ice:ArgumentException', 'expecting a proxy'));
            else
                r = obj.callWithResult_('proxyToString', proxy.impl_);
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
        function r = identityToString(obj, id)
            r = obj.callWithResult_('identityToString', id);
        end
        function r = getProperties(obj)
            impl = libpointer('voidPtr');
            obj.call_('getProperties', impl);
            r = Ice.Properties(impl);
        end
        function r = getDefaultRouter(obj)
            impl = libpointer('voidPtr');
            obj.call_('getDefaultRouter', impl);
            if ~isNull(impl)
                r = Ice.RouterPrx(impl, obj);
            else
                r = [];
            end
        end
        function setDefaultRouter(obj, proxy)
            if isempty(proxy)
                impl = libpointer('voidPtr');
            elseif ~isa(proxy, 'Ice.RouterPrx')
                throw(MException('Ice:ArgumentException', 'expecting a router proxy'));
            else
                impl = proxy.impl_;
            end
            obj.call_('setDefaultRouter', impl_);
        end
        function r = getDefaultLocator(obj)
            impl = libpointer('voidPtr');
            obj.call_('getDefaultLocator', impl);
            if ~isNull(impl)
                r = Ice.LocatorPrx(impl, obj);
            else
                r = [];
            end
        end
        function setDefaultLocator(obj, proxy)
            if isempty(proxy)
                impl = libpointer('voidPtr');
            elseif ~isa(proxy, 'Ice.LocatorPrx')
                throw(MException('Ice:ArgumentException', 'expecting a locator proxy'));
            else
                impl = proxy.impl_;
            end
            obj.call_('setDefaultLocator', impl_);
        end
        function r = getValueFactoryManager(obj)
            r = obj.initData.valueFactoryManager;
        end
        function flushBatchRequests(obj, mode)
            obj.call_('flushBatchRequests', mode);
        end
        function f = flushBatchRequestsAsync(obj, mode)
            future = libpointer('voidPtr');
            obj.call_('flushBatchRequestsAsync', mode, future);
            assert(~isNull(future));
            r = Ice.Future(future, 'flushBatchRequests', 0, 'Ice_SimpleFuture', []);
        end
        function r = getClassResolver(obj) 
            if isempty(obj.classResolver) % Lazy initialization.
                obj.classResolver = IceInternal.ClassResolver(obj.getProperties());
            end
            r = obj.classResolver;
        end
        function r = getCompactIdResolver(obj)
            r = obj.initData.compactIdResolver;
        end
    end
    properties(Access=private)
        initData
        classResolver
    end
end
