classdef Communicator < IceInternal.WrapperObject
    %COMMUNICATOR The central object in Ice.
    %   Communicator is the central object in Ice. Its responsibilities include:
    %   - creating and managing outgoing connections
    %   - managing properties (configuration), retries, logging, and more.
    %
    % Copyright (c) ZeroC, Inc.

    methods
        function [obj, remArgs] = Communicator(args, options)
            %COMMUNICATOR Constructs a new communicator.
            %
            %   Examples
            %     communicator = Ice.Communicator();
            %     [communicator, remArgs] = Ice.Communicator(args);
            %     communicator = Ice.Communicator(Properties = props, SliceLoader = sliceLoader);
            %     [communicator, remArgs] = Ice.Communicator(args, SliceLoader = sliceLoader);
            %
            %   Input Arguments
            %     args - Argument vector. Any Ice-related options in this vector are used to set the communicator
            %       properties.
            %       empty cell array (default) | cell array of character | string array
            %
            %   Input Name-Value Arguments
            %     Properties - Properties object used to initialize the communicator properties. If args is non-empty,
            %       any reserved properties specified in args override these in properties.
            %       Ice.Properties scalar
            %     SliceLoader - Slice loader used to load Slice classes and exceptions.
            %       Ice.SliceLoader scalar
            %
            %   Output Arguments
            %     communicator - The new communicator.
            %       Ice.Communicator scalar
            %     remArgs - Remaining command-line arguments that were not used to set properties.
            %       string array
            arguments
                args (1, :) = {}
                options.?Ice.InitializationData
                options.Properties (1, 1) Ice.Properties = Ice.Properties()
                options.SliceLoader (1, 1) Ice.SliceLoader = IceInternal.DefaultSliceLoader.Instance
            end

            % We need to extract and pass the libpointer object for properties to the C function. Passing the wrapper
            % (Ice.Properties) object won't work because the C code has no way to obtain the inner pointer.
            propsImpl = options.Properties.impl_;
            impl = libpointer('voidPtr');
            remArgs = IceInternal.Util.callWithResult('Ice_initialize', args, propsImpl, impl);

            obj@IceInternal.WrapperObject(impl);

            obj.SliceLoader = options.SliceLoader;
            if obj.SliceLoader ~= IceInternal.DefaultSliceLoader.Instance
                obj.SliceLoader = Ice.CompositeSliceLoader(obj.SliceLoader, IceInternal.DefaultSliceLoader.Instance);
            end

            notFoundCacheSize = obj.getProperties().getIcePropertyAsInt('Ice.SliceLoader.NotFoundCacheSize');
            if notFoundCacheSize > 0
                % Install the NotFoundSliceLoaderDecorator.
                if obj.getProperties().getIcePropertyAsInt('Ice.Warn.SliceLoader') > 0
                    cacheFullLogger = obj.getLogger();
                else
                    cacheFullLogger = [];
                end

                obj.SliceLoader = IceInternal.NotFoundSliceLoaderDecorator(...
                    obj.SliceLoader, notFoundCacheSize, cacheFullLogger);
            end

            enc = obj.getProperties().getProperty('Ice.Default.EncodingVersion');
            if isempty(enc)
                obj.encoding = IceInternal.Protocol.CurrentEncoding;
            else
                arr = sscanf(enc, '%u.%u');
                if length(arr) ~= 2
                    error('Ice:ArgumentException', 'Invalid value for Ice.Default.EncodingVersion');
                end
                obj.encoding = Ice.EncodingVersion(arr(1), arr(2));
            end
            if obj.getProperties().getIcePropertyAsInt('Ice.Default.SlicedFormat') > 0
                obj.format = Ice.FormatType.SlicedFormat;
            else
                obj.format = Ice.FormatType.CompactFormat;
            end
        end

        function destroy(obj)
            %DESTROY Destroys the communicator and cleans up memory, shutting down this communicator's client
            %   functionality. Subsequent calls to destroy are ignored.

            arguments
                obj (1, 1) Ice.Communicator
            end
            obj.iceCall('destroy');
        end

        function f = destroyAsync(obj)
            %DESTROYASYNC Asynchronously destroys the communicator. Calling destroyAsync cleans up memory, and shuts
            %   down this communicator's client functionality. Subsequent calls to destroyAsync are ignored.
            %
            %   Output Arguments
            %     f - A future that will be completed when the destruction completes.
            %       Ice.Future scalar

            arguments
                obj (1, 1) Ice.Communicator
            end
            future = libpointer('voidPtr');
            obj.iceCall('destroyAsync', future);
            assert(~isNull(future));
            f = Ice.Future(future, 'destroy', 0, 'Ice_SimpleFuture', @(fut) fut.iceCall('check'));
        end

        function r = stringToProxy(obj, str)
            %STRINGTOPROXY Converts a stringified proxy into a proxy.
            %   Deprecated: Use the constructor of your proxy class instead.
            %
            %   Input Arguments
            %     str - The stringified proxy to convert into a proxy.
            %       character vector
            %
            %   Output Arguments
            %     r - The proxy, or an empty array if str is an empty string.
            %       Ice.ObjectPrx scalar | Ice.ObjectPrx empty array

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
            %PROXYTOSTRING Converts a proxy into a string.
            %
            %   Input Arguments
            %     proxy - The proxy to convert into a stringified proxy.
            %       Ice.ObjectPrx scalar | Ice.ObjectPrx empty array
            %
            %   Output Arguments
            %     r - The stringified proxy, or an empty string if proxy is an empty array.
            %       character vector

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
            %PROPERTYTOPROXY Converts a set of proxy properties into a proxy.
            %   The "base name" supplied in the prop argument refers to a property containing a stringified proxy, such
            %   as:
            %
            %   MyProxy=id:tcp -h localhost -p 10000
            %
            %   Additional properties configure local settings for the proxy, such as:
            %
            %   MyProxy.PreferSecure=1
            %
            %   Input Arguments
            %     prop - The base property name.
            %       character vector
            %
            %   Output Arguments
            %     r - The proxy.
            %       Ice.ObjectPrx scalar | Ice.ObjectPrx empty array

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
            %PROXYTOPROPERTY Converts a proxy to a set of proxy properties.
            %
            %   Input Arguments
            %     proxy - The proxy.
            %       Ice.ObjectPrx scalar | Ice.ObjectPrx empty array
            %     prop - The base property name.
            %       character vector
            %
            %   Output Arguments
            %     r - The property set.
            %       dictionary(string, string) scalar

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
            %IDENTITYTOSTRING Convert an identity into a string.
            %
            %   Input Arguments
            %     id - The identity to convert into a string.
            %       Ice.Identity scalar
            %
            %   Output Arguments
            %     r - The stringified identity.
            %       character vector

            arguments
                obj (1, 1) Ice.Communicator
                id (1, 1) Ice.Identity
            end
            r = obj.iceCallWithResult('identityToString', id);
        end

        function r = getImplicitContext(obj)
            %GETIMPLICITCONTEXT Gets the implicit context associated with this communicator.
            %
            %   Output Arguments
            %     r - The implicit context associated with this communicator.
            %       Ice.ImplicitContext scalar | Ice.ImplicitContext empty array

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
            %GETPROPERTIES Gets the Ice properties for this communicator.
            %
            %   Output Arguments
            %     r - This communicator's properties.
            %       Ice.Properties scalar

            arguments
                obj (1, 1) Ice.Communicator
            end
            if isempty(obj.Properties)
                impl = libpointer('voidPtr');
                obj.iceCall('getProperties', impl);
                obj.Properties = Ice.Properties({}, Ice.Properties.empty, impl);
            end
            r = obj.Properties;
        end

        function r = getLogger(obj)
            %GETLOGGER Gets the logger for this communicator.
            %
            %   Output Arguments
            %     r - This communicator's logger.
            %       Ice.Logger scalar

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
            %GETDEFAULTROUTER Gets the default router for this communicator.
            %
            %   Output Arguments
            %     r - This communicator's default router.
            %       Ice.RouterPrx scalar | Ice.RouterPrx empty array

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
            %SETDEFAULTROUTER Sets a default router for this communicator.
            %   All newly created proxies will use this default router.
            %
            %   Input Arguments
            %     proxy - The default router to use for this communicator.
            %       Ice.RouterPrx scalar | Ice.RouterPrx empty array

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
            %GETDEFAULTLOCATOR Gets the default locator for this communicator.
            %
            %   Output Arguments
            %     r - This communicator's default locator.
            %       Ice.LocatorPrx scalar | Ice.LocatorPrx empty array

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
            %SETDEFAULTLOCATOR Sets a default locator for this communicator.
            %   All newly created proxies will use this default locator.
            %
            %   Input Arguments
            %     proxy - The default locator to use for this communicator.
            %       Ice.LocatorPrx scalar | Ice.LocatorPrx empty array

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
            %FLUSHBATCHREQUESTS Flushes any pending batch requests for this communicator. This means all batch requests
            %   invoked on fixed proxies for all connections associated with the communicator. Any errors that occur
            %   while flushing are ignored.
            %
            %   Input Arguments
            %     mode - Specifies whether or not the queued batch requests should be compressed before being sent over
            %       the wire.
            %       Ice.CompressBatch scalar

            arguments
                obj (1, 1) Ice.Communicator
                mode (1, 1) Ice.CompressBatch
            end
            obj.iceCall('flushBatchRequests', mode);
        end

        function r = flushBatchRequestsAsync(obj, mode)
            %FLUSHBATCHREQUESTSASYNC Flushes any pending batch requests for this communicator. This means all batch
            %   requests invoked on fixed proxies for all connections associated with the communicator. Any errors that
            %   occur while flushing are ignored.
            %
            %   Input Arguments
            %     mode - Specifies whether or not the queued batch requests should be compressed before being sent over
            %       the wire.
            %       Ice.CompressBatch scalar
            %
            %   Output Arguments
            %     r - A future that will be completed when the invocation completes.
            %       Ice.Future scalar

            arguments
                obj (1, 1) Ice.Communicator
                mode (1, 1) Ice.CompressBatch
            end
            future = libpointer('voidPtr');
            obj.iceCall('flushBatchRequestsAsync', mode, future);
            assert(~isNull(future));
            r = Ice.Future(future, 'flushBatchRequests', 0, 'Ice_SimpleFuture', @(fut) fut.iceCall('check'));
        end
    end
    methods (Hidden)
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
            r = obj.SliceLoader;
        end
    end
    properties(Access=private)
        SliceLoader
        encoding
        format
        implicitContext
        Properties
        logger
    end
end
