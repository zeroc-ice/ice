%{
**********************************************************************

Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.

This copy of Ice is licensed to you under the terms described in the
ICE_LICENSE file included in this distribution.

**********************************************************************
%}

classdef ObjectPrx < IceInternal.WrapperObject
    methods
        function obj = ObjectPrx(communicator, encoding, impl, bytes)
            obj = obj@IceInternal.WrapperObject(impl, 'Ice_ObjectPrx');
            obj.communicator = communicator;
            obj.encoding = encoding;
            if nargin == 4
                obj.bytes = bytes;
            end

            if ~isempty(impl)
                obj.isTwoway = obj.callWithResult_('ice_isTwoway');
            end
        end

        function delete(obj)
            if ~isempty(obj.impl_)
                obj.call_('_release');
                obj.impl_ = [];
            end
        end

        %
        % Override == operator.
        %
        function r = eq(obj, other)
            if isempty(other) || ~isa(other, 'Ice.ObjectPrx')
                r = false;
            elseif ~isempty(obj.bytes) && ~isempty(other.bytes)
                %
                % Compare the marshaled forms of the two proxies.
                %
                r = isequal(obj.bytes, other.bytes);
            else
                %
                % Call into C++ to compare the two proxies.
                %
                obj.instantiate_();
                other.instantiate_();
                v = libpointer('uint8Ptr', 0);
                obj.call_('equals', other.impl_, v);
                r = v.Value == 1;
            end
        end

        function r = ice_createOutputStream(obj)
            r = Ice.OutputStream(obj.communicator, obj.encoding);
        end

        function r = ice_toString(obj)
            obj.instantiate_();
            r = obj.callWithResult_('ice_toString');
        end

        function r = ice_getCommunicator(obj)
            r = obj.communicator;
        end

        function ice_ping(obj, varargin)
            obj.invoke_('ice_ping', 1, false, [], false, {}, varargin{:});
        end

        function r = ice_pingAsync(obj, varargin)
            r = obj.invokeAsync_('ice_ping', 1, false, [], 0, [], {}, varargin{:});
        end

        function r = ice_isA(obj, id, varargin)
            os = obj.startWriteParams_([]);
            os.writeString(id);
            obj.endWriteParams_(os);
            is = obj.invoke_('ice_isA', 1, true, os, true, {}, varargin{:});
            is.startEncapsulation();
            r = is.readBool();
            is.endEncapsulation();
        end

        function r = ice_isAAsync(obj, id, varargin)
            os = obj.startWriteParams_([]);
            os.writeString(id);
            obj.endWriteParams_(os);
            function varargout = unmarshal(is)
                is.startEncapsulation();
                varargout{1} = is.readBool();
                is.endEncapsulation();
            end
            r = obj.invokeAsync_('ice_isA', 1, true, os, 1, @unmarshal, {}, varargin{:});
        end

        function r = ice_id(obj, varargin)
            is = obj.invoke_('ice_id', 1, true, [], true, {}, varargin{:});
            is.startEncapsulation();
            r = is.readString();
            is.endEncapsulation();
        end

        function r = ice_idAsync(obj, varargin)
            function varargout = unmarshal(is)
                is.startEncapsulation();
                varargout{1} = is.readString();
                is.endEncapsulation();
            end
            r = obj.invokeAsync_('ice_id', 1, true, [], 1, @unmarshal, {}, varargin{:});
        end

        function r = ice_ids(obj, varargin)
            is = obj.invoke_('ice_ids', 1, true, [], true, {}, varargin{:});
            is.startEncapsulation();
            r = is.readStringSeq();
            is.endEncapsulation();
        end

        function r = ice_idsAsync(obj, varargin)
            function varargout = unmarshal(is)
                is.startEncapsulation();
                varargout{1} = is.readStringSeq();
                is.endEncapsulation();
            end
            r = obj.invokeAsync_('ice_ids', 1, true, [], 1, @unmarshal, {}, varargin{:});
        end

        function r = ice_getIdentity(obj)
            obj.instantiate_();
            r = obj.callWithResult_('ice_getIdentity');
        end

        function r = ice_identity(obj, id)
            r = obj.factory_('ice_identity', false, id);
        end

        function r = ice_getContext(obj)
            obj.instantiate_();
            r = obj.callWithResult_('ice_getContext');
        end

        function r = ice_context(obj, ctx)
            r = obj.factory_('ice_context', true, ctx);
        end

        function r = ice_getFacet(obj)
            obj.instantiate_();
            r = obj.callWithResult_('ice_getFacet');
        end

        function r = ice_facet(obj, f)
            r = obj.factory_('ice_facet', false, f);
        end

        function r = ice_getAdapterId(obj)
            obj.instantiate_();
            r = obj.callWithResult_('ice_getAdapterId');
        end

        function r = ice_adapterId(obj, id)
            r = obj.factory_('ice_adapterId', true, id);
        end

        function r = ice_getEndpoints(obj)
            obj.instantiate_();
            num = obj.callWithResult_('ice_getNumEndpoints');
            r = {};
            for i = 1:num
                impl = libpointer('voidPtr');
                e = obj.callWithResult_('ice_getEndpoint', i - 1, impl); % C-style index
                assert(~isNull(impl));
                r{i} = Ice.Endpoint(impl);
            end
        end

        function r = ice_endpoints(obj, endpts)
            %
            % It's not clear how we can pass a vector of void* to a C function. So we create a temporary C vector
            % and populate it one element at a time.
            %

            for i = 1:length(endpts)
                if ~isa(endpts{i}, 'Ice.Endpoint')
                    throw(MException('Ice:ArgumentException', 'expected an Ice.Endpoint'))
                end
            end
            arr = libpointer('voidPtr');
            obj.call_('ice_createEndpointList', length(endpts), arr);
            for i = 1:length(endpts)
                obj.call_('ice_setEndpoint', arr, i - 1, endpts{i}.impl_); % C-style index
            end
            r = obj.factory_('ice_endpoints', true, arr); % The C function also destroys the temporary array.
        end

        function r = ice_getLocatorCacheTimeout(obj)
            obj.instantiate_();
            r = obj.callWithResult_('ice_getLocatorCacheTimeout');
        end

        function r = ice_locatorCacheTimeout(obj, t)
            r = obj.factory_('ice_locatorCacheTimeout', true, t);
        end

        function r = ice_getInvocationTimeout(obj)
            obj.instantiate_();
            r = obj.callWithResult_('ice_getInvocationTimeout');
        end

        function r = ice_invocationTimeout(obj, t)
            r = obj.factory_('ice_invocationTimeout', true, t);
        end

        function r = ice_getConnectionId(obj)
            obj.instantiate_();
            r = obj.callWithResult_('ice_getConnectionId');
        end

        function r = ice_connectionId(obj, id)
            r = obj.factory_('ice_connectionId', true, id);
        end

        function r = ice_isConnectionCached(obj)
            r = obj.callWithResult_('ice_isConnectionCached');
        end

        function r = ice_connectionCached(obj, b)
            if b
                val = 1;
            else
                val = 0;
            end
            r = obj.factory_('ice_connectionCached', true, val);
        end

        function r = ice_getEndpointSelection(obj)
            obj.instantiate_();
            r = obj.callWithResult_('ice_getEndpointSelection');
        end

        function r = ice_endpointSelection(obj, t)
            r = obj.factory_('ice_endpointSelection', true, t);
        end

        function r = ice_getEncodingVersion(obj)
            r = obj.encoding;
        end

        function r = ice_encodingVersion(obj, ver)
            r = obj.factory_('ice_encodingVersion', true, ver);
            r.encoding = ver;
        end

        function r = ice_getRouter(obj)
            obj.instantiate_();
            v = libpointer('voidPtr');
            obj.call_('ice_getRouter', v);
            if isNull(v)
                r = [];
            else
                r = Ice.RouterPrx(obj.communicator, obj.encoding, v, []);
            end
        end

        function r = ice_router(obj, rtr)
            if isempty(rtr)
                impl = libpointer('voidPtr');
            else
                impl = rtr.impl_;
            end
            r = obj.factory_('ice_router', true, impl);
        end

        function r = ice_getLocator(obj)
            obj.instantiate_();
            v = libpointer('voidPtr');
            obj.call_('ice_getLocator', v);
            if isNull(v)
                r = [];
            else
                r = Ice.LocatorPrx(obj.communicator, obj.encoding, v, []);
            end
        end

        function r = ice_locator(obj, loc)
            if isempty(loc)
                impl = libpointer('voidPtr');
            else
                impl = loc.impl_;
            end
            r = obj.factory_('ice_locator', true, impl);
        end

        function r = ice_isSecure(obj)
            r = obj.callWithResult_('ice_isSecure');
        end

        function r = ice_secure(obj, b)
            if b
                val = 1;
            else
                val = 0;
            end
            r = obj.factory_('ice_secure', true, val);
        end

        function r = ice_isPreferSecure(obj)
            r = obj.callWithResult_('ice_isPreferSecure');
        end

        function r = ice_preferSecure(obj, b)
            if b
                val = 1;
            else
                val = 0;
            end
            r = obj.factory_('ice_preferSecure', true, val);
        end

        function r = ice_isTwoway(obj)
            r = obj.isTwoway;
        end

        function r = ice_twoway(obj)
            r = obj.factory_('ice_twoway', true);
        end

        function r = ice_isOneway(obj)
            r = obj.callWithResult_('ice_isOneway');
        end

        function r = ice_oneway(obj)
            r = obj.factory_('ice_oneway', true);
        end

        function r = ice_isBatchOneway(obj)
            r = obj.callWithResult_('ice_isBatchOneway');
        end

        function r = ice_batchOneway(obj)
            r = obj.factory_('ice_batchOneway', true);
        end

        function r = ice_isDatagram(obj)
            r = obj.callWithResult_('ice_isDatagram');
        end

        function r = ice_datagram(obj)
            r = obj.factory_('ice_datagram', true);
        end

        function r = ice_isBatchDatagram(obj)
            r = obj.callWithResult_('ice_isBatchDatagram');
        end

        function r = ice_batchDatagram(obj)
            r = obj.factory_('ice_batchDatagram', true);
        end

        function r = ice_compress(obj, b)
            if b
                val = 1;
            else
                val = 0;
            end
            r = obj.factory_('ice_compress', true, val);
        end

        function r = ice_timeout(obj, t)
            r = obj.factory_('ice_timeout', true, t);
        end

        function r = ice_getConnection(obj)
            obj.instantiate_();
            v = libpointer('voidPtr');
            obj.call_('ice_getConnection', v);
            if isNull(v)
                r = [];
            else
                r = Ice.Connection(v, obj.communicator);
            end
        end

        function r = ice_getConnectionAsync(obj)
            obj.instantiate_();
            future = libpointer('voidPtr');
            obj.call_('ice_getConnectionAsync', future);
            assert(~isNull(future));
            function varargout = fetch(f)
                con = libpointer('voidPtr', 0); % Output param
                f.call_('fetch', con);
                assert(~isNull(con));
                varargout{1} = Ice.Connection(con);
            end
            r = Ice.Future(future, 'ice_getConnection', 1, 'Ice_GetConnectionFuture', @fetch);
        end

        function r = ice_getCachedConnection(obj)
            obj.instantiate_();
            v = libpointer('voidPtr');
            obj.call_('ice_getCachedConnection', v);
            if isNull(v)
                r = [];
            else
                r = Ice.Connection(v, obj.communicator);
            end
        end

        function ice_flushBatchRequests(obj)
            obj.instantiate_();
            obj.call_('ice_flushBatchRequests');
        end

        function r = ice_flushBatchRequestsAsync(obj)
            obj.instantiate_();
            future = libpointer('voidPtr');
            obj.call_('ice_flushBatchRequestsAsync', future);
            assert(~isNull(future));
            r = Ice.Future(future, 'ice_flushBatchRequests', 0, 'Ice_SimpleFuture', @(fut) fut.call_('check'));
        end
    end

    methods(Hidden=true)
        function write_(obj, os, encoding)
            %
            % If we don't yet have a byte buffer representing the marshaled form of the proxy, then call into
            % C++ to marshal the proxy and then cache the bytes.
            %
            if isempty(obj.bytes)
                obj.bytes = obj.callWithResult_('write', obj.communicator.impl_, encoding);
            end
            os.writeBlob(obj.bytes);
        end
        function r = getImpl_(obj)
            obj.instantiate_();
            r = obj.impl_;
        end
    end

    methods(Access=protected)
        function os = startWriteParams_(obj, format)
            os = obj.ice_createOutputStream();
            os.startEncapsulation(format);
        end

        function endWriteParams_(obj, os)
            os.endEncapsulation();
        end

        function is = invoke_(obj, op, mode, twowayOnly, os, hasOutParams, exceptions, varargin)
            if isempty(obj.impl_)
                obj.instantiate_();
            end

            try
                % Vararg accepted for optional context argument.
                if length(varargin) > 1
                    throw(MException('Ice:ArgumentException', 'one optional argument is allowed for request context'))
                end

                if twowayOnly && ~obj.isTwoway
                    throw(Ice.TwowayOnlyException('', 'invocation requires twoway proxy', op));
                end

                if isempty(os)
                    buf = [];
                    size = 0;
                else
                    buf = os.buf.buf;
                    size = os.buf.size;
                end

                if length(varargin) == 1
                    %
                    % Avoid the string concatenation
                    %
                    % res = obj.callWithResult_('ice_invoke', op, mode, buf, size, varargin{1});
                    %
                    res = IceInternal.Util.callWithResult('Ice_ObjectPrx_ice_invoke', obj.impl_, op, mode, buf, ...
                                                          size, varargin{1});
                else
                    %
                    % Avoid the string concatenation
                    %
                    % res = obj.callWithResult_('ice_invokeNC', op, mode, buf, size);
                    %
                    res = IceInternal.Util.callWithResult('Ice_ObjectPrx_ice_invokeNC', obj.impl_, op, mode, buf, size);
                end

                is = [];
                if ~isempty(res.params)
                    if isempty(obj.cachedInputStream)
                        is = Ice.InputStream(obj.communicator, obj.encoding, IceInternal.Buffer(res.params));
                        obj.cachedInputStream = is;
                    else
                        is = obj.cachedInputStream;
                        is.reset(res.params);
                    end
                end

                if obj.isTwoway
                    if ~res.ok
                        obj.throwUserException_(is, exceptions{:});
                    elseif ~hasOutParams
                        is.skipEmptyEncapsulation();
                    end
                end
            catch ex
                ex.throwAsCaller();
            end
        end

        function fut = invokeAsync_(obj, op, mode, twowayOnly, os, numOutArgs, unmarshalFunc, exceptions, varargin)
            if isempty(obj.impl_)
                obj.instantiate_();
            end

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
                        % res = f.callWithResult_('results');
                        %
                        res = IceInternal.Util.callWithResult('Ice_InvocationFuture_results', f.impl_);
                        is = Ice.InputStream(obj.communicator, obj.encoding, IceInternal.Buffer(res.params));
                        if ~res.ok
                            obj.throwUserException_(is, exceptions{:});
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
                        % f.call_('check');
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
                    throw(MException('Ice:ArgumentException', 'one optional argument is allowed for request context'))
                end
                if twowayOnly && ~isTwoway
                    throw(Ice.TwowayOnlyException('', 'invocation requires twoway proxy', op));
                end
                if isempty(os)
                    buf = [];
                    size = 0;
                else
                    buf = os.buf.buf;
                    size = os.buf.size;
                end
                futPtr = libpointer('voidPtr'); % Output param
                if length(varargin) == 1
                    %
                    % Avoid the string concatenation
                    %
                    % obj.call_('ice_invokeAsync', op, mode, buf, size, varargin{1}, futPtr);
                    %
                    IceInternal.Util.call('Ice_ObjectPrx_ice_invokeAsync', obj.impl_, op, mode, buf, size, ...
                                          varargin{1}, futPtr);
                else
                    %
                    % Avoid the string concatenation
                    %
                    % obj.call_('ice_invokeAsyncNC', op, mode, buf, size, futPtr);
                    %
                    IceInternal.Util.call('Ice_ObjectPrx_ice_invokeAsyncNC', obj.impl_, op, mode, buf, size, futPtr);
                end
                assert(~isNull(futPtr));
                fut = Ice.Future(futPtr, op, numOutArgs, 'Ice_InvocationFuture', @fetch);
            catch ex
                ex.throwAsCaller();
            end
        end

        function throwUserException_(obj, is, varargin) % Varargs are user exception type names
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
                    uue = Ice.UnknownUserException('', '', ex.ice_id());
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
            if length(varargin) == 0
                r = p;
            else
                r = Ice.ObjectPrx.checkedCast_(p, Ice.ObjectPrx.ice_staticId(), 'Ice.ObjectPrx', varargin{:});
            end
        end

        function r = uncheckedCast(p, varargin)
            if length(varargin) == 0
                r = p;
            elseif length(varargin) == 1
                if ~isempty(p)
                    r = p.ice_facet(varargin{1});
                else
                    r = p;
                end
            else
                throw(MException('Ice:ArgumentException', 'too many arguments to uncheckedCast'));
            end
        end
    end

    methods(Static,Access=protected)
        function r = checkedCast_(p, id, cls, varargin)
            try
                hasFacet = false;
                facet = [];
                context = {};
                if length(varargin) == 1
                    if isa(varargin{1}, 'containers.Map')
                        context = { varargin{1} };
                    elseif isempty(varargin{1}) || isa(varargin{1}, 'char')
                        hasFacet = true;
                        facet = varargin{1};
                    else
                        throw(MException('Ice:ArgumentException', 'expecting string or containers.Map'));
                    end
                elseif length(varargin) == 2
                    hasFacet = true;
                    facet = varargin{1};
                    context = { varargin{2} };
                elseif length(varargin) > 2
                    throw(MException('Ice:ArgumentException', 'too many arguments to checkedCast'));
                end
                if ~isempty(p)
                    if hasFacet
                        p = p.ice_facet(facet);
                    end
                    if isa(p, cls)
                        r = p;
                    elseif p.ice_isA(id, context{:})
                        constructor = str2func(cls);
                        r = constructor(p.communicator, p.encoding, p.clone_(), []);
                    else
                        r = [];
                    end
                else
                    r = p;
                end
            catch ex
                ex.throwAsCaller();
            end
        end

        function r = uncheckedCast_(p, cls, varargin)
            hasFacet = false;
            facet = [];
            if length(varargin) == 1
                hasFacet = true;
                facet = varargin{1};
            elseif length(varargin) > 1
                throw(MException('Ice:ArgumentException', 'too many arguments to uncheckedCast'));
            end
            if ~isempty(p)
                if hasFacet
                    p = p.ice_facet(facet);
                end
                if isa(p, cls)
                    r = p;
                else
                    constructor = str2func(cls);
                    r = constructor(p.communicator, p.encoding, p.clone_(), []);
                end
            else
                r = p;
            end
        end
    end

    methods(Access=private)
        function instantiate_(obj)
            %
            % An unmarshaled proxy delays the creation of its corresponding C++ object until the application
            % needs it. To obtain the C++ object, we unmarshal it (again) by calling into C++ to extract it
            % from the byte buffer that contains the proxy's marshaled form.
            %
            if isempty(obj.impl_)
                assert(~isempty(obj.bytes));
                %
                % Call into C++ to construct a proxy. We pass the data buffer and start position (adjusted for
                % C-style pointers), along with the size of the entire buffer. The C++ implementation reads what
                % it needs and returns the new proxy object as well as number of bytes it consumed.
                %
                impl = libpointer('voidPtr');
                start = 0; % Starting position for a C-style pointer.
                IceInternal.Util.call('Ice_ObjectPrx_read', obj.communicator.impl_, obj.encoding, obj.bytes, ...
                                      start, length(obj.bytes), impl);
                obj.impl_ = impl;

                % Cache the twoway status
                obj.isTwoway = obj.callWithResult_('ice_isTwoway');
            end
        end

        function r = factory_(obj, op, keepType, varargin)
            %
            % Call a C++ proxy factory function. The function returns nil if the call results in no change to the
            % proxy, in which case we can return the current object.
            %

            obj.instantiate_();

            newImpl = libpointer('voidPtr');
            obj.call_(op, newImpl, varargin{:});
            if isNull(newImpl)
                r = obj;
            elseif keepType
                r = obj.newInstance_(newImpl); % Retain the proxy's current type.
            else
                %
                % We don't retain the proxy's existing type for a couple of factory functions.
                %
                r = Ice.ObjectPrx(obj.communicator, obj.encoding, newImpl);
            end
        end

        function r = newInstance_(obj, impl)
            %
            % Return a new instance of this proxy type.
            %
            constructor = str2func(class(obj)); % Obtain the constructor for this class
            r = constructor(obj.communicator, obj.encoding, impl, []); % Call the constructor
        end

        function r = clone_(obj)
            %
            % Clone the C++ reference for use by a new instance of ObjectPrx.
            %
            obj.instantiate_();
            implPtr = libpointer('voidPtr'); % Output param
            obj.call_('clone', implPtr);
            r = implPtr;
        end
    end

    properties(Access=private)
        communicator % The communicator wrapper
        encoding
        isTwoway
        cachedInputStream % Only used for synchronous invocations
        bytes
    end
end
