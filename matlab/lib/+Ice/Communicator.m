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
            if ~isa(impl, 'lib.pointer')
                throw(MException('Ice:ArgumentException', 'invalid argument'));
            end
            obj = obj@IceInternal.WrapperObject(impl);
            obj.initData = initData;

            enc = obj.getProperties().getProperty('Ice.Default.EncodingVersion');
            if isempty(enc)
                obj.encoding = Ice.currentEncoding();
            else
                arr = sscanf(enc, '%u.%u');
                if length(arr) ~= 2
                    throw(MException('Ice:ArgumentException', 'invalid value for Ice.Default.EncodingVersion'));
                end
                obj.encoding = Ice.EncodingVersion(arr(1), arr(2));
            end
        end
        function destroy(obj)
            obj.iceCall('destroy');
        end
        function f = destroyAsync(obj)
            future = libpointer('voidPtr');
            obj.iceCall('destroyAsync', future);
            assert(~isNull(future));
            f = Ice.Future(future, 'destroy', 0, 'Ice_SimpleFuture', @(fut) fut.iceCall('check'));
        end
        function r = stringToProxy(obj, str)
            impl = libpointer('voidPtr');
            obj.iceCall('stringToProxy', str, impl);
            if isNull(impl)
                r = [];
            else
                r = Ice.ObjectPrx(obj, obj.encoding, impl);
            end
        end
        function r = proxyToString(obj, proxy)
            if isempty(proxy)
                r = '';
            elseif ~isa(proxy, 'Ice.ObjectPrx')
                throw(MException('Ice:ArgumentException', 'expecting a proxy'));
            else
                r = proxy.ice_toString();
            end
        end
        function r = propertyToProxy(obj, prop)
            impl = libpointer('voidPtr');
            obj.iceCall('propertyToProxy', prop, impl);
            if isNull(impl)
                r = [];
            else
                r = Ice.ObjectPrx(obj, obj.encoding, impl);
            end
        end
        function r = proxyToProperty(obj, proxy, prop)
            if isempty(proxy)
                r = containers.Map('KeyType', 'char', 'ValueType', 'char');
            elseif ~isa(proxy, 'Ice.ObjectPrx')
                throw(MException('Ice:ArgumentException', 'expecting a proxy'));
            else
                r = obj.iceCallWithResult('proxyToProperty', proxy.iceGetImpl(), prop);
            end
        end
        function r = identityToString(obj, id)
            r = obj.iceCallWithResult('identityToString', id);
        end
        function r = getProperties(obj)
            if isempty(obj.properties_)
                impl = libpointer('voidPtr');
                obj.iceCall('getProperties', impl);
                obj.properties_ = Ice.Properties(impl);
            end
            r = obj.properties_;
        end
        function r = getLogger(obj)
            if isempty(obj.logger)
                impl = libpointer('voidPtr');
                obj.iceCall('getLogger', impl);
                obj.logger = Ice.Logger(impl);
            end
            r = obj.logger;
        end
        function r = getDefaultRouter(obj)
            impl = libpointer('voidPtr');
            obj.iceCall('getDefaultRouter', impl);
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
                impl = proxy.iceGetImpl();
            end
            obj.iceCall('setDefaultRouter', impl);
        end
        function r = getDefaultLocator(obj)
            impl = libpointer('voidPtr');
            obj.iceCall('getDefaultLocator', impl);
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
                impl = proxy.iceGetImpl();
            end
            obj.iceCall('setDefaultLocator', impl);
        end
        function r = getValueFactoryManager(obj)
            r = obj.initData.valueFactoryManager;
        end
        function flushBatchRequests(obj, mode)
            obj.iceCall('flushBatchRequests', mode);
        end
        function f = flushBatchRequestsAsync(obj, mode)
            future = libpointer('voidPtr');
            obj.iceCall('flushBatchRequestsAsync', mode, future);
            assert(~isNull(future));
            r = Ice.Future(future, 'flushBatchRequests', 0, 'Ice_SimpleFuture', @(fut) fut.iceCall('check'));
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
        function r = getEncoding(obj)
            r = obj.encoding;
        end
        function r = createOutputStream(obj, encoding)
            if nargin == 1
                encoding = obj.encoding;
            end
            r = Ice.OutputStream(obj, encoding);
        end
    end
    properties(Access=private)
        initData
        classResolver
        encoding
        logger
        properties_
    end
end
