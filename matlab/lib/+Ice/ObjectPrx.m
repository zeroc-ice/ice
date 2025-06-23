classdef ObjectPrx < IceInternal.WrapperObject
    %OBJECTPRX The base class for all Ice proxies.
    %
    %   Creation
    %     Syntax
    %       prx = Ice.ObjectPrx(communicator, proxyString)
    %
    %     Input Arguments
    %       communicator - The associated communicator.
    %         Ice.Communicator scalar
    %       proxyString - A stringified proxy, such as 'name:tcp -p localhost -p 4061'.
    %         character vector
    %
    %   ObjectPrx Methods:
    %     disp - Displays this proxy as a string.
    %     eq - Compares this proxy with another ObjectPrx for equality.
    %     ice_adapterId - Returns a proxy that is identical to this proxy, except for the adapter ID.
    %     ice_batchDatagram - Returns a proxy that is identical to this proxy, but uses batch datagram invocations.
    %     ice_batchOneway - Returns a proxy that is identical to this proxy, but uses batch oneway invocations.
    %     ice_compress - Returns a proxy that is identical to this proxy, except for compression.
    %     ice_connectionCached - Returns a proxy that is identical to this proxy, except for connection caching.
    %     ice_connectionId - Returns a proxy that is identical to this proxy, except for its connection ID.
    %     ice_context - Returns a proxy that is identical to this proxy, except for the per-proxy context.
    %     ice_datagram - Returns a proxy that is identical to this proxy, but uses datagram invocations.
    %     ice_encodingVersion - Returns a proxy that is identical to this proxy, except for the encoding used to marshal parameters.
    %     ice_endpointSelection - Returns a proxy that is identical to this proxy, except for the endpoint selection policy.
    %     ice_endpoints - Returns a proxy that is identical to this proxy, except for the endpoints.
    %     ice_facet - Returns a proxy that is identical to this proxy, except for the facet.
    %     ice_fixed - Obtains a proxy that is identical to this proxy, except it's a fixed proxy bound to the given connection.
    %     ice_flushBatchRequests - Flushes any pending batched requests for this communicator.
    %     ice_flushBatchRequestsAsync - An asynchronous ice_flushBatchRequests.
    %     ice_getAdapterId - Returns the adapter ID for this proxy.
    %     ice_getCachedConnection - Returns the cached Connection for this proxy.
    %     ice_getCommunicator - Gets the communicator that created this proxy.
    %     ice_getCompress - Obtains the compression override setting of this proxy.
    %     ice_getConnection - Returns the Connection for this proxy.
    %     ice_getConnectionAsync - An asynchronous ice_getConnection.
    %     ice_getConnectionId - Returns the connection id of this proxy.
    %     ice_getContext - Returns the per-proxy context for this proxy.
    %     ice_getEncodingVersion - Returns the encoding version used to marshal requests parameters.
    %     ice_getEndpoints - Returns the endpoints used by this proxy.
    %     ice_getEndpointSelection - Returns how this proxy selects endpoints (randomly or ordered).
    %     ice_getFacet - Returns the facet for this proxy.
    %     ice_getIdentity - Returns the identity embedded in this proxy.
    %     ice_getInvocationTimeout - Returns the invocation timeout of this proxy.
    %     ice_getLocator - Returns the locator for this proxy.
    %     ice_getLocatorCacheTimeout - Returns the locator cache timeout of this proxy.
    %     ice_getRouter - Returns the router for this proxy.
    %     ice_id - Returns the Slice type ID of the most-derived interface supported by the target object of this proxy.
    %     ice_idAsync - An asynchronous ice_id.
    %     ice_identity - Returns a proxy that is identical to this proxy, except for the identity.
    %     ice_ids - Returns the Slice type IDs of the interfaces supported by the target object of this proxy.
    %     ice_idsAsync - Returns the Slice type IDs of the interfaces supported by the target object of this proxy.
    %     ice_invocationTimeout - Returns a proxy that is identical to this proxy, except for the invocation timeout.
    %     ice_isA - Tests whether this object supports a specific Slice interface.
    %     ice_isAAsync - An asynchronous ice_isA.
    %     ice_isBatchDatagram - Returns whether this proxy uses batch datagram invocations.
    %     ice_isBatchOneway - Returns whether this proxy uses batch oneway invocations.
    %     ice_isConnectionCached - Returns whether this proxy caches connections.
    %     ice_isDatagram - Returns whether this proxy uses datagram invocations.
    %     ice_isFixed - Determines whether this proxy is a fixed proxy.
    %     ice_isOneway - Returns whether this proxy uses oneway invocations.
    %     ice_isPreferSecure - Returns whether this proxy prefers secure endpoints.
    %     ice_isSecure - Returns whether this proxy uses only secure endpoints.
    %     ice_isTwoway - Returns whether this proxy uses twoway invocations.
    %     ice_locator - Returns a proxy that is identical to this proxy, except for the locator.
    %     ice_locatorCacheTimeout - Returns a proxy that is identical to this proxy, except for the locator cache timeout.
    %     ice_oneway - Returns a proxy that is identical to this proxy, but uses oneway invocations.
    %     ice_ping - Tests whether the target object of this proxy can be reached.
    %     ice_pingAsync - An asynchronous ice_ping.
    %     ice_preferSecure - Returns a proxy that is identical to this proxy, except for its endpoint selection policy.
    %     ice_router - Returns a proxy that is identical to this proxy, except for the router.
    %     ice_secure - Returns a proxy that is identical to this proxy, except for how it selects endpoints.
    %     ice_toString - Creates a stringified version of this proxy.
    %     ice_twoway - Returns a proxy that is identical to this proxy, but uses twoway invocations.

    % Copyright (c) ZeroC, Inc.

    methods
        function obj = ObjectPrx(communicator, proxyString, impl, encoding)
            %OBJECTPRX Constructs an Ice.ObjectPrx from a communicator and a proxy string.
            %
            %   Input Arguments
            %     communicator - The associated communicator.
            %       Ice.Communicator scalar
            %     proxyString - A stringified proxy, such as 'name:tcp -p localhost -p 4061'.
            %       character vector
            %
            %   Output Arguments
            %     obj - The new Ice.ObjectPrx.

            if nargin == 0 % default constructor, typically called with multiple inheritance
                superArgs = {};
            else
                assert(nargin >= 2 && nargin <= 4, 'ObjectPrx constructor requires 2 to 4 arguments');

                % validate arguments
                if ~isa(communicator, 'Ice.Communicator')
                    error('Ice:ArgumentException', 'communicator must be an Ice.Communicator');
                end
                if ~ischar(proxyString)
                    error('Ice:ArgumentException', 'proxyString must be a char array');
                end

                if nargin < 3
                    impl = libpointer('voidPtr');
                    communicator.iceCall('stringToProxy', proxyString, impl);
                    assert(~isNull(impl), 'Invalid proxy string');
                    encoding = [];
                    superArgs = {impl, 'Ice_ObjectPrx'};
                else
                    assert(isempty(proxyString), 'proxyString must be empty');
                    assert(~isempty(impl), 'impl must be non-empty');
                    if nargin == 3
                        encoding = [];
                    end
                    superArgs = {impl, 'Ice_ObjectPrx'};
                end
            end
            obj@IceInternal.WrapperObject(superArgs{:});

            if nargin > 0
                obj.communicator = communicator;
                if isempty(encoding)
                    encoding = obj.iceCallWithResult('ice_getEncodingVersion');
                end
                obj.encoding = encoding;
                obj.isTwoway = obj.iceCallWithResult('ice_isTwoway');
            end
            % else, we leave the properties unset as they may be already set by another call to the same constructor
            % when using multiple inheritance.
        end

        function r = eq(obj, other)
            %EQ Compares this proxy with another ObjectPrx for equality.
            %   See also eq.
            if isempty(other) || ~isa(other, 'Ice.ObjectPrx')
                r = false;
            else
                %
                % Call into C++ to compare the two proxies.
                %
                r = obj.iceCallWithResult('equals', other.impl_);
            end
        end

        function r = ice_toString(obj)
            %ICE_TOSTRING Creates a stringified version of this proxy.
            %
            %   Output Arguments
            %     r - A stringified proxy.
            %       character vector

            arguments
                obj (1, 1) Ice.ObjectPrx
            end
            r = obj.iceCallWithResult('ice_toString');
        end

        function disp(obj)
            %DISP Displays this proxy as a string.
            %   This method is called when the object is displayed in the command window.

            arguments
                obj (1, 1) Ice.ObjectPrx
            end
            builtin('disp', obj.ice_toString());
        end

        function r = ice_getCommunicator(obj)
            %ICE_GETCOMMUNICATOR Gets the communicator that created this proxy.
            %
            %   Output Arguments
            %     r - The communicator that created this proxy.
            %       Ice.Communicator scalar

            arguments
                obj (1, 1) Ice.ObjectPrx
            end
            r = obj.communicator;
        end

        function ice_ping(obj, context)
            %ICE_PING Tests whether the target object of this proxy can be reached.
            %
            %   Input Arguments
            %     context - The request context.
            %       unconfigured dictionary (default) | dictionary(string, string) scalar

            arguments
                obj (1, 1) Ice.ObjectPrx
                context (1, 1) dictionary = dictionary
            end
            obj.iceInvoke('ice_ping', 2, false, [], false, {}, context);
        end

        function r = ice_pingAsync(obj, context)
            %ICE_PINGASYNC Tests whether the target object of this proxy can be reached.
            %
            %   Input Arguments
            %     context - The request context.
            %       unconfigured dictionary (default) | dictionary(string, string) scalar
            %
            %   Output Arguments
            %     future - A future that will be completed with the result of the invocation.
            %       Ice.Future scalar
            %
            %   See also ice_ping, Ice.Future.

            arguments
                obj (1, 1) Ice.ObjectPrx
                context (1, 1) dictionary = dictionary
            end
            r = obj.iceInvokeAsync('ice_ping', 2, false, [], 0, [], {}, context);
        end

        function r = ice_isA(obj, id, context)
            %ICE_ISA Tests whether this object supports a specific Slice interface.
            %
            %   Input Arguments
            %     id - The type ID of the Slice interface to test against.
            %       character vector
            %     context - The request context.
            %       unconfigured dictionary (default) | dictionary(string, string) scalar
            %
            %   Output Arguments
            %     r - True if the target object implements the Slice interface specified by id or implements a
            %       derived interface, false otherwise.
            %       logical scalar

            arguments
                obj (1, 1) Ice.ObjectPrx
                id (1, :) char
                context (1, 1) dictionary = dictionary
            end
            os = obj.iceStartWriteParams([]);
            os.writeString(id);
            obj.iceEndWriteParams(os);
            is = obj.iceInvoke('ice_isA', 2, true, os, true, {}, context);
            is.startEncapsulation();
            r = is.readBool();
            is.endEncapsulation();
        end

        function r = ice_isAAsync(obj, id, context)
            %ICE_ISAASYNC Tests whether this object supports a specific Slice interface.
            %
            %   Input Arguments
            %     id - The type ID of the Slice interface to test against.
            %       character vector
            %     context - The request context.
            %       unconfigured dictionary (default) | dictionary(string, string) scalar
            %
            %   Output Arguments
            %     r - A future that will be completed with the result of the invocation.
            %       Ice.Future scalar
            %
            %   See also ice_isA, Ice.Future.

            arguments
                obj (1, 1) Ice.ObjectPrx
                id (1, :) char
                context (1, 1) dictionary = dictionary
            end
            os = obj.iceStartWriteParams([]);
            os.writeString(id);
            obj.iceEndWriteParams(os);
            function varargout = unmarshal(is)
                is.startEncapsulation();
                varargout{1} = is.readBool();
                is.endEncapsulation();
            end
            r = obj.iceInvokeAsync('ice_isA', 2, true, os, 1, @unmarshal, {}, context);
        end

        function r = ice_id(obj, context)
            %ICE_ID Returns the Slice type ID of the most-derived interface supported by the target object of this
            %   proxy.
            %
            %   Input Arguments
            %     context - The request context.
            %       unconfigured dictionary (default) | dictionary(string, string) scalar
            %
            %   Output Arguments
            %     r - The Slice type ID of the most-derived interface.
            %       character vector

            arguments
                obj (1, 1) Ice.ObjectPrx
                context (1, 1) dictionary = dictionary
            end
            is = obj.iceInvoke('ice_id', 2, true, [], true, {}, context);
            is.startEncapsulation();
            r = is.readString();
            is.endEncapsulation();
        end

        function r = ice_idAsync(obj, context)
            %ICE_IDASYNC Returns the Slice type ID of the most-derived interface supported by the target object of this
            %   proxy.
            %
            %   Input Arguments
            %     context - The request context.
            %       unconfigured dictionary (default) | dictionary(string, string) scalar
            %
            %   Output Arguments
            %     r - A future that will be completed with the result of the invocation.
            %       Ice.Future scalar
            %
            %   See also ice_id, Ice.Future.

            arguments
                obj (1, 1) Ice.ObjectPrx
                context (1, 1) dictionary = dictionary
            end
            function varargout = unmarshal(is)
                is.startEncapsulation();
                varargout{1} = is.readString();
                is.endEncapsulation();
            end
            r = obj.iceInvokeAsync('ice_id', 2, true, [], 1, @unmarshal, {}, context);
        end

        function r = ice_ids(obj, context)
            %ICE_IDS Returns the Slice type IDs of the interfaces supported by the target object of this proxy.
            %
            %   Input Arguments
            %     context - The request context.
            %       unconfigured dictionary (default) | dictionary(string, string) scalar
            %
            %   Output Arguments
            %     r - The Slice type IDs of the interfaces supported by the target object, in alphabetical order.
            %       string array

            arguments
                obj (1, 1) Ice.ObjectPrx
                context (1, 1) dictionary = dictionary
            end
            is = obj.iceInvoke('ice_ids', 2, true, [], true, {}, context);
            is.startEncapsulation();
            r = is.readStringSeq();
            is.endEncapsulation();
        end

        function r = ice_idsAsync(obj, context)
            %ICE_IDSASYNC Returns the Slice type IDs of the interfaces supported by the target object of this proxy.
            %
            %   Input Arguments
            %     context - The request context.
            %       unconfigured dictionary (default) | dictionary(string, string) scalar
            %
            %   Output Arguments
            %     r - A future that will be completed with the result of the invocation.
            %       Ice.Future scalar
            %
            %   See also ice_ids, Ice.Future.

            arguments
                obj (1, 1) Ice.ObjectPrx
                context (1, 1) dictionary = dictionary
            end
            function varargout = unmarshal(is)
                is.startEncapsulation();
                varargout{1} = is.readStringSeq();
                is.endEncapsulation();
            end
            r = obj.iceInvokeAsync('ice_ids', 2, true, [], 1, @unmarshal, {}, context);
        end

        function r = ice_getIdentity(obj)
            %ICE_GETIDENTITY Returns the identity embedded in this proxy.
            %
            %   Output Arguments
            %     r - The identity of the target object.
            %       Ice.Identity scalar

            arguments
                obj (1, 1) Ice.ObjectPrx
            end
            r = obj.iceCallWithResult('ice_getIdentity');
        end

        function r = ice_identity(obj, id)
            %ICE_IDENTITY Returns a proxy that is identical to this proxy, except for the identity.
            %
            %   Input Arguments
            %     id - The identity for the new proxy.
            %       Ice.Identity scalar
            %
            %   Output Arguments
            %     r - The proxy with the new identity.
            %       Ice.ObjectPrx scalar

            arguments
                obj (1, 1) Ice.ObjectPrx
                id (1, 1) Ice.Identity
            end
            r = obj.factory_('ice_identity', false, id);
        end

        function r = ice_getContext(obj)
            %ICE_GETCONTEXT Returns the per-proxy context for this proxy.
            %
            %   Output Arguments
            %     r - The per-proxy context.
            %       dictionary(string, string) scalar

            arguments
                obj (1, 1) Ice.ObjectPrx
            end
            r = obj.iceCallWithResult('ice_getContext');
        end

        function r = ice_context(obj, context)
            %ICE_CONTEXT Returns a proxy that is identical to this proxy, except for the per-proxy context.
            %
            %   Input Arguments
            %     context - The context for the new proxy.
            %       dictionary(string, string) scalar
            %
            %   Output Arguments
            %     r - The proxy with the new per-proxy context.
            %       proxy scalar with the same type as obj

            arguments
                obj (1, 1) Ice.ObjectPrx
                context (1, 1) dictionary {Ice.mustBeStringStringDictionary}
            end
            r = obj.factory_('ice_context', true, context);
        end

        function r = ice_getFacet(obj)
            %ICE_GETFACET Returns the facet for this proxy.
            %
            %   Output Arguments
            %     r - The facet for this proxy.
            %       character vector

            arguments
                obj (1, 1) Ice.ObjectPrx
            end
            r = obj.iceCallWithResult('ice_getFacet');
        end

        function r = ice_facet(obj, f)
            %ICE_FACET Returns a proxy that is identical to this proxy, except for the facet.
            %
            %   Input Arguments
            %     f - The facet for the new proxy.
            %       character vector
            %
            %   Output Arguments
            %     r - The proxy with the new facet.
            %       Ice.ObjectPrx scalar

            arguments
                obj (1, 1) Ice.ObjectPrx
                f (1, :) char
            end
            r = obj.factory_('ice_facet', false, f);
        end

        function r = ice_getAdapterId(obj)
            %ICE_GETADAPTERID Returns the adapter ID for this proxy.
            %
            %   Output Arguments
            %     r - The adapter ID. If the proxy does not have an adapter ID, r is the empty string.
            %       character vector

            arguments
                obj (1, 1) Ice.ObjectPrx
            end
            r = obj.iceCallWithResult('ice_getAdapterId');
        end

        function r = ice_adapterId(obj, id)
            %ICE_ADAPTERID Returns a proxy that is identical to this proxy, except for the adapter ID.
            %
            %   Input Arguments
            %     id - The adapter ID for the new proxy.
            %       character vector
            %
            %   Output Arguments
            %     r - The proxy with the new adapter ID.
            %        proxy scalar with the same type as obj

            arguments
                obj (1, 1) Ice.ObjectPrx
                id (1, :) char
            end
            r = obj.factory_('ice_adapterId', true, id);
        end

        function r = ice_getEndpoints(obj)
            %ICE_GETENDPOINTS Returns the endpoints used by this proxy.
            %
            %   Output Arguments
            %     r - The endpoints used by this proxy.
            %       cell array of Ice.Endpoint

            arguments
                obj (1, 1) Ice.ObjectPrx
            end
            num = obj.iceCallWithResult('ice_getNumEndpoints');
            r = cell(1, num);
            for i = 1:num
                impl = libpointer('voidPtr');
                obj.iceCallWithResult('ice_getEndpoint', i - 1, impl); % C-style index
                assert(~isNull(impl));
                r{i} = Ice.Endpoint(impl);
            end
        end

        function r = ice_endpoints(obj, endpoints)
            %ICE_ENDPOINTS Returns a proxy that is identical to this proxy, except for the endpoints.
            %
            %   Input Arguments
            %     endpoints - The endpoints for the new proxy.
            %       cell array of Ice.Endpoint
            %
            %   Output Arguments
            %     r - The proxy with the new endpoints.
            %        proxy scalar with the same type as obj

            arguments
                obj (1, 1) Ice.ObjectPrx
                endpoints (1, :) cell
            end

            % It's not clear how we can pass a vector of void* to a C function. So we create a temporary C vector
            % and populate it one element at a time.
            for i = 1:length(endpoints)
                if ~isa(endpoints{i}, 'Ice.Endpoint')
                   error('Ice:ArgumentException', 'Expected an Ice.Endpoint');
                end
            end
            arr = libpointer('voidPtr');
            obj.iceCall('ice_createEndpointList', length(endpoints), arr);
            for i = 1:length(endpoints)
                obj.iceCall('ice_setEndpoint', arr, i - 1, endpoints{i}.impl_); % C-style index
            end
            r = obj.factory_('ice_endpoints', true, arr); % The C function also destroys the temporary array.
        end

        function r = ice_getLocatorCacheTimeout(obj)
            %ICE_GETLOCATORCACHETIMEOUT Returns the locator cache timeout of this proxy.
            %
            %   Output Arguments
            %     r - The locator cache timeout value (in seconds).
            %       int32 scalar

            arguments
                obj (1, 1) Ice.ObjectPrx
            end
            r = obj.iceCallWithResult('ice_getLocatorCacheTimeout');
        end

        function r = ice_locatorCacheTimeout(obj, t)
            %ICE_LOCATORCACHETIMEOUT Returns a proxy that is identical to this proxy, except for the locator cache
            %   timeout.
            %
            %   Input Arguments
            %     t - The new locator cache timeout (in seconds).
            %       int32 scalar
            %
            %   Output Arguments
            %     r - The proxy with the new timeout.
            %       proxy scalar with the same type as obj

            arguments
                obj (1, 1) Ice.ObjectPrx
                t (1, 1) int32
            end
            r = obj.factory_('ice_locatorCacheTimeout', true, t);
        end

        function r = ice_getInvocationTimeout(obj)
            %ICE_GETINVOCATIONTIMEOUT Returns the invocation timeout of this proxy.
            %
            %   Output Arguments
            %     r - The invocation timeout value (in seconds).
            %       int32 scalar

            arguments
                obj (1, 1) Ice.ObjectPrx
            end
            r = obj.iceCallWithResult('ice_getInvocationTimeout');
        end

        function r = ice_invocationTimeout(obj, t)
            %ICE_INVOCATIONTIMEOUT Returns a proxy that is identical to this proxy, except for the invocation timeout.
            %
            %   Input Arguments
            %     t - The new invocation timeout (in seconds).
            %       int32 scalar
            %
            %   Output Arguments
            %     r - The proxy with the new timeout.
            %       proxy scalar with the same type as obj

            arguments
                obj (1, 1) Ice.ObjectPrx
                t (1, 1) int32
            end
            r = obj.factory_('ice_invocationTimeout', true, t);
        end

        function r = ice_getConnectionId(obj)
            %ICE_GETCONNECTIONID Returns the connection ID of this proxy.
            %
            %   Output Arguments
            %      r - The connection ID.
            %        character vector

            arguments
                obj (1, 1) Ice.ObjectPrx
            end
            r = obj.iceCallWithResult('ice_getConnectionId');
        end

        function r = ice_connectionId(obj, id)
            %ICE_CONNECTIONID Returns a proxy that is identical to this proxy, except for its connection ID.
            %
            %   Input Arguments
            %      id - The connection ID for the new proxy. An empty string removes the connection ID.
            %        character vector
            %
            %   Output Arguments
            %      r - A proxy with the specified connection ID.
            %        proxy scalar with the same type as obj

            arguments
                obj (1, 1) Ice.ObjectPrx
                id (1, :) char
            end
            r = obj.factory_('ice_connectionId', true, id);
        end

        function r = ice_isConnectionCached(obj)
            %ICE_ISCONNECTIONCACHED Returns whether this proxy caches connections.
            %
            %   Output Arguments
            %      r - True if this proxy caches connections; false otherwise.
            %        logical scalar

            arguments
                obj (1, 1) Ice.ObjectPrx
            end
            r = obj.iceCallWithResult('ice_isConnectionCached');
        end

        function r = ice_connectionCached(obj, b)
            %ICE_CONNECTIONCACHED Returns a proxy that is identical to this proxy, except for connection caching.
            %
            %   Input Arguments
            %      b - True if the new proxy should cache connections; false otherwise.
            %        logical scalar
            %
            %   Output Arguments
            %      r - The proxy with the specified caching policy.
            %        proxy scalar with the same type as obj

            arguments
                obj (1, 1) Ice.ObjectPrx
                b (1, 1) logical
            end
            if b
                val = 1;
            else
                val = 0;
            end
            r = obj.factory_('ice_connectionCached', true, val);
        end

        function r = ice_getEndpointSelection(obj)
            %ICE_GETENDPOINTSELECTION Returns how this proxy selects endpoints (randomly or ordered).
            %
            %   Output Arguments
            %      r - The endpoint selection policy.
            %        Ice.EndpointSelectionType scalar

            arguments
                obj (1, 1) Ice.ObjectPrx
            end
            r = Ice.EndpointSelectionType.ice_getValue(obj.iceCallWithResult('ice_getEndpointSelection'));
        end

        function r = ice_endpointSelection(obj, t)
            %ICE_ENDPOINTSELECTION Returns a proxy that is identical to this proxy, except for the endpoint
            %   selection policy.
            %
            %   Input Arguments
            %      t - The new endpoint selection policy.
            %        Ice.EndpointSelectionType scalar
            %
            %   Output Arguments
            %      r - The proxy with the specified endpoint selection policy.
            %        proxy scalar with the same type as obj

            arguments
                obj (1, 1) Ice.ObjectPrx
                t (1, 1) Ice.EndpointSelectionType
            end
            r = obj.factory_('ice_endpointSelection', true, t);
        end

        function r = ice_getEncodingVersion(obj)
            %ICE_GETENCODINGVERSION Returns the encoding version used to marshal requests parameters.
            %
            %   Output Arguments
            %      r - The encoding version.
            %        Ice.EncodingVersion scalar

            arguments
                obj (1, 1) Ice.ObjectPrx
            end
            r = obj.encoding;
        end

        function r = ice_encodingVersion(obj, ver)
            %ICE_ENCODINGVERSION Returns a proxy that is identical to this proxy, except for the encoding used to
            %   marshal parameters.
            %
            %   Input Arguments
            %      ver - The encoding version to use to marshal request parameters.
            %        Ice.EncodingVersion scalar
            %
            %   Output Arguments
            %      r - The proxy with the specified encoding version.
            %        proxy scalar with the same type as obj

            arguments
                obj (1, 1) Ice.ObjectPrx
                ver (1, 1) Ice.EncodingVersion
            end
            r = obj.factory_('ice_encodingVersion', true, ver);
            r.encoding = ver;
        end

        function r = ice_getRouter(obj)
            %ICE_GETROUTER Returns the router for this proxy.
            %
            %   Output Arguments
            %      r - The router for the proxy.
            %        Ice.RouterPrx scalar | Ice.RouterPrx empty array

            arguments
                obj (1, 1) Ice.ObjectPrx
            end
            v = libpointer('voidPtr');
            obj.iceCall('ice_getRouter', v);
            if isNull(v)
                r = Ice.RouterPrx.empty;
            else
                r = Ice.RouterPrx(obj.communicator, '', v);
            end
        end

        function r = ice_router(obj, rtr)
            %ICE_ROUTER Returns a proxy that is identical to this proxy, except for the router.
            %
            %   Input Arguments
            %      rtr - The router for the new proxy.
            %        Ice.RouterPrx scalar | Ice.RouterPrx empty array
            %
            %   Output Arguments
            %      r - The proxy with the specified router.
            %        proxy scalar with the same type as obj

            arguments
                obj (1, 1) Ice.ObjectPrx
                rtr Ice.RouterPrx {mustBeScalarOrEmpty}
            end
            if isempty(rtr)
                impl = libpointer('voidPtr');
            else
                impl = rtr.impl_;
            end
            r = obj.factory_('ice_router', true, impl);
        end

        function r = ice_getLocator(obj)
            %ICE_GETLOCATOR Returns the locator for this proxy.
            %
            %   Output Arguments
            %      r - The locator for the proxy.
            %        Ice.LocatorPrx scalar | Ice.LocatorPrx empty array

            arguments
                obj (1, 1) Ice.ObjectPrx
            end
            v = libpointer('voidPtr');
            obj.iceCall('ice_getLocator', v);
            if isNull(v)
                r = Ice.LocatorPrx.empty;
            else
                r = Ice.LocatorPrx(obj.communicator, '', v);
            end
        end

        function r = ice_locator(obj, loc)
            %ICE_LOCATOR Returns a proxy that is identical to this proxy, except for the locator.
            %
            %   Input Arguments
            %      loc - The locator for the new proxy.
            %        Ice.LocatorPrx scalar | Ice.LocatorPrx empty array
            %
            %   Output Arguments
            %      r - The proxy with the specified locator.
            %        proxy scalar with the same type as obj

            arguments
                obj (1, 1) Ice.ObjectPrx
                loc Ice.LocatorPrx {mustBeScalarOrEmpty}
            end
            if isempty(loc)
                impl = libpointer('voidPtr');
            else
                impl = loc.impl_;
            end
            r = obj.factory_('ice_locator', true, impl);
        end

        function r = ice_isSecure(obj)
            %ICE_ISSECURE Returns whether this proxy uses only secure endpoints.
            %
            %   Output Arguments
            %      r - True if this proxy communicates only via secure endpoints; false otherwise.
            %        logical scalar

            arguments
                obj (1, 1) Ice.ObjectPrx
            end
            r = obj.iceCallWithResult('ice_isSecure');
        end

        function r = ice_secure(obj, b)
            %ICE_SECURE Returns a proxy that is identical to this proxy, except for how it selects endpoints.
            %
            %   Input Arguments
            %      b - If b is true, only endpoints that use a secure transport are used by the new proxy. If b is
            %        false, the returned proxy uses both secure and insecure endpoints.
            %        logical scalar
            %
            %   Output Arguments
            %      r - The proxy with the specified selection policy.
            %        proxy scalar with the same type as obj

            arguments
                obj (1, 1) Ice.ObjectPrx
                b (1, 1) logical
            end
            if b
                val = 1;
            else
                val = 0;
            end
            r = obj.factory_('ice_secure', true, val);
        end

        function r = ice_isPreferSecure(obj)
            %ICE_ISPREFERSECURE Returns whether this proxy prefers secure endpoints.
            %
            %   Output Arguments
            %      r - True if the proxy always attempts to invoke via secure endpoints before it attempts to use
            %        insecure endpoints; false otherwise.
            %        logical scalar

            arguments
                obj (1, 1) Ice.ObjectPrx
            end
            r = obj.iceCallWithResult('ice_isPreferSecure');
        end

        function r = ice_preferSecure(obj, b)
            %ICE_PREFERSECURE Returns a proxy that is identical to this proxy, except for its endpoint selection
            %   policy.
            %
            %   Input Arguments
            %      b - If b is true, the new proxy will use secure endpoints for invocations and only use insecure
            %        endpoints if an invocation cannot be made via secure endpoints. If b is false, the proxy prefers
            %        insecure endpoints to secure ones.
            %        logical scalar

            %
            %   Output Arguments
            %      r - The proxy with the specified selection policy.
            %        proxy scalar with the same type as obj

            arguments
                obj (1, 1) Ice.ObjectPrx
                b (1, 1) logical
            end
            if b
                val = 1;
            else
                val = 0;
            end
            r = obj.factory_('ice_preferSecure', true, val);
        end

        function r = ice_isTwoway(obj)
            %ICE_ISTWOWAY Returns whether this proxy uses twoway invocations.
            %
            %   Output Arguments
            %     r - True if this proxy uses twoway invocations; false otherwise.
            %       logical scalar

            arguments
                obj (1, 1) Ice.ObjectPrx
            end
            r = obj.isTwoway;
        end

        function r = ice_twoway(obj)
            %ICE_TWOWAY Returns a proxy that is identical to this proxy, but uses twoway invocations.
            %
            %   Output Arguments
            %     r - A proxy that uses twoway invocations.
            %       proxy scalar with the same type as obj

            arguments
                obj (1, 1) Ice.ObjectPrx
            end
            r = obj.factory_('ice_twoway', true);
        end

        function r = ice_isOneway(obj)
            %ICE_ISONEWAY Returns whether this proxy uses oneway invocations.
            %
            %   Output Arguments
            %     r - True if this proxy uses oneway invocations; false otherwise.
            %       logical scalar

            arguments
                obj (1, 1) Ice.ObjectPrx
            end
            r = obj.iceCallWithResult('ice_isOneway');
        end

        function r = ice_oneway(obj)
            %ICE_ONEWAY Returns a proxy that is identical to this proxy, but uses oneway invocations.
            %
            %   Output Arguments
            %     r - A proxy that uses oneway invocations.
            %       proxy scalar with the same type as obj

            arguments
                obj (1, 1) Ice.ObjectPrx
            end
            r = obj.factory_('ice_oneway', true);
        end

        function r = ice_isBatchOneway(obj)
            %ICE_ISBATCHONEWAY Returns whether this proxy uses batch oneway invocations.
            %
            %   Output Arguments
            %     r - True if this proxy uses batch oneway invocations; false otherwise.
            %       logical scalar

            arguments
                obj (1, 1) Ice.ObjectPrx
            end
            r = obj.iceCallWithResult('ice_isBatchOneway');
        end

        function r = ice_batchOneway(obj)
            %ICE_BATCHONEWAY Returns a proxy that is identical to this proxy, but uses batch oneway invocations.
            %
            %   Output Arguments
            %     r - A new proxy that uses batch oneway invocations.
            %       proxy scalar with the same type as obj

            arguments
                obj (1, 1) Ice.ObjectPrx
            end
            r = obj.factory_('ice_batchOneway', true);
        end

        function r = ice_isDatagram(obj)
            %ICE_ISDATAGRAM Returns whether this proxy uses datagram invocations.
            %
            %   Output Arguments
            %     r - True if this proxy uses datagram invocations; false otherwise.
            %       logical scalar

            arguments
                obj (1, 1) Ice.ObjectPrx
            end
            r = obj.iceCallWithResult('ice_isDatagram');
        end

        function r = ice_datagram(obj)
            %ICE_DATAGRAM Returns a proxy that is identical to this proxy, but uses datagram invocations.
            %
            %   Output Arguments
            %     r - A new proxy that uses datagram invocations.
            %       proxy scalar with the same type as obj

            arguments
                obj (1, 1) Ice.ObjectPrx
            end
            r = obj.factory_('ice_datagram', true);
        end

        function r = ice_isBatchDatagram(obj)
            %ICE_ISBATCHDATAGRAM Returns whether this proxy uses batch datagram invocations.
            %
            %   Output Arguments
            %     r - True if this proxy uses batch datagram invocations; false otherwise.
            %       logical scalar

            arguments
                obj (1, 1) Ice.ObjectPrx
            end
            r = obj.iceCallWithResult('ice_isBatchDatagram');
        end

        function r = ice_batchDatagram(obj)
            %ICE_BATCHDATAGRAM Returns a proxy that is identical to this proxy, but uses batch datagram invocations.
            %
            %   Output Arguments
            %     r - A new proxy that uses batch datagram invocations.
            %       proxy scalar with the same type as obj

            arguments
                obj (1, 1) Ice.ObjectPrx
            end
            r = obj.factory_('ice_batchDatagram', true);
        end

        function r = ice_compress(obj, b)
            %ICE_COMPRESS Returns a proxy that is identical to this proxy, except for compression.
            %
            %   Input Arguments
            %     b - True enables compression for the new proxy; false disables compression.
            %       logical scalar
            %
            %   Output Arguments
            %     r - A proxy with the specified compression override setting.
            %       proxy scalar with the same type as obj

            arguments
                obj (1, 1) Ice.ObjectPrx
                b (1, 1) logical
            end
            if b
                val = 1;
            else
                val = 0;
            end
            r = obj.factory_('ice_compress', true, val);
        end

        function r = ice_getCompress(obj)
            %ICE_GETCOMPRESS Obtains the compression override setting of this proxy.
            %
            %   Output Arguments
            %     r - The compression override setting. If Ice.Unset is returned, no override is set. Otherwise,
            %       true if compression is enabled, false otherwise.
            %       logical scalar | Ice.Unset scalar

            arguments
                obj (1, 1) Ice.ObjectPrx
            end
            opt = obj.iceCallWithResult('ice_getCompress');
            if opt.hasValue
                r = opt.value;
            else
                r = IceInternal.UnsetI.Instance;
            end
        end

        function r = ice_fixed(obj, connection)
            %ICE_FIXED Obtains a proxy that is identical to this proxy, except it's a fixed proxy bound to the
            %   given connection.
            %
            %   Input Arguments
            %     connection - The fixed proxy connection.
            %       Ice.Connection scalar
            %
            %   Output Arguments
            %     r - A fixed proxy bound to the given connection.
            %       proxy scalar with the same type as obj

            arguments
                obj (1, 1) Ice.ObjectPrx
                connection (1, 1) Ice.Connection
            end
            r = obj.factory_('ice_fixed', true, connection.iceGetImpl());
        end

        function r = ice_isFixed(obj)
            %ICE_ISFIXED Determines whether this proxy is a fixed proxy.
            %
            %   Output Arguments
            %     r - True if this proxy is a fixed proxy, false otherwise.
            %       logical scalar

            arguments
                obj (1, 1) Ice.ObjectPrx
            end
            r = obj.iceCallWithResult('ice_isFixed');
        end

        function r = ice_getConnection(obj)
            %ICE_GETCONNECTION Returns the Connection for this proxy. If the proxy does not yet have an
            %   established connection, it first attempts to create a connection.
            %
            %   Output Arguments
            %     r - The Connection for this proxy.
            %       Ice.Connection scalar

            arguments
                obj (1, 1) Ice.ObjectPrx
            end
            v = libpointer('voidPtr');
            obj.iceCall('ice_getConnection', v);
            if isNull(v)
                r = Ice.Connection.empty;
            else
                r = Ice.Connection(v, obj.communicator);
            end
        end

        function r = ice_getConnectionAsync(obj)
            %ICE_GETCONNECTIONASYNC Returns the Connection for this proxy.
            %   If the proxy does not yet have an established connection, it first attempts to create a connection.
            %
            %   Output Arguments
            %     r - A future that will be completed when the invocation completes.
            %       Ice.Future scalar

            arguments
                obj (1, 1) Ice.ObjectPrx
            end
            future = libpointer('voidPtr');
            obj.iceCall('ice_getConnectionAsync', future);
            assert(~isNull(future));
            function varargout = fetch(f)
                con = libpointer('voidPtr', 0); % Output param
                f.iceCall('fetch', con);
                assert(~isNull(con));
                varargout{1} = Ice.Connection(con);
            end
            r = Ice.Future(future, 'ice_getConnection', 1, 'Ice_GetConnectionFuture', @fetch);
        end

        function r = ice_getCachedConnection(obj)
            %ICE_GETCACHEDCONNECTION Returns the cached Connection for this proxy.
            %   If the proxy does not yet have an established connection, it does not attempt to create a connection.
            %
            %   Output Arguments
            %     r - The cached Connection for this proxy, or an empty array if the proxy does not have a cached
            %       connection.
            %       Ice.Connection scalar | Ice.Connection empty array

            arguments
                obj (1, 1) Ice.ObjectPrx
            end
            v = libpointer('voidPtr');
            obj.iceCall('ice_getCachedConnection', v);
            if isNull(v)
                r = [];
            else
                r = Ice.Connection(v, obj.communicator);
            end
        end

        function ice_flushBatchRequests(obj)
            %ICE_FLUSHBATCHREQUESTS Flushes any pending batched requests for this communicator.
            %   The call blocks until the flush is complete.

            arguments
                obj (1, 1) Ice.ObjectPrx
            end
            obj.iceCall('ice_flushBatchRequests');
        end

        function r = ice_flushBatchRequestsAsync(obj)
            %ICE_FLUSHBATCHREQUESTSASYNC Flushes asynchronously any pending batched requests for this communicator.
            %
            %   Output Arguments
            %     r - A future that will be completed when the invocation completes.
            %       Ice.Future scalar

            arguments
                obj (1, 1) Ice.ObjectPrx
            end
            future = libpointer('voidPtr');
            obj.iceCall('ice_flushBatchRequestsAsync', future);
            assert(~isNull(future));
            r = Ice.Future(future, 'ice_flushBatchRequests', 0, 'Ice_SimpleFuture', @(fut) fut.iceCall('check'));
        end
    end

    methods(Hidden=true)
        function iceWrite(obj, os, encoding)
           bytes = obj.iceCallWithResult('write', obj.communicator.impl_, encoding);
           os.writeBlob(bytes);
        end
    end

    methods(Access=protected)
        function os = iceStartWriteParams(obj, format)
            os = Ice.OutputStream(obj.encoding, obj.communicator.getFormat());
            os.startEncapsulation(format);
        end

        function iceEndWriteParams(~, os)
            os.endEncapsulation();
        end

        function is = iceInvoke(obj, op, mode, twowayOnly, os, hasOutParams, exceptions, ctx)
            try
                % Replace unconfigured dictionary with empty array.
                if ~isConfigured(ctx)
                    ctx = [];
                end

                if twowayOnly && ~obj.isTwoway
                    throw(Ice.TwowayOnlyException(op));
                end

                if isempty(os)
                    buf = [];
                    size = 0;
                else
                    buf = os.buf.buf;
                    size = os.buf.size;
                end

                res = IceInternal.Util.callWithResult('Ice_ObjectPrx_ice_invoke', obj.impl_, op, mode, buf, size, ctx);

                is = [];
                if ~isempty(res.params)
                    if isempty(obj.cachedInputStream)
                        is = Ice.InputStream(obj.communicator, obj.encoding, res.params);
                        obj.cachedInputStream = is;
                    else
                        is = obj.cachedInputStream;
                        is.reset(res.params);
                    end
                end

                if obj.isTwoway
                    if ~res.ok
                        obj.iceThrowUserException(is, exceptions{:});
                    elseif ~hasOutParams
                        is.skipEmptyEncapsulation();
                    end
                end
            catch ex
                ex.throwAsCaller();
            end
        end

        function fut = iceInvokeAsync(obj, op, mode, twowayOnly, os, numOutArgs, unmarshalFunc, exceptions, ctx)
            isTwoway = obj.isTwoway;

            % This nested function is invoked by Future.fetchOutputs()
            function varargout = fetch(f)
                try
                    if isTwoway
                        %
                        % Call 'results' to obtain a boolean indicating whether the request completed successfully
                        % or returned a user exception, and the encoded out parameters. The function can also raise
                        % a local exception if necessary.
                        %
                        % Avoid the string concatenation
                        %
                        % res = f.iceCallWithResult('results');
                        %
                        res = IceInternal.Util.callWithResult('Ice_InvocationFuture_results', f.impl_);
                        is = Ice.InputStream(obj.communicator, obj.encoding, res.params);
                        if ~res.ok
                            obj.iceThrowUserException(is, exceptions{:});
                        end
                        if isempty(unmarshalFunc)
                            is.skipEmptyEncapsulation();
                        else
                            [varargout{1:numOutArgs}] = unmarshalFunc(is);
                        end
                    else
                        %
                        % Check for a local exception.
                        %
                        % Avoid the string concatenation
                        %
                        % f.iceCall('check');
                        %
                        IceInternal.Util.call('Ice_InvocationFuture_check', f.impl_);
                    end
                catch ex
                    ex.throwAsCaller();
                end
            end

            try
                % Replace unconfigured dictionary with empty array.
                if ~isConfigured(ctx)
                    ctx = [];
                end

                if twowayOnly && ~isTwoway
                    throw(Ice.TwowayOnlyException(op));
                end
                if isempty(os)
                    buf = [];
                    size = 0;
                else
                    buf = os.buf.buf;
                    size = os.buf.size;
                end
                futPtr = libpointer('voidPtr'); % Output param
                IceInternal.Util.call('Ice_ObjectPrx_ice_invokeAsync', obj.impl_, op, mode, buf, size, ctx, futPtr);
                assert(~isNull(futPtr));
                fut = Ice.Future(futPtr, op, numOutArgs, 'Ice_InvocationFuture', @fetch);
            catch ex
                ex.throwAsCaller();
            end
        end

        function iceThrowUserException(~, is, varargin) % varargs are user exception type names
            try
                is.startEncapsulation();
                is.throwException();
            catch ex
                if isa(ex, 'Ice.UserException')
                    is.endEncapsulation();
                    for i = 1:length(varargin)
                        if isa(ex, varargin{i})
                            ex.throwAsCaller();
                        end
                    end
                    uue = Ice.UnknownUserException(ex.ice_id());
                    uue.throwAsCaller();
                else
                    rethrow(ex);
                end
            end
        end
    end

    methods(Static)
        function r = ice_staticId()
            r = '::Ice::Object';
        end

        function r = checkedCast(p, varargin)
            %CHECKEDCAST Contacts the remote server to check if the target object implements the pseudo-Slice interface
            %   Ice::Object.
            %
            %   Input Arguments
            %     p - The proxy to check.
            %       Ice.ObjectPrx scalar | empty array of Ice.ObjectPrx
            %     facet - The desired facet (optional).
            %       character vector
            %     context - The request context (optional).
            %       dictionary(string, string) scalar
            %
            %   Output Arguments
            %     r - An Ice.ObjectPrx scalar if the target object implements Slice interface
            %       ::Ice::Object; otherwise, an empty array of Ice.ObjectPrx.
            %
            arguments
                p Ice.ObjectPrx {mustBeScalarOrEmpty}
            end
            arguments (Repeating)
                varargin % facet or context or both, so can't specify types
            end
            r = Ice.ObjectPrx.iceCheckedCast(p, Ice.ObjectPrx.ice_staticId(), 'Ice.ObjectPrx', varargin{:});
        end

        function r = uncheckedCast(p, varargin)
            %UNCHECKEDCAST Creates an Ice.ObjectPrx from another proxy without any validation.
            %
            %   Input Arguments
            %     p - The source proxy.
            %       Ice.ObjectPrx scalar | empty array of Ice.ObjectPrx
            %     facet - The desired facet (optional).
            %       character vector
            %
            %   Output Arguments
            %     r - A new Ice.ObjectPrx scalar, or an empty array when p is an empty array.
            %
            arguments
                p Ice.ObjectPrx {mustBeScalarOrEmpty}
            end
            arguments (Repeating)
                varargin (1, :) char
            end
            r = Ice.ObjectPrx.iceUncheckedCast(p, 'Ice.ObjectPrx', varargin{:});
        end
    end

    methods(Static,Access=protected)
        function r = iceCheckedCast(p, id, cls, varargin)
            if ~isempty(p)
                context = dictionary; % unconfigured dictionary
                if isscalar(varargin)
                    if isa(varargin{1}, 'dictionary')
                        context = varargin{1};
                    elseif isempty(varargin{1}) || isa(varargin{1}, 'char')
                       p = p.ice_facet(varargin{1});
                    else
                        error('Ice:ArgumentException', 'Expecting char or context dictionary');
                    end
                elseif length(varargin) == 2
                    if isa(varargin{1}, 'char')
                        p = p.ice_facet(varargin{1});
                    else
                        error('Ice:ArgumentException', 'Expecting char array for facet');
                    end
                    context = varargin{2};
                elseif length(varargin) > 2
                    error('Ice:ArgumentException', 'Too many arguments to checkedCast');
                end

                if p.ice_isA(id, context)
                    constructor = str2func(cls);
                    r = constructor(p.communicator, '',  p.clone_(), p.encoding);
                else
                    emptyFunc = str2func(strcat(cls, '.empty'));
                    r = emptyFunc();
                end
            else
                emptyFunc = str2func(strcat(cls, '.empty'));
                r = emptyFunc();
            end
        end

        function r = iceUncheckedCast(p, cls, varargin)
            if ~isempty(p)
                if isscalar(varargin)
                    p = p.ice_facet(varargin{1});
                elseif length(varargin) > 1
                    error('Ice:ArgumentException', 'Too many arguments to uncheckedCast');
                end
                constructor = str2func(cls);
                r = constructor(p.communicator, '', p.clone_(), p.encoding);
            else
                emptyFunc = str2func(strcat(cls, '.empty'));
                r = emptyFunc();
            end
        end
    end

    methods(Access=private)
        function r = factory_(obj, op, keepType, varargin)
            %
            % Call a C++ proxy factory function. The function returns nil if the call results in no change to the
            % proxy, in which case we can return the current object.
            %

            newImpl = libpointer('voidPtr');
            obj.iceCall(op, newImpl, varargin{:});
            if isNull(newImpl)
                r = obj;
            elseif keepType
                r = obj.newInstance_(newImpl); % Retain the proxy's current type.
            else
                %
                % We don't retain the proxy's existing type for a couple of factory functions.
                %
                r = Ice.ObjectPrx(obj.communicator, '', newImpl, obj.encoding); % encoding doesn't change
            end
        end

        function r = newInstance_(obj, impl)
            %
            % Return a new instance of this proxy type.
            %
            constructor = str2func(class(obj)); % Obtain the constructor for this class
            r = constructor(obj.communicator, '', impl, obj.encoding); % Call the constructor
        end

        function r = clone_(obj)
            %
            % Clone the C++ proxy for use by a new instance of ObjectPrx.
            %
            implPtr = libpointer('voidPtr'); % Output param
            obj.iceCall('clone', implPtr);
            r = implPtr;
        end
    end

    properties(Access=private)
        communicator % The communicator wrapper
        encoding
        isTwoway
        cachedInputStream % Only used for synchronous invocations
    end
end
