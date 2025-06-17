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
    %   getImplicitContext - Get the implicit context associated with this
    %     communicator.
    %   getProperties - Get the properties for this communicator.
    %   getLogger - Get the logger for this communicator.
    %   getDefaultRouter - Get the default router for this communicator.
    %   setDefaultRouter - Set a default router for this communicator.
    %   getDefaultLocator - Get the default locator for this communicator.
    %   setDefaultLocator - Set a default locator for this communicator.
    %   getEncoding - Get the encoding version for this communicator.
    %   getFormat - Get the class format for this communicator.
    %   flushBatchRequests - Flush any pending batch requests for this
    %     communicator.
    %   flushBatchRequestsAsync - Flush any pending batch requests for this
    %     communicator.

    % Copyright (c) ZeroC, Inc.

    methods(Hidden)
        function obj = Communicator(impl, initData)
            % Called by Ice.initialize.
            assert(isa(impl, 'lib.pointer'))
            obj@IceInternal.WrapperObject(impl);

            % The caller (initialize) consumes initData.Properties and we don't use them at all in this class.
            obj.initData = initData;

            if obj.initData.SliceLoader ~= IceInternal.DefaultSliceLoader.Instance
                obj.initData.SliceLoader = Ice.CompositeSliceLoader(obj.initData.SliceLoader, ...
                    IceInternal.DefaultSliceLoader.Instance);
            end

            notFoundCacheSize = obj.getProperties().getIcePropertyAsInt('Ice.SliceLoader.NotFoundCacheSize');
            if notFoundCacheSize > 0
                % Install the NotFoundSliceLoaderDecorator.
                if obj.getProperties().getIcePropertyAsInt('Ice.Warn.SliceLoader') > 0
                    cacheFullLogger = obj.getLogger();
                else
                    cacheFullLogger = [];
                end

                obj.initData.SliceLoader = IceInternal.NotFoundSliceLoaderDecorator(...
                    obj.initData.SliceLoader, notFoundCacheSize, cacheFullLogger);
            end

            enc = obj.getProperties().getProperty('Ice.Default.EncodingVersion');
            if isempty(enc)
                obj.encoding = Ice.currentEncoding();
            else
                arr = sscanf(enc, '%u.%u');
                if length(arr) ~= 2
                    throw(Ice.LocalException('Ice:ArgumentException', 'invalid value for Ice.Default.EncodingVersion'));
                end
                obj.encoding = Ice.EncodingVersion(arr(1), arr(2));
            end
            if obj.getProperties().getIcePropertyAsInt('Ice.Default.SlicedFormat') > 0
                obj.format = Ice.FormatType.SlicedFormat;
            else
                obj.format = Ice.FormatType.CompactFormat;
            end
        end
    end
    methods
        function destroy(obj)
            % destroy   Destroy the communicator. Calling destroy cleans up
            % memory, and shuts down this communicator's client functionality.
            % Subsequent calls to destroy are ignored.

            arguments
                obj (1, 1) Ice.Communicator
            end
            obj.iceCall('destroy');
        end
        function f = destroyAsync(obj)
            % destroyAsync   Asynchronously destroy the communicator. Calling
            % destroy cleans up memory, and shuts down this communicator's
            % client functionality. Subsequent calls to destroy are ignored.
            %
            % Returns (Ice.Future) - A future that will be completed when the
            %   invocation completes.

            arguments
                obj (1, 1) Ice.Communicator
            end
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
            % does not parse correctly, the operation throws ParseException.
            %
            % The Ice manual provides a detailed description of the syntax
            % supported by stringified proxies.
            %
            % Parameters:
            %   str (char) - The stringified proxy to convert into a proxy.
            %
            % Returns (Ice.ObjectPrx) - The proxy, or an empty array if str is
            %   an empty string.

            arguments
                obj (1, 1) Ice.Communicator
                str (1, :) char
            end
            impl = libpointer('voidPtr');
            obj.iceCall('stringToProxy', str, impl);
            if isNull(impl)
                r = Ice.ObjectPrx.empty;
            else
                r = Ice.ObjectPrx(obj, '', impl);
            end
        end
        function r = proxyToString(~, proxy)
            % proxyToString   Convert a proxy into a string.
            %
            % Parameters:
            %   proxy (Ice.ObjectPrx) - The proxy to convert into a stringified
            %     proxy.
            %
            % Returns (char) - The stringified proxy, or an empty string if
            %   proxy is an empty array.

            arguments
                ~
                proxy Ice.ObjectPrx {mustBeScalarOrEmpty}
            end
            if isempty(proxy)
                r = '';
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

            arguments
                obj (1, 1) Ice.Communicator
                prop (1, :) char
            end
            impl = libpointer('voidPtr');
            obj.iceCall('propertyToProxy', prop, impl);
            if isNull(impl)
                r = Ice.ObjectPrx.empty;
            else
                r = Ice.ObjectPrx(obj, '', impl);
            end
        end
        function r = proxyToProperty(obj, proxy, prop)
            % proxyToProperty   Convert a proxy to a set of proxy properties.
            %
            % Parameters:
            %   proxy (Ice.ObjectPrx) - The proxy.
            %   property (char) - The base property name.
            %
            % Returns (dictionary) - The property set.

            arguments
                obj (1, 1) Ice.Communicator
                proxy Ice.ObjectPrx {mustBeScalarOrEmpty}
                prop (1, :) char
            end
            if isempty(proxy)
                r = configureDictionary('char', 'char');
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

            arguments
                obj (1, 1) Ice.Communicator
                id (1, 1) Ice.Identity
            end
            r = obj.iceCallWithResult('identityToString', id);
        end
        function r = getImplicitContext(obj)
            % getImplicitContext   Get the implicit context associated with
            %   this communicator.
            %
            % Returns (Ice.ImplicitContext) - The implicit context associated
            %   with this communicator; returns an empty array when the property
            %   Ice.ImplicitContext is not set or is set to None.

            arguments
                obj (1, 1) Ice.Communicator
            end
            if isempty(obj.implicitContext)
                impl = libpointer('voidPtr');
                obj.iceCall('getImplicitContext', impl);
                obj.implicitContext = Ice.ImplicitContext(impl);
            end
            r = obj.implicitContext;
        end
        function r = getProperties(obj)
            % getProperties   Get the properties for this communicator.
            %
            % Returns (Ice.Properties) - This communicator's properties.

            arguments
                obj (1, 1) Ice.Communicator
            end
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

            arguments
                obj (1, 1) Ice.Communicator
            end
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

            arguments
                obj (1, 1) Ice.Communicator
            end
            impl = libpointer('voidPtr');
            obj.iceCall('getDefaultRouter', impl);
            if ~isNull(impl)
                r = Ice.RouterPrx(obj, '', impl);
            else
                r = Ice.RouterPrx.empty;
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

            arguments
                obj (1, 1) Ice.Communicator
                proxy Ice.RouterPrx {mustBeScalarOrEmpty}
            end
            if isempty(proxy)
                impl = libpointer('voidPtr');
            else
                impl = proxy.iceGetImpl();
            end
            obj.iceCall('setDefaultRouter', impl);
        end
        function r = getDefaultLocator(obj)
            % getDefaultLocator   Get the default locator for this communicator.
            %
            % Returns (Ice.LocatorPrx) - This communicator's default locator.

            arguments
                obj (1, 1) Ice.Communicator
            end
            impl = libpointer('voidPtr');
            obj.iceCall('getDefaultLocator', impl);
            if ~isNull(impl)
                r = Ice.LocatorPrx(obj, '', impl);
            else
                r = Ice.LocatorPrx.empty;
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

            arguments
                obj (1, 1) Ice.Communicator
                proxy Ice.LocatorPrx {mustBeScalarOrEmpty}
            end
            if isempty(proxy)
                impl = libpointer('voidPtr');
            else
                impl = proxy.iceGetImpl();
            end
            obj.iceCall('setDefaultLocator', impl);
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

            arguments
                obj (1, 1) Ice.Communicator
                mode (1, 1) Ice.CompressBatch
            end
            obj.iceCall('flushBatchRequests', mode);
        end
        function r = flushBatchRequestsAsync(obj, mode)
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

            arguments
                obj (1, 1) Ice.Communicator
                mode (1, 1) Ice.CompressBatch
            end
            future = libpointer('voidPtr');
            obj.iceCall('flushBatchRequestsAsync', mode, future);
            assert(~isNull(future));
            r = Ice.Future(future, 'flushBatchRequests', 0, 'Ice_SimpleFuture', @(fut) fut.iceCall('check'));
        end

        function r = getEncoding(obj)
            arguments
                obj (1, 1) Ice.Communicator
            end
            r = obj.encoding;
        end

        function r = getFormat(obj)
            arguments
                obj (1, 1) Ice.Communicator
            end
            r = obj.format;
        end

        function r = getSliceLoader(obj)
            arguments
                obj (1, 1) Ice.Communicator
            end
            r = obj.initData.SliceLoader;
        end
    end
    properties(Access=private)
        initData
        encoding
        format
        implicitContext
        properties_
        logger
    end
end
