%{
**********************************************************************

Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.

This copy of Ice is licensed to you under the terms described in the
ICE_LICENSE file included in this distribution.

**********************************************************************
%}

classdef ObjectPrx < IceInternal.WrapperObject
    methods
        function obj = ObjectPrx(impl, communicator)
            obj = obj@IceInternal.WrapperObject(impl, 'Ice_ObjectPrx');
            obj.communicator = communicator;
        end

        function delete(obj)
            obj.call_('_release');
            obj.impl_ = [];
        end

        %
        % Override == operator.
        %
        function r = eq(obj, other)
            if isempty(other) || ~isa(other, 'Ice.ObjectPrx')
                r = false;
            else
                v = libpointer('uint8Ptr', 0);
                obj.call_('equals', other.impl_, v);
                r = v.Value == 1;
            end
        end

        function r = ice_createOutputStream(obj)
            stream = libpointer('voidPtr');
            obj.call_('ice_createOutputStream', stream);
            r = Ice.OutputStream(stream, obj.communicator);
        end

        function r = ice_toString(obj)
            r = obj.callWithResult_('ice_toString');
        end

        function r = ice_getCommunicator(obj)
            r = obj.communicator;
        end

        function ice_ping(obj, varargin)
            [ok, inStream] = obj.invoke_('ice_ping', 'Nonmutating', false, [], varargin{:});
            obj.checkNoResponse_(ok, inStream);
        end

        function r = ice_pingAsync(obj, varargin)
            r = obj.invokeAsync_('ice_ping', 'Nonmutating', false, [], 0, [], varargin{:});
        end

        function r = ice_isA(obj, id, varargin)
            os = obj.startWriteParams_();
            os.writeString(id);
            obj.endWriteParams_(os);
            [ok, inStream] = obj.invoke_('ice_isA', 'Nonmutating', true, os, varargin{:});
            if ok
                inStream.startEncapsulation();
                r = inStream.readBool();
                inStream.endEncapsulation();
            else
                obj.throwUserException_(inStream);
            end
        end

        function r = ice_isAAsync(obj, id, varargin)
            os = obj.startWriteParams_();
            os.writeString(id);
            obj.endWriteParams_(os);
            function varargout = unmarshal(ok, is)
                if ok
                    is.startEncapsulation();
                    varargout{1} = is.readBool();
                    is.endEncapsulation();
                else
                    obj.throwUserException_(is);
                end
            end
            r = obj.invokeAsync_('ice_isA', 'Nonmutating', true, os, 1, @unmarshal, varargin{:});
        end

        function r = ice_id(obj, varargin)
            [ok, inStream] = obj.invoke_('ice_id', 'Nonmutating', true, [], varargin{:});
            if ok
                inStream.startEncapsulation();
                r = inStream.readString();
                inStream.endEncapsulation();
            else
                obj.throwUserException_(inStream);
            end
        end

        function r = ice_idAsync(obj, varargin)
            function varargout = unmarshal(ok, is)
                if ok
                    is.startEncapsulation();
                    varargout{1} = is.readString();
                    is.endEncapsulation();
                else
                    obj.throwUserException_(is);
                end
            end
            r = obj.invokeAsync_('ice_id', 'Nonmutating', true, [], 1, @unmarshal, varargin{:});
        end

        function r = ice_ids(obj, varargin)
            [ok, inStream] = obj.invoke_('ice_ids', 'Nonmutating', true, [], varargin{:});
            if ok
                inStream.startEncapsulation();
                r = inStream.readStringSeq();
                inStream.endEncapsulation();
            else
                obj.throwUserException_(inStream);
            end
        end

        function r = ice_idsAsync(obj, varargin)
            function varargout = unmarshal(ok, is)
                if ok
                    is.startEncapsulation();
                    varargout{1} = is.readStringSeq();
                    is.endEncapsulation();
                else
                    obj.throwUserException_(is);
                end
            end
            r = obj.invokeAsync_('ice_ids', 'Nonmutating', true, [], 1, @unmarshal, varargin{:});
        end

        function r = ice_getIdentity(obj)
            r = obj.callWithResult_('ice_getIdentity');
        end

        function r = ice_identity(obj, id)
            v = libpointer('voidPtr');
            obj.call_('ice_identity', id, v);
            if isNull(v)
                r = obj;
            else
                r = Ice.ObjectPrx(v, obj.communicator); % Don't use newInstance_ here
            end
        end

        function r = ice_getContext(obj)
            r = obj.callWithResult_('ice_getContext');
        end

        function r = ice_context(obj, ctx)
            v = libpointer('voidPtr');
            obj.call_('ice_context', ctx, v);
            if isNull(v)
                r = obj;
            else
                r = obj.newInstance_(v);
            end
        end

        function r = ice_getFacet(obj)
            r = obj.callWithResult_('ice_getFacet');
        end

        function r = ice_facet(obj, f)
            v = libpointer('voidPtr');
            obj.call_('ice_facet', f, v);
            if isNull(v)
                r = obj;
            else
                r = Ice.ObjectPrx(v, obj.communicator); % Don't use newInstance_ here
            end
        end

        function r = ice_getAdapterId(obj)
            r = obj.callWithResult_('ice_getAdapterId');
        end

        function r = ice_adapterId(obj, id)
            v = libpointer('voidPtr');
            obj.call_('ice_adapterId', id, v);
            if isNull(v)
                r = obj;
            else
                r = obj.newInstance_(v);
            end
        end

        function r = ice_getEndpoints(obj)
            r = obj.callWithResult_('ice_getEndpoints');
        end

        function r = ice_endpoints(obj, endpts)
            v = libpointer('voidPtr');
            obj.call_('ice_endpoints', endpts, v);
            if isNull(v)
                r = obj;
            else
                r = obj.newInstance_(v);
            end
        end

        function r = ice_getLocatorCacheTimeout(obj)
            v = libpointer('int32Ptr', 0);
            obj.call_('ice_getLocatorCacheTimeout', v);
            r = v.Value;
        end

        function r = ice_locatorCacheTimeout(obj, t)
            v = libpointer('voidPtr');
            obj.call_('ice_locatorCacheTimeout', t, v);
            if isNull(v)
                r = obj;
            else
                r = obj.newInstance_(v);
            end
        end

        function r = ice_getInvocationTimeout(obj)
            v = libpointer('int32Ptr', 0);
            obj.call_('ice_getInvocationTimeout', v);
            r = v.Value;
        end

        function r = ice_invocationTimeout(obj, t)
            v = libpointer('voidPtr');
            obj.call_('ice_invocationTimeout', t, v);
            if isNull(v)
                r = obj;
            else
                r = obj.newInstance_(v);
            end
        end

        function r = ice_getConnectionId(obj)
            r = obj.callWithResult_('ice_getConnectionId');
        end

        function r = ice_connectionId(obj, id)
            v = libpointer('voidPtr');
            obj.call_('ice_connectionId', id, v);
            if isNull(v)
                r = obj;
            else
                r = obj.newInstance_(v);
            end
        end

        function r = ice_isConnectionCached(obj)
            v = libpointer('uint8Ptr', 0);
            obj.call_('ice_isConnectionCached', v);
            r = v.Value == 1;
        end

        function r = ice_connectionCached(obj, b)
            v = libpointer('voidPtr');
            if b
                val = 1;
            else
                val = 0;
            end
            obj.call_('ice_connectionCached', val, v);
            if isNull(v)
                r = obj;
            else
                r = obj.newInstance_(v);
            end
        end

        function r = ice_getEndpointSelection(obj)
            r = obj.callWithResult_('ice_getEndpointSelection');
        end

        function r = ice_endpointSelection(obj, t)
            v = libpointer('voidPtr');
            obj.call_('ice_endpointSelection', t, v);
            if isNull(v)
                r = obj;
            else
                r = obj.newInstance_(v);
            end
        end

        function r = ice_getEncodingVersion(obj)
            r = obj.callWithResult_('ice_getEncodingVersion');
        end

        function r = ice_encodingVersion(obj, ver)
            v = libpointer('voidPtr');
            obj.call_('ice_encodingVersion', ver, v);
            if isNull(v)
                r = obj;
            else
                r = obj.newInstance_(v);
            end
        end

        function r = ice_getRouter(obj)
            v = libpointer('voidPtr');
            obj.call_('ice_getRouter', v);
            if isNull(v)
                r = [];
            else
                r = Ice.RouterPrx(v, obj.communicator);
            end
        end

        function r = ice_router(obj, rtr)
            v = libpointer('voidPtr');
            if isempty(rtr)
                impl = libpointer('voidPtr');
            else
                impl = rtr.impl_;
            end
            obj.call_('ice_router', impl, v);
            if isNull(v)
                r = obj;
            else
                r = obj.newInstance_(v);
            end
        end

        function r = ice_getLocator(obj)
            v = libpointer('voidPtr');
            obj.call_('ice_getLocator', v);
            if isNull(v)
                r = [];
            else
                r = Ice.LocatorPrx(v, obj.communicator);
            end
        end

        function r = ice_locator(obj, loc)
            v = libpointer('voidPtr');
            if isempty(loc)
                impl = libpointer('voidPtr');
            else
                impl = loc.impl_;
            end
            obj.call_('ice_locator', impl, v);
            if isNull(v)
                r = obj;
            else
                r = obj.newInstance_(v);
            end
        end

        function r = ice_isSecure(obj)
            v = libpointer('uint8Ptr', 0);
            obj.call_('ice_isSecure', v);
            r = v.Value == 1;
        end

        function r = ice_secure(obj, b)
            v = libpointer('voidPtr');
            if b
                val = 1;
            else
                val = 0;
            end
            obj.call_('ice_secure', val, v);
            if isNull(v)
                r = obj;
            else
                r = obj.newInstance_(v);
            end
        end

        function r = ice_isPreferSecure(obj)
            v = libpointer('uint8Ptr', 0);
            obj.call_('ice_isPreferSecure', v);
            r = v.Value == 1;
        end

        function r = ice_preferSecure(obj, b)
            v = libpointer('voidPtr');
            if b
                val = 1;
            else
                val = 0;
            end
            obj.call_('ice_preferSecure', val, v);
            if isNull(v)
                r = obj;
            else
                r = obj.newInstance_(v);
            end
        end

        function r = ice_isTwoway(obj)
            v = libpointer('uint8Ptr', 0);
            obj.call_('ice_isTwoway', v);
            r = v.Value == 1;
        end

        function r = ice_twoway(obj)
            v = libpointer('voidPtr');
            obj.call_('ice_twoway', v);
            if isNull(v)
                r = obj;
            else
                r = obj.newInstance_(v);
            end
        end

        function r = ice_isOneway(obj)
            v = libpointer('uint8Ptr', 0);
            obj.call_('ice_isOneway', v);
            r = v.Value == 1;
        end

        function r = ice_oneway(obj)
            v = libpointer('voidPtr');
            obj.call_('ice_oneway', v);
            if isNull(v)
                r = obj;
            else
                r = obj.newInstance_(v);
            end
        end

        function r = ice_isBatchOneway(obj)
            v = libpointer('uint8Ptr', 0);
            obj.call_('ice_isBatchOneway', v);
            r = v.Value == 1;
        end

        function r = ice_batchOneway(obj)
            v = libpointer('voidPtr');
            obj.call_('ice_batchOneway', v);
            if isNull(v)
                r = obj;
            else
                r = obj.newInstance_(v);
            end
        end

        function r = ice_isDatagram(obj)
            v = libpointer('uint8Ptr', 0);
            obj.call_('ice_isDatagram', v);
            r = v.Value == 1;
        end

        function r = ice_datagram(obj)
            v = libpointer('voidPtr');
            obj.call_('ice_datagram', v);
            if isNull(v)
                r = obj;
            else
                r = obj.newInstance_(v);
            end
        end

        function r = ice_isBatchDatagram(obj)
            v = libpointer('uint8Ptr', 0);
            obj.call_('ice_isBatchDatagram', v);
            r = v.Value == 1;
        end

        function r = ice_batchDatagram(obj)
            v = libpointer('voidPtr');
            obj.call_('ice_batchDatagram', v);
            if isNull(v)
                r = obj;
            else
                r = obj.newInstance_(v);
            end
        end

        function r = ice_compress(obj, b)
            v = libpointer('voidPtr');
            if b
                val = 1;
            else
                val = 0;
            end
            obj.call_('ice_compress', val, v);
            if isNull(v)
                r = obj;
            else
                r = obj.newInstance_(v);
            end
        end

        function r = ice_timeout(obj, t)
            v = libpointer('voidPtr');
            obj.call_('ice_timeout', t, v);
            if isNull(v)
                r = obj;
            else
                r = obj.newInstance_(v);
            end
        end

        function r = ice_getConnection(obj)
            v = libpointer('voidPtr');
            obj.call_('ice_getConnection', v);
            if isNull(v)
                r = [];
            else
                r = Ice.Connection(v, obj.communicator);
            end
        end

        function r = ice_getConnectionAsync(obj)
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
            v = libpointer('voidPtr');
            obj.call_('ice_getCachedConnection', v);
            if isNull(v)
                r = [];
            else
                r = Ice.Connection(v);
            end
        end

        function ice_flushBatchRequests(obj)
            obj.call_('ice_flushBatchRequests');
        end

        function r = ice_flushBatchRequestsAsync(obj)
            future = libpointer('voidPtr');
            obj.call_('ice_flushBatchRequestsAsync', future);
            assert(~isNull(future));
            r = Ice.Future(future, 'ice_flushBatchRequests', 0, 'Ice_SimpleFuture', []);
        end
    end

    methods(Access=protected)
        function checkNoResponse_(obj, ok, inStream)
            if obj.ice_isTwoway()
                if ok == 0
                    try
                        obj.throwUserException_(inStream);
                    catch ex
                        ex.throwAsCaller();
                    end
                else
                    inStream.skipEmptyEncapsulation();
                end
            end
        end

        function os = startWriteParams_(obj)
            os = obj.ice_createOutputStream();
            os.startEncapsulation([]);
        end

        function os = startWriteParamsWithFormat_(obj, format)
            os = obj.ice_createOutputStream();
            os.startEncapsulation(format);
        end

        function endWriteParams_(obj, os)
            os.endEncapsulation();
        end

        function [ok, inStream] = invoke_(obj, op, mode, twowayOnly, os, varargin)
            try
                % Vararg accepted for optional context argument.
                if length(varargin) > 1
                    throw(MException('Ice:ArgumentException', 'one optional argument is allowed for request context'))
                end
                if twowayOnly && ~obj.ice_isTwoway()
                    throw(Ice.TwowayOnlyException('', 'invocation requires twoway proxy', op));
                end
                if ~isempty(os)
                    outStream = os.impl_;
                else
                    outStream = libpointer('voidPtr'); % Null pointer for output stream
                end
                okPtr = libpointer('uint8Ptr', 0); % Output param
                inStreamPtr = libpointer('voidPtr'); % Output param
                if length(varargin) == 1
                    obj.call_('ice_invoke', op, mode, outStream, varargin{1}, okPtr, inStreamPtr);
                else
                    obj.call_('ice_invokeNC', op, mode, outStream, okPtr, inStreamPtr);
                end
                ok = okPtr.Value == 1;
                inStream = [];
                if ~isNull(inStreamPtr)
                    inStream = Ice.InputStream(inStreamPtr, obj.communicator);
                end
            catch ex
                ex.throwAsCaller();
            end
        end

        function fut = invokeAsync_(obj, op, mode, twowayOnly, os, numOutArgs, unmarshalFunc, varargin)
            isTwoway = obj.ice_isTwoway();

            % This nested function is invoked by Future.fetchOutputs()
            function varargout = fetch(f)
                try
                    if isTwoway
                        %
                        % Retrieve a boolean indicating whether the request completed successfully or returned
                        % a user exception. The 'stream' function can also raise a local exception if necessary.
                        %
                        okPtr = libpointer('uint8Ptr', 0); % Output param
                        inStreamPtr = libpointer('voidPtr', 0); % Output param
                        f.call_('stream', okPtr, inStreamPtr);
                        ok = okPtr.Value == 1;
                        assert(~isNull(inStreamPtr));
                        inStream = Ice.InputStream(inStreamPtr, obj.communicator);
                        if isempty(unmarshalFunc)
                            if ~ok
                                % Unexpected user exception
                                obj.throwUserException_(inStream);
                            else
                                inStream.skipEmptyEncapsulation();
                            end
                        else
                            [varargout{1:numOutArgs}] = unmarshalFunc(ok, inStream);
                        end
                    else
                        %
                        % Check for a local exception.
                        %
                        f.call_('check');
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
                if twowayOnly && ~obj.ice_isTwoway()
                    throw(Ice.TwowayOnlyException('', 'invocation requires twoway proxy', op));
                end
                if ~isempty(os)
                    outStream = os.impl_;
                else
                    outStream = libpointer('voidPtr'); % Null pointer for output stream
                end
                futPtr = libpointer('voidPtr'); % Output param
                if length(varargin) == 1
                    obj.call_('ice_invokeAsync', op, mode, outStream, varargin{1}, futPtr);
                else
                    obj.call_('ice_invokeAsyncNC', op, mode, outStream, futPtr);
                end
                assert(~isNull(futPtr));
                fut = Ice.Future(futPtr, op, numOutArgs, 'Ice_InvocationFuture', @fetch);
            catch ex
                ex.throwAsCaller();
            end
        end

        function throwUserException_(obj, inStream, varargin) % Varargs are user exception type names
            try
                inStream.startEncapsulation();
                inStream.throwException();
            catch ex
                if isa(ex, 'Ice.UserException')
                    inStream.endEncapsulation();
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

        function r = newInstance_(obj, impl)
            constructor = str2func(class(obj)); % Obtain the constructor for this class
            r = constructor(impl, obj.communicator); % Call the constructor
        end

        function r = clone_(obj)
            implPtr = libpointer('voidPtr'); % Output param
            obj.call_('clone', implPtr);
            r = implPtr;
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
                r = Ice.ObjectPrx.checkedCast(p, Ice.ObjectPrx.ice_staticId(), 'Ice.ObjectPrx', varargin{:});
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
        function r = read_(inStream, cls)
            p = inStream.readProxy();
            if ~isempty(p)
                constructor = str2func(cls);
                r = constructor(p.clone_(), inStream.getCommunicator());
            else
                r = [];
            end
        end

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
                        r = constructor(p.clone_(), p.communicator);
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
            facet = [];
            if length(varargin) == 1
                facet = varargin{1};
            elseif length(varargin) > 1
                throw(MException('Ice:ArgumentException', 'too many arguments to uncheckedCast'));
            end
            if ~isempty(p)
                if ~isempty(facet)
                    p = p.ice_facet(facet);
                end
                if isa(p, cls)
                    r = p;
                else
                    constructor = str2func(cls);
                    r = constructor(p.clone_(), p.communicator);
                end
            else
                r = p;
            end
        end
    end

    properties(Access=private)
        communicator % The communicator wrapper
    end
end
