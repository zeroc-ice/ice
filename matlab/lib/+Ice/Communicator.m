classdef Communicator < IceInternal.WrapperObject
    % Communicator   Summary of Communicator
    %
    % The central object in Ice. One or more communicators can be
    % instantiated for an Ice application. Communicator instantiation
    % is language-specific, and not specified in Slice code.
    %
    % Communicator Methods:
    %   destroy - Destroy the communicator.
    %   destroyAsync - Destroy the communicator.
    %   stringToProxy - Convert a stringified proxy into a proxy.
    %   proxyToString - Convert a proxy into a string.
    %   propertyToProxy - Convert a set of proxy properties into a proxy.
    %   proxyToProperty - Convert a proxy to a set of proxy properties.
    %   identityToString - Convert an identity into a string.
    %   getProperties - Get the properties for this communicator.
    %   getLogger - Get the logger for this communicator.
    %   getDefaultRouter - Get the default router for this communicator.
    %   setDefaultRouter - Set a default router for this communicator.
    %   getDefaultLocator - Get the default locator for this communicator.
    %   setDefaultLocator - Set a default locator for this communicator.
    %   getValueFactoryManager - Get the value factory manager for this communicator.
    %   flushBatchRequests - Flush any pending batch requests for this communicator.
    %   flushBatchRequestsAsync - Flush any pending batch requests for this communicator.

    % Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.

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
            % destroy   Destroy the communicator. Calling destroy cleans up
            % memory, and shuts down this communicator's client functionality.
            % Subsequent calls to destroy are ignored.
            obj.iceCall('destroy');
        end
        function f = destroyAsync(obj)
            % destroyAsync   Asynchronously destroy the communicator. Calling
            % destroy cleans up memory, and shuts down this communicator's
            % client functionality. Subsequent calls to destroy are ignored.
            %
            % Returns (Ice.Future) - A future that will be completed when the
            %   invocation completes.

            future = libpointer('voidPtr');
            obj.iceCall('destroyAsync', future);
            assert(~isNull(future));
            f = Ice.Future(future, 'destroy', 0, 'Ice_SimpleFuture', @(fut) fut.iceCall('check'));
        end
        function r = stringToProxy(obj, str)
            % stringToProxy   Convert a stringified proxy into a proxy.
            % For example:
            %
            % MyCategory/MyObject:tcp -h some_host -p 10000
            %
            % is a proxy that refers to the Ice object having an identity with
            % a name "MyObject" and a category "MyCategory", with the server
            % running on host "some_host", port 10000. If the stringified proxy
            % does not parse correctly, the operation throws one of
            % ProxyParseException, EndpointParseException, or
            % IdentityParseException.
            %
            % The Ice manual provides a detailed description of the syntax
            % supported by stringified proxies.
            %
            % Parameters:
            %   str (char) - The stringified proxy to convert into a proxy.
            %
            % Returns (Ice.ObjectPrx) - The proxy, or an empty array if str is
            %   an empty string.

            impl = libpointer('voidPtr');
            obj.iceCall('stringToProxy', str, impl);
            if isNull(impl)
                r = [];
            else
                r = Ice.ObjectPrx(obj, obj.encoding, impl);
            end
        end
        function r = proxyToString(obj, proxy)
            % proxyToString   Convert a proxy into a string.
            %
            % Parameters:
            %   proxy (Ice.ObjectPrx) - The proxy to convert into a stringified
            %     proxy.
            %
            % Returns (char) - The stringified proxy, or an empty string if
            %   proxy is an empty array.

            if isempty(proxy)
                r = '';
            elseif ~isa(proxy, 'Ice.ObjectPrx')
                throw(MException('Ice:ArgumentException', 'expecting a proxy'));
            else
                r = proxy.ice_toString();
            end
        end
        function r = propertyToProxy(obj, prop)
            % propertyToProxy   Convert a set of proxy properties into a proxy.
            % The "base" name supplied in the property argument refers to a
            % property containing a stringified proxy, such as
            %
            % MyProxy=id:tcp -h localhost -p 10000
            %
            % Additional properties configure local settings for the proxy, such as
            %
            % MyProxy.PreferSecure=1
            %
            % The property reference in the Ice manual describes each of the
            % supported proxy properties.
            %
            % Parameters:
            %   prop (char) - The base property name.
            %
            % Returns (Ice.ObjectPrx) - The proxy.

            impl = libpointer('voidPtr');
            obj.iceCall('propertyToProxy', prop, impl);
            if isNull(impl)
                r = [];
            else
                r = Ice.ObjectPrx(obj, obj.encoding, impl);
            end
        end
        function r = proxyToProperty(obj, proxy, prop)
            % proxyToProperty   Convert a proxy to a set of proxy properties.
            %
            % Parameters:
            %   proxy (Ice.ObjectPrx) - The proxy.
            %   property (char) - The base property name.
            %
            % Returns (containers.Map) - The property set.

            if isempty(proxy)
                r = containers.Map('KeyType', 'char', 'ValueType', 'char');
            elseif ~isa(proxy, 'Ice.ObjectPrx')
                throw(MException('Ice:ArgumentException', 'expecting a proxy'));
            else
                r = obj.iceCallWithResult('proxyToProperty', proxy.iceGetImpl(), prop);
            end
        end
        function r = identityToString(obj, id)
            % identityToString   Convert an identity into a string.
            %
            % Parameters:
            %   id (Ice.Identity) - The identity to convert into a string.
            %
            % Returns (char) - The stringified identity.

            r = obj.iceCallWithResult('identityToString', id);
        end
        function r = getProperties(obj)
            % getProperties   Get the properties for this communicator.
            %
            % Returns (Ice.Properties) - This communicator's properties.

            if isempty(obj.properties_)
                impl = libpointer('voidPtr');
                obj.iceCall('getProperties', impl);
                obj.properties_ = Ice.Properties(impl);
            end
            r = obj.properties_;
        end
        function r = getLogger(obj)
            % getLogger   Get the logger for this communicator.
            %
            % Returns (Ice.Logger) - This communicator's logger.

            if isempty(obj.logger)
                impl = libpointer('voidPtr');
                obj.iceCall('getLogger', impl);
                obj.logger = Ice.Logger(impl);
            end
            r = obj.logger;
        end
        function r = getDefaultRouter(obj)
            % getDefaultRouter   Get the default router for this communicator.
            %
            % Returns (Ice.RouterPrx) - This communicator's default router.

            impl = libpointer('voidPtr');
            obj.iceCall('getDefaultRouter', impl);
            if ~isNull(impl)
                r = Ice.RouterPrx(impl, obj);
            else
                r = [];
            end
        end
        function setDefaultRouter(obj, proxy)
            % setDefaultRouter   Set a default router for this communicator.
            % All newly created proxies will use this default router.
            % To disable the default router, an empty array can be used.
            % Note that this operation has no effect on existing proxies.
            %
            % You can also set a router for an individual proxy by calling
            % the operation ice_router on the proxy.
            %
            % Parameters:
            %   proxy (Ice.RouterPrx) - The default router to use for this
            %     communicator.

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
            % getDefaultLocator   Get the default locator for this communicator.
            %
            % Returns (Ice.LocatorPrx) - This communicator's default locator.

            impl = libpointer('voidPtr');
            obj.iceCall('getDefaultLocator', impl);
            if ~isNull(impl)
                r = Ice.LocatorPrx(impl, obj);
            else
                r = [];
            end
        end
        function setDefaultLocator(obj, proxy)
            % setDefaultLocator   Set a default locator for this communicator.
            % All newly created proxies will use this default locator.
            % To disable the default locator, an empty array can be used.
            % Note that this operation has no effect on existing proxies.
            %
            % You can also set a locator for an individual proxy by calling
            % the operation ice_locator on the proxy.
            %
            % Parameters:
            %   proxy (Ice.LocatorPrx) - The default locator to use for this
            %     communicator.

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
            % getValueFactoryManager   Get the value factory manager for this
            %   communicator.
            %
            % Returns (Ice.ValueFactoryManager) - This communicator's value
            %   factory manager.

            r = obj.initData.valueFactoryManager;
        end
        function flushBatchRequests(obj, mode)
            % flushBatchRequests   Flush any pending batch requests for this
            %   communicator. This means all batch requests invoked on fixed
            %   proxies for all connections associated with the communicator.
            %   Any errors that occur while flushing a connection are ignored.
            %
            % Parameters:
            %   mode (Ice.CompressBatch) - Specifies whether or not the queued
            %     batch requests should be compressed before being sent over
            %     the wire.

            obj.iceCall('flushBatchRequests', mode);
        end
        function f = flushBatchRequestsAsync(obj, mode)
            % flushBatchRequestsAsync   Flush any pending batch requests for this
            %   communicator. This means all batch requests invoked on fixed
            %   proxies for all connections associated with the communicator.
            %   Any errors that occur while flushing a connection are ignored.
            %
            % Parameters:
            %   mode (Ice.CompressBatch) - Specifies whether or not the queued
            %     batch requests should be compressed before being sent over
            %     the wire.
            %
            % Returns (Ice.Future) - A future that will be completed when the
            %   invocation completes.

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
