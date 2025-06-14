classdef ObjectPrx < IceInternal.WrapperObject
    % ObjectPrx   Summary of ObjectPrx
    %
    % Base interface of all object proxies.
    %
    % ObjectPrx Methods:
    %   ice_toString - Returns a stringified version of this proxy.
    %   ice_getCommunicator - Returns the communicator that created this
    %     proxy.
    %   ice_ping - Tests whether the target object of this proxy can
    %     be reached.
    %   ice_pingAsync - Tests whether the target object of this proxy can
    %     be reached.
    %   ice_isA - Tests whether this object supports a specific
    %     Slice interface.
    %   ice_isAAsync - Tests whether this object supports a specific
    %     Slice interface.
    %   ice_id - Returns the Slice type ID of the most-derived interface
    %     supported by the target object of this proxy.
    %   ice_idAsync - Returns the Slice type ID of the most-derived
    %     interface supported by the target object of this proxy.
    %   ice_ids - Returns the Slice type IDs of the interfaces supported
    %     by the target object of this proxy.
    %   ice_idsAsync - Returns the Slice type IDs of the interfaces
    %     supported by the target object of this proxy.
    %   ice_getIdentity - Returns the identity embedded in this proxy.
    %   ice_identity - Returns a proxy that is identical to this proxy,
    %     except for the identity.
    %   ice_getContext - Returns the per-proxy context for this proxy.
    %   ice_context - Returns a proxy that is identical to this proxy,
    %     except for the per-proxy context.
    %   ice_getFacet - Returns the facet for this proxy.
    %   ice_facet - Returns a proxy that is identical to this proxy,
    %     except for the facet.
    %   ice_getAdapter - Returns the adapter ID for this proxy.
    %   ice_adapterId - Returns a proxy that is identical to this proxy,
    %     except for the adapter ID.
    %   ice_getEndpoints - Returns the endpoints used by this proxy.
    %   ice_endpoints - Returns a proxy that is identical to this proxy,
    %     except for the endpoints.
    %   ice_getLocatorCacheTimeout - Returns the locator cache timeout
    %     of this proxy.
    %   ice_locatorCacheTimeout - Returns a proxy that is identical to
    %     this proxy, except for the locator cache timeout.
    %   ice_getInvocationTimeout - Returns the invocation timeout of
    %     this proxy.
    %   ice_invocationTimeout - Returns a proxy that is identical to
    %     this proxy, except for the invocation timeout.
    %   ice_getConnectionId - Returns the connection id of this proxy.
    %   ice_connectionId - Returns a proxy that is identical to this
    %     proxy, except for its connection ID.
    %   ice_isConnectionCached - Returns whether this proxy caches
    %     connections.
    %   ice_connectionCached - Returns a proxy that is identical to this
    %     proxy, except for connection caching.
    %   ice_getEndpointSelection - Returns how this proxy selects
    %     endpoints (randomly or ordered).
    %   ice_endpointSelection - Returns a proxy that is identical to
    %     this proxy, except for the endpoint selection policy.
    %   ice_getEncodingVersion - Returns the encoding version used to
    %     marshal requests parameters.
    %   ice_encodingVersion - Returns a proxy that is identical to this
    %     proxy, except for the encoding used to marshal parameters.
    %   ice_getRouter - Returns the router for this proxy.
    %   ice_router - Returns a proxy that is identical to this proxy,
    %     except for the router.
    %   ice_getLocator - Returns the locator for this proxy.
    %   ice_locator - Returns a proxy that is identical to this proxy,
    %     except for the locator.
    %   ice_isSecure - Returns whether this proxy uses only secure
    %     endpoints.
    %   ice_secure - Returns a proxy that is identical to this proxy,
    %     except for how it selects endpoints.
    %   ice_isPreferSecure - Returns whether this proxy prefers secure
    %     endpoints.
    %   ice_preferSecure - Returns a proxy that is identical to this
    %     proxy, except for its endpoint selection policy.
    %   ice_isTwoway - Returns whether this proxy uses twoway invocations.
    %   ice_twoway - Returns a proxy that is identical to this proxy,
    %     but uses twoway invocations.
    %   ice_isOneway - Returns whether this proxy uses oneway invocations.
    %   ice_oneway - Returns a proxy that is identical to this proxy,
    %     but uses oneway invocations.
    %   ice_isBatchOneway - Returns whether this proxy uses batch oneway
    %     invocations.
    %   ice_batchOneway - Returns a proxy that is identical to this
    %     proxy, but uses batch oneway invocations.
    %   ice_isDatagram - Returns whether this proxy uses datagram
    %     invocations.
    %   ice_datagram - Returns a proxy that is identical to this proxy,
    %     but uses datagram invocations.
    %   ice_isBatchDatagram - Returns whether this proxy uses batch
    %     datagram invocations.
    %   ice_batchDatagram - Returns a proxy that is identical to this
    %     proxy, but uses batch datagram invocations.
    %   ice_compress - Returns a proxy that is identical to this proxy,
    %     except for compression.
    %   ice_getCompress - Obtains the compression override setting of this proxy.
    %   ice_fixed - Obtains a proxy that is identical to this proxy, except it's
    %     a fixed proxy bound to the given connection.
    %   ice_isFixed - Returns whether this proxy is a fixed proxy.
    %   ice_getConnection - Returns the Connection for this proxy.
    %   ice_getConnectionAsync - Returns the Connection for this proxy.
    %   ice_getCachedConnection - Returns the cached Connection for this
    %     proxy.
    %   ice_flushBatchRequests - Flushes any pending batched requests for
    %     this communicator.
    %   ice_flushBatchRequestsAsync - Flushes any pending batched
    %     requests for this communicator.

    % Copyright (c) ZeroC, Inc.

    methods
        function obj = ObjectPrx(communicator, proxyString, impl, encoding)
            if nargin == 0 % default constructor, typically called with multiple inheritance
                superArgs = {};
            elseif nargin == 2
                impl = libpointer('voidPtr');
                communicator.iceCall('stringToProxy', proxyString, impl);
                assert(~isNull(impl), 'Invalid proxy string');
                encoding = [];
                superArgs = {impl, 'Ice_ObjectPrx'};
            else
                assert(nargin == 3 || nargin == 4, 'Invalid number of arguments');
                assert(isempty(proxyString), 'proxyString must be empty');
                assert(~isempty(impl), 'impl must be non-empty');
                if nargin == 3
                    encoding = [];
                end
                superArgs = {impl, 'Ice_ObjectPrx'};
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

        %
        % Override == operator.
        %
        function r = eq(obj, other)
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
            % ice_toString - Returns a stringified version of this proxy.
            %
            % Returns (char) - A stringified proxy.

            r = obj.iceCallWithResult('ice_toString');
        end

        function r = ice_getCommunicator(obj)
            % ice_getCommunicator - Returns the communicator that created this
            %   proxy.
            %
            % Returns (Ice.Communicator) - The communicator that created this
            %   proxy.

            r = obj.communicator;
        end

        function ice_ping(obj, varargin)
            % ice_ping - Tests whether the target object of this proxy can
            %   be reached.
            %
            % Parameters:
            %   context - Optional context map for the invocation.

            obj.iceInvoke('ice_ping', 2, false, [], false, {}, varargin{:});
        end

        function r = ice_pingAsync(obj, varargin)
            % ice_pingAsync - Tests whether the target object of this proxy can
            %   be reached.
            %
            % Parameters:
            %   context - Optional context map for the invocation.
            %
            % Returns (Ice.Future) - A future that will be completed when the
            %   invocation completes.

            r = obj.iceInvokeAsync('ice_ping', 2, false, [], 0, [], {}, varargin{:});
        end

        function r = ice_isA(obj, id, varargin)
            % ice_isA - Tests whether this object supports a specific
            %   Slice interface.
            %
            % Parameters:
            %   id - The type ID of the Slice interface to test against.
            %   context - Optional context map for the invocation.
            %
            % Returns (logical) - True if the target object has the interface
            %   specified by id or derives from the interface specified by id.

            os = obj.iceStartWriteParams([]);
            os.writeString(id);
            obj.iceEndWriteParams(os);
            is = obj.iceInvoke('ice_isA', 2, true, os, true, {}, varargin{:});
            is.startEncapsulation();
            r = is.readBool();
            is.endEncapsulation();
        end

        function r = ice_isAAsync(obj, id, varargin)
            % ice_isAAsync - Tests whether this object supports a specific
            %   Slice interface.
            %
            % Parameters:
            %   id - The type ID of the Slice interface to test against.
            %   context - Optional context map for the invocation.
            %
            % Returns (Ice.Future) - A future that will be completed when the
            %   invocation completes.

            os = obj.iceStartWriteParams([]);
            os.writeString(id);
            obj.iceEndWriteParams(os);
            function varargout = unmarshal(is)
                is.startEncapsulation();
                varargout{1} = is.readBool();
                is.endEncapsulation();
            end
            r = obj.iceInvokeAsync('ice_isA', 2, true, os, 1, @unmarshal, {}, varargin{:});
        end

        function r = ice_id(obj, varargin)
            % ice_id - Returns the Slice type ID of the most-derived interface
            %   supported by the target object of this proxy.
            %
            % Parameters:
            %   context - Optional context map for the invocation.
            %
            % Returns (char) - The Slice type ID of the most-derived interface.

            is = obj.iceInvoke('ice_id', 2, true, [], true, {}, varargin{:});
            is.startEncapsulation();
            r = is.readString();
            is.endEncapsulation();
        end

        function r = ice_idAsync(obj, varargin)
            % ice_idAsync - Returns the Slice type ID of the most-derived
            %   interface supported by the target object of this proxy.
            %
            % Parameters:
            %   context - Optional context map for the invocation.
            %
            % Returns (Ice.Future) - A future that will be completed when the
            %   invocation completes.

            function varargout = unmarshal(is)
                is.startEncapsulation();
                varargout{1} = is.readString();
                is.endEncapsulation();
            end
            r = obj.iceInvokeAsync('ice_id', 2, true, [], 1, @unmarshal, {}, varargin{:});
        end

        function r = ice_ids(obj, varargin)
            % ice_ids - Returns the Slice type IDs of the interfaces supported
            %   by the target object of this proxy.
            %
            % Parameters:
            %   context - Optional context map for the invocation.
            %
            % Returns (cell array of char) - The Slice type IDs of the
            %   interfaces supported by the target object, in alphabetical order.

            is = obj.iceInvoke('ice_ids', 2, true, [], true, {}, varargin{:});
            is.startEncapsulation();
            r = is.readStringSeq();
            is.endEncapsulation();
        end

        function r = ice_idsAsync(obj, varargin)
            % ice_idsAsync - Returns the Slice type IDs of the interfaces
            %   supported by the target object of this proxy.
            %
            % Parameters:
            %   context - Optional context map for the invocation.
            %
            % Returns (Ice.Future) - A future that will be completed when the
            %   invocation completes.

            function varargout = unmarshal(is)
                is.startEncapsulation();
                varargout{1} = is.readStringSeq();
                is.endEncapsulation();
            end
            r = obj.iceInvokeAsync('ice_ids', 2, true, [], 1, @unmarshal, {}, varargin{:});
        end

        function r = ice_getIdentity(obj)
            % ice_getIdentity - Returns the identity embedded in this proxy.
            %
            % Returns (Ice.Identity) - The identity of the target object.

            r = obj.iceCallWithResult('ice_getIdentity');
        end

        function r = ice_identity(obj, id)
            % ice_identity - Returns a proxy that is identical to this proxy,
            %   except for the identity.
            %
            % Parameters:
            %   id (Ice.Identity) - The identity for the new proxy.
            %
            % Returns (Ice.ObjectPrx) - The proxy with the new identity.

            r = obj.factory_('ice_identity', false, id);
        end

        function r = ice_getContext(obj)
            % ice_getContext - Returns the per-proxy context for this proxy.
            %
            % Returns (dictionary) - The per-proxy context. If the proxy
            % does not have a per-proxy (implicit) context, the return value
            % is an empty array.

            r = obj.iceCallWithResult('ice_getContext');
        end

        function r = ice_context(obj, ctx)
            % ice_context - Returns a proxy that is identical to this proxy,
            %   except for the per-proxy context.
            %
            % Parameters:
            %   ctx (dictionary) - The context for the new proxy.
            %
            % Returns - The proxy with the new per-proxy context.

            r = obj.factory_('ice_context', true, ctx);
        end

        function r = ice_getFacet(obj)
            % ice_getFacet - Returns the facet for this proxy.
            %
            % Returns (char) - The facet for this proxy. If the proxy uses the
            %   default facet, the return value is the empty string.

            r = obj.iceCallWithResult('ice_getFacet');
        end

        function r = ice_facet(obj, f)
            % ice_facet - Returns a proxy that is identical to this proxy,
            %   except for the facet.
            %
            % Parameters:
            %   f (char) - The facet for the new proxy.
            %
            % Returns (Ice.ObjectPrx) - The proxy with the new facet.

            r = obj.factory_('ice_facet', false, f);
        end

        function r = ice_getAdapterId(obj)
            % ice_getAdapter - Returns the adapter ID for this proxy.
            %
            % Returns (char) - The adapter ID. If the proxy does not have an
            %   adapter ID, the return value is the empty string.

            r = obj.iceCallWithResult('ice_getAdapterId');
        end

        function r = ice_adapterId(obj, id)
            % ice_adapterId - Returns a proxy that is identical to this proxy,
            %   except for the adapter ID.
            %
            % Parameters:
            %   id (char) - The adapter ID for the new proxy.
            %
            % Returns - The proxy with the new adapter ID.

            r = obj.factory_('ice_adapterId', true, id);
        end

        function r = ice_getEndpoints(obj)
            % ice_getEndpoints - Returns the endpoints used by this proxy.
            %
            % Returns (cell array of Ice.Endpoint) - The endpoints used by
            %   this proxy.

            num = obj.iceCallWithResult('ice_getNumEndpoints');
            r = cell(1, num);
            for i = 1:num
                impl = libpointer('voidPtr');
                obj.iceCallWithResult('ice_getEndpoint', i - 1, impl); % C-style index
                assert(~isNull(impl));
                r{i} = Ice.Endpoint(impl);
            end
        end

        function r = ice_endpoints(obj, endpts)
            % ice_endpoints - Returns a proxy that is identical to this proxy,
            %   except for the endpoints.
            %
            % Parameters:
            %   endpts (cell array of Ice.Endpoint) - The endpoints for the
            %     new proxy.
            %
            % Returns - The proxy with the new endpoints.

            %
            % It's not clear how we can pass a vector of void* to a C function. So we create a temporary C vector
            % and populate it one element at a time.
            %

            for i = 1:length(endpts)
                if ~isa(endpts{i}, 'Ice.Endpoint')
                    throw(LocalException('Ice:ArgumentException', 'expected an Ice.Endpoint'))
                end
            end
            arr = libpointer('voidPtr');
            obj.iceCall('ice_createEndpointList', length(endpts), arr);
            for i = 1:length(endpts)
                obj.iceCall('ice_setEndpoint', arr, i - 1, endpts{i}.impl_); % C-style index
            end
            r = obj.factory_('ice_endpoints', true, arr); % The C function also destroys the temporary array.
        end

        function r = ice_getLocatorCacheTimeout(obj)
            % ice_getLocatorCacheTimeout - Returns the locator cache timeout
            %   of this proxy.
            %
            % Returns (int32) - The locator cache timeout value (in seconds).

            r = obj.iceCallWithResult('ice_getLocatorCacheTimeout');
        end

        function r = ice_locatorCacheTimeout(obj, t)
            % ice_locatorCacheTimeout - Returns a proxy that is identical to
            %   this proxy, except for the locator cache timeout.
            %
            % Parameters:
            %   t (int32) - The new locator cache timeout (in seconds).
            %
            % Returns - The proxy with the new timeout.

            r = obj.factory_('ice_locatorCacheTimeout', true, t);
        end

        function r = ice_getInvocationTimeout(obj)
            % ice_getInvocationTimeout - Returns the invocation timeout of
            %   this proxy.
            %
            % Returns (int32) - The invocation timeout value (in seconds).

            r = obj.iceCallWithResult('ice_getInvocationTimeout');
        end

        function r = ice_invocationTimeout(obj, t)
            % ice_invocationTimeout - Returns a proxy that is identical to
            %   this proxy, except for the invocation timeout.
            %
            % Parameters:
            %   t (int32) - The new invocation timeout (in seconds).
            %
            % Returns - The proxy with the new timeout.

            r = obj.factory_('ice_invocationTimeout', true, t);
        end

        function r = ice_getConnectionId(obj)
            % ice_getConnectionId - Returns the connection id of this proxy.
            %
            % Returns (char) - The connection id.

            r = obj.iceCallWithResult('ice_getConnectionId');
        end

        function r = ice_connectionId(obj, id)
            % ice_connectionId - Returns a proxy that is identical to this
            %   proxy, except for its connection ID.
            %
            % Parameters:
            %   id (char) - The connection ID for the new proxy. An empty
            %   string removes the connection ID.
            %
            % Returns - A proxy with the specified connection ID.

            r = obj.factory_('ice_connectionId', true, id);
        end

        function r = ice_isConnectionCached(obj)
            % ice_isConnectionCached - Returns whether this proxy caches
            %   connections.
            %
            % Returns (logical) - True if this proxy caches connections;
            %   false otherwise.

            r = obj.iceCallWithResult('ice_isConnectionCached');
        end

        function r = ice_connectionCached(obj, b)
            % ice_connectionCached - Returns a proxy that is identical to this
            %   proxy, except for connection caching.
            %
            % Parameters:
            %   b (logical) - True if the new proxy should cache connections;
            %     false otherwise.
            %
            % Returns - The proxy with the specified caching policy.

            if b
                val = 1;
            else
                val = 0;
            end
            r = obj.factory_('ice_connectionCached', true, val);
        end

        function r = ice_getEndpointSelection(obj)
            % ice_getEndpointSelection - Returns how this proxy selects
            %   endpoints (randomly or ordered).
            %
            % Returns (Ice.EndpointSelectionType) - The endpoint selection
            %   policy.

            r = Ice.EndpointSelectionType.ice_getValue(obj.iceCallWithResult('ice_getEndpointSelection'));
        end

        function r = ice_endpointSelection(obj, t)
            % ice_endpointSelection - Returns a proxy that is identical to
            %   this proxy, except for the endpoint selection policy.
            %
            % Parameters:
            %   t (Ice.EndpointSelectionType) - The new endpoint selection policy.
            %
            % Returns - The proxy with the specified endpoint selection policy.

            r = obj.factory_('ice_endpointSelection', true, t);
        end

        function r = ice_getEncodingVersion(obj)
            % ice_getEncodingVersion - Returns the encoding version used to
            %   marshal requests parameters.
            %
            % Returns (Ice.EncodingVersion) - The encoding version.

            r = obj.encoding;
        end

        function r = ice_encodingVersion(obj, ver)
            % ice_encodingVersion - Returns a proxy that is identical to this
            %   proxy, except for the encoding used to marshal parameters.
            %
            % Parameters:
            %   ver (Ice.EncodingVersion) - The encoding version to use to
            %   marshal request parameters.
            %
            % Returns - The proxy with the specified encoding version.

            r = obj.factory_('ice_encodingVersion', true, ver);
            r.encoding = ver;
        end

        function r = ice_getRouter(obj)
            % ice_getRouter - Returns the router for this proxy.
            %
            % Returns (Ice.RouterPrx) - The router for the proxy. If no router
            %   is configured for the proxy, the return value is an empty
            %   array.

            v = libpointer('voidPtr');
            obj.iceCall('ice_getRouter', v);
            if isNull(v)
                r = [];
            else
                r = Ice.RouterPrx(obj.communicator, '', v);
            end
        end

        function r = ice_router(obj, rtr)
            % ice_router - Returns a proxy that is identical to this proxy,
            %   except for the router.
            %
            % Parameters:
            %   rtr (Ice.RouterPrx) - The router for the new proxy.
            %
            % Returns - The proxy with the specified router.

            if isempty(rtr)
                impl = libpointer('voidPtr');
            else
                impl = rtr.impl_;
            end
            r = obj.factory_('ice_router', true, impl);
        end

        function r = ice_getLocator(obj)
            % ice_getLocator - Returns the locator for this proxy.
            %
            % Returns (Ice.LocatorPrx) - The locator for the proxy. If no
            %   locator is configured for the proxy, the return value is
            %   an empty array.

            v = libpointer('voidPtr');
            obj.iceCall('ice_getLocator', v);
            if isNull(v)
                r = [];
            else
                r = Ice.LocatorPrx(obj.communicator, '', v);
            end
        end

        function r = ice_locator(obj, loc)
            % ice_locator - Returns a proxy that is identical to this proxy,
            %   except for the locator.
            %
            % Parameters:
            %   loc (Ice.LocatorPrx) - The locator for the new proxy.
            %
            % Returns - The proxy with the specified locator.

            if isempty(loc)
                impl = libpointer('voidPtr');
            else
                impl = loc.impl_;
            end
            r = obj.factory_('ice_locator', true, impl);
        end

        function r = ice_isSecure(obj)
            % ice_isSecure - Returns whether this proxy uses only secure
            %   endpoints.
            %
            % Returns (logical) - True if this proxy communicates only via
            %   secure endpoints; false otherwise.

            r = obj.iceCallWithResult('ice_isSecure');
        end

        function r = ice_secure(obj, b)
            % ice_secure - Returns a proxy that is identical to this proxy,
            %   except for how it selects endpoints.
            %
            % Parameters:
            %   b (logical) - If b is true, only endpoints that use a secure
            %     transport are used by the new proxy. If b is false, the
            %     returned proxy uses both secure and insecure endpoints.
            %
            % Returns - The proxy with the specified selection policy.

            if b
                val = 1;
            else
                val = 0;
            end
            r = obj.factory_('ice_secure', true, val);
        end

        function r = ice_isPreferSecure(obj)
            % ice_isPreferSecure - Returns whether this proxy prefers secure
            %   endpoints.
            %
            % Returns (logical) - True if the proxy always attempts to invoke
            %   via secure endpoints before it attempts to use insecure
            %   endpoints; false otherwise.

            r = obj.iceCallWithResult('ice_isPreferSecure');
        end

        function r = ice_preferSecure(obj, b)
            % ice_preferSecure - Returns a proxy that is identical to this
            %   proxy, except for its endpoint selection policy.
            %
            % Parameters:
            %   b (logical) - If b is true, the new proxy will use secure
            %     endpoints for invocations and only use insecure endpoints
            %     if an invocation cannot be made via secure endpoints.
            %     If b is false, the proxy prefers insecure endpoints to
            %     secure ones.
            %
            % Returns - The proxy with the specified selection policy.

            if b
                val = 1;
            else
                val = 0;
            end
            r = obj.factory_('ice_preferSecure', true, val);
        end

        function r = ice_isTwoway(obj)
            % ice_isTwoway - Returns whether this proxy uses twoway invocations.
            %
            % Returns (logical) - True if this proxy uses twoway invocations;
            %   false otherwise.

            r = obj.isTwoway;
        end

        function r = ice_twoway(obj)
            % ice_twoway - Returns a proxy that is identical to this proxy,
            %   but uses twoway invocations.
            %
            % Returns - A proxy that uses twoway invocations.

            r = obj.factory_('ice_twoway', true);
        end

        function r = ice_isOneway(obj)
            % ice_isOneway - Returns whether this proxy uses oneway invocations.
            %
            % Returns (logical) - True if this proxy uses oneway invocations;
            %   false otherwise.

            r = obj.iceCallWithResult('ice_isOneway');
        end

        function r = ice_oneway(obj)
            % ice_oneway - Returns a proxy that is identical to this proxy,
            %   but uses oneway invocations.
            %
            % Returns - A proxy that uses oneway invocations.

            r = obj.factory_('ice_oneway', true);
        end

        function r = ice_isBatchOneway(obj)
            % ice_isBatchOneway - Returns whether this proxy uses batch oneway
            %   invocations.
            %
            % Returns (logical) - True if this proxy uses batch oneway
            %   invocations; false otherwise.

            r = obj.iceCallWithResult('ice_isBatchOneway');
        end

        function r = ice_batchOneway(obj)
            % ice_batchOneway - Returns a proxy that is identical to this
            %   proxy, but uses batch oneway invocations.
            %
            % Returns - A new proxy that uses batch oneway invocations.

            r = obj.factory_('ice_batchOneway', true);
        end

        function r = ice_isDatagram(obj)
            % ice_isDatagram - Returns whether this proxy uses datagram
            %   invocations.
            %
            % Returns (logical) - True if this proxy uses datagram invocations;
            %   false otherwise.

            r = obj.iceCallWithResult('ice_isDatagram');
        end

        function r = ice_datagram(obj)
            % ice_datagram - Returns a proxy that is identical to this proxy,
            %   but uses datagram invocations.
            %
            % Returns - A new proxy that uses datagram invocations.

            r = obj.factory_('ice_datagram', true);
        end

        function r = ice_isBatchDatagram(obj)
            % ice_isBatchDatagram - Returns whether this proxy uses batch
            %   datagram invocations.
            %
            % Returns (logical) - True if this proxy uses batch datagram
            %   invocations; false otherwise.

            r = obj.iceCallWithResult('ice_isBatchDatagram');
        end

        function r = ice_batchDatagram(obj)
            % ice_batchDatagram - Returns a proxy that is identical to this
            %   proxy, but uses batch datagram invocations.
            %
            % Returns - A new proxy that uses batch datagram invocations.

            r = obj.factory_('ice_batchDatagram', true);
        end

        function r = ice_compress(obj, b)
            % ice_compress - Returns a proxy that is identical to this proxy,
            %   except for compression.
            %
            % Parameters:
            %   b (logical) - True enables compression for the new proxy;
            %     false disables compression.
            %
            % Returns - A proxy with the specified compression override setting.

            if b
                val = 1;
            else
                val = 0;
            end
            r = obj.factory_('ice_compress', true, val);
        end

        function r = ice_getCompress(obj)
            % ice_getCompress - Obtains the compression override setting of this proxy.
            %
            % Returns (optional bool) - The compression override setting. If Ice.Unset
            %   is returned, no override is set. Otherwise, true if compression is
            %   enabled, false otherwise.

            opt = obj.iceCallWithResult('ice_getCompress');
            if opt.hasValue
                r = opt.value;
            else
                r = IceInternal.UnsetI.Instance;
            end
        end

        function r = ice_fixed(obj, connection)
            % ice_fixed - Obtains a proxy that is identical to this proxy, except it's
            %   a fixed proxy bound to the given connection.
            %
            % Parameters:
            %   connection (Ice.Connection) - The fixed proxy connection.
            %
            % Returns (Ice.ObjectPrx) - A fixed proxy bound to the given connection.

            if isempty(connection)
                throw(LocalException('Ice:ArgumentException', 'invalid null connection passed to ice_fixed'));
            end

            r = obj.factory_('ice_fixed', true, connection.iceGetImpl());
        end

        function r = ice_isFixed(obj)
            % ice_isFixed - Determines whether this proxy is a fixed proxy.
            %
            % Returns (logical) - True if this proxy is a fixed proxy, false otherwise.

            r = obj.iceCallWithResult('ice_isFixed');
        end

        function r = ice_getConnection(obj)
            % ice_getConnection - Returns the Connection for this proxy. If the
            %   proxy does not yet have an established connection, it first
            %   attempts to create a connection.
            %
            % Returns (Ice.Connection) - The Connection for this proxy.

            v = libpointer('voidPtr');
            obj.iceCall('ice_getConnection', v);
            if isNull(v)
                r = [];
            else
                r = Ice.Connection(v, obj.communicator);
            end
        end

        function r = ice_getConnectionAsync(obj)
            % ice_getConnectionAsync - Returns the Connection for this proxy.
            %   If the proxy does not yet have an established connection, it
            %   first attempts to create a connection.
            %
            % Returns (Ice.Future) - A future that will be completed when the
            %   invocation completes.

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
            % ice_getCachedConnection - Returns the cached Connection for this
            %   proxy. If the proxy does not yet have an established
            %   connection, it does not attempt to create a connection.
            %
            % Returns (Ice.Connection) - The cached Connection for this proxy,
            %   or an empty array if the proxy does not have an established
            %   connection.

            v = libpointer('voidPtr');
            obj.iceCall('ice_getCachedConnection', v);
            if isNull(v)
                r = [];
            else
                r = Ice.Connection(v, obj.communicator);
            end
        end

        function ice_flushBatchRequests(obj)
            % ice_flushBatchRequests - Flushes any pending batched requests for
            %   this communicator. The call blocks until the flush is complete.

            obj.iceCall('ice_flushBatchRequests');
        end

        function r = ice_flushBatchRequestsAsync(obj)
            % ice_flushBatchRequestsAsync - Flushes asynchronously any pending batched
            %   requests for this communicator.
            %
            % Returns (Ice.Future) - A future that will be completed when the
            %   invocation completes.

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

        function is = iceInvoke(obj, op, mode, twowayOnly, os, hasOutParams, exceptions, varargin)
            try
                % Vararg accepted for optional context argument.
                if length(varargin) > 1
                    throw(LocalException('Ice:ArgumentException', 'one optional argument is allowed for request context'))
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

                if isscalar(varargin)
                    %
                    % Avoid the string concatenation
                    %
                    % res = obj.iceCallWithResult('ice_invoke', op, mode, buf, size, varargin{1});
                    %
                    res = IceInternal.Util.callWithResult('Ice_ObjectPrx_ice_invoke', obj.impl_, op, mode, buf, ...
                                                          size, varargin{1});
                else
                    %
                    % Avoid the string concatenation
                    %
                    % res = obj.iceCallWithResult('ice_invokeNC', op, mode, buf, size);
                    %
                    res = IceInternal.Util.callWithResult('Ice_ObjectPrx_ice_invokeNC', obj.impl_, op, mode, buf, size);
                end

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

        function fut = iceInvokeAsync(obj, op, mode, twowayOnly, os, numOutArgs, unmarshalFunc, exceptions, varargin)
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
                % Vararg accepted for optional context argument.
                if length(varargin) > 1
                    throw(LocalException('Ice:ArgumentException', 'one optional argument is allowed for request context'))
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
                if isscalar(varargin)
                    %
                    % Avoid the string concatenation
                    %
                    % obj.iceCall('ice_invokeAsync', op, mode, buf, size, varargin{1}, futPtr);
                    %
                    IceInternal.Util.call('Ice_ObjectPrx_ice_invokeAsync', obj.impl_, op, mode, buf, size, ...
                                          varargin{1}, futPtr);
                else
                    %
                    % Avoid the string concatenation
                    %
                    % obj.iceCall('ice_invokeAsyncNC', op, mode, buf, size, futPtr);
                    %
                    IceInternal.Util.call('Ice_ObjectPrx_ice_invokeAsyncNC', obj.impl_, op, mode, buf, size, futPtr);
                end
                assert(~isNull(futPtr));
                fut = Ice.Future(futPtr, op, numOutArgs, 'Ice_InvocationFuture', @fetch);
            catch ex
                ex.throwAsCaller();
            end
        end

        function iceThrowUserException(~, is, varargin) % Varargs are user exception type names
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
            if isempty(varargin)
                r = p;
            else
                r = Ice.ObjectPrx.iceCheckedCast(p, Ice.ObjectPrx.ice_staticId(), 'Ice.ObjectPrx', varargin{:});
            end
        end

        function r = uncheckedCast(p, varargin)
            if isempty(varargin)
                r = p;
            elseif isscalar(varargin)
                if ~isempty(p)
                    r = p.ice_facet(varargin{1});
                else
                    r = p;
                end
            else
                throw(LocalException('Ice:ArgumentException', 'too many arguments to uncheckedCast'));
            end
        end
    end

    methods(Static,Access=protected)
        function r = iceCheckedCast(p, id, cls, varargin)
            hasFacet = false;
            facet = [];
            context = {};
            if isscalar(varargin)
                if isa(varargin{1}, 'dictionary')
                    context = varargin(1);
                elseif isempty(varargin{1}) || isa(varargin{1}, 'char')
                    hasFacet = true;
                    facet = varargin{1};
                else
                    throw(LocalException('Ice:ArgumentException', 'expecting string or context dictionary'));
                end
            elseif length(varargin) == 2
                hasFacet = true;
                facet = varargin{1};
                context = varargin(2);
            elseif length(varargin) > 2
                throw(LocalException('Ice:ArgumentException', 'too many arguments to checkedCast'));
            end
            if ~isempty(p)
                if hasFacet
                    p = p.ice_facet(facet);
                end
                if p.ice_isA(id, context{:})
                    constructor = str2func(cls);
                    r = constructor(p.communicator, '',  p.clone_(), p.encoding);
                else
                    r = [];
                end
            else
                r = p;
            end
        end

        function r = iceUncheckedCast(p, cls, varargin)
            hasFacet = false;
            facet = [];
            if isscalar(varargin)
                hasFacet = true;
                facet = varargin{1};
            elseif length(varargin) > 1
                throw(LocalException('Ice:ArgumentException', 'too many arguments to uncheckedCast'));
            end
            if ~isempty(p)
                if hasFacet
                    p = p.ice_facet(facet);
                end
                if isa(p, cls)
                    r = p;
                else
                    constructor = str2func(cls);
                    r = constructor(p.communicator, '', p.clone_(), p.encoding);
                end
            else
                r = p;
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
