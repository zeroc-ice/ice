%{
**********************************************************************

Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.

This copy of Ice is licensed to you under the terms described in the
ICE_LICENSE file included in this distribution.

**********************************************************************
%}

classdef ObjectPrx < Ice.WrapperObject
    methods
        function self = ObjectPrx(impl)
            self = self@Ice.WrapperObject(impl);
        end

        function delete(self)
            self.callMethod('_release');
            self.impl = [];
        end

        %
        % Override == operator.
        %
        function r = eq(self, other)
            if isempty(other) || ~isa(other, 'Ice.ObjectPrx')
                r = false;
            else
                v = libpointer('uint8Ptr', 0);
                self.callMethod('equals', other.impl, v);
                r = v.Value == 1;
            end
        end

        function r = ice_createOutputStream(self)
            stream = libpointer('voidPtr');
            self.callMethod('ice_createOutputStream', stream);
            r = Ice.OutputStream(stream);
        end

        function r = ice_toString(self)
            r = self.callMethodWithResult('ice_toString');
        end

        function r = ice_getCommunicator(self)
            v = libpointer('voidPtr');
            self.callMethod('ice_getCommunicator', v);
            assert(~isNull(v));
            r = Ice.Communicator(v);
        end

        function ice_ping(self, varargin)
            [ok, inStream] = self.invoke_('ice_ping', 'Nonmutating', false, [], varargin{:});
            self.checkNoResponse_(ok, inStream);
        end

        function r = ice_pingAsync(self, varargin)
            r = self.invokeAsync_('ice_ping', 'Nonmutating', false, [], 0, [], varargin{:});
        end

        function r = ice_isA(self, id, varargin)
            os = self.startWriteParams_();
            os.writeString(id);
            self.endWriteParams_(os);
            [ok, inStream] = self.invoke_('ice_isA', 'Nonmutating', true, os, varargin{:});
            if ok
                inStream.startEncapsulation();
                r = inStream.readBool();
                inStream.endEncapsulation();
            else
                self.throwUserException_(inStream);
            end
        end

        function r = ice_isAAsync(self, id, varargin)
            os = self.startWriteParams_();
            os.writeString(id);
            self.endWriteParams_(os);
            function varargout = unmarshal(ok, is)
                if ok
                    is.startEncapsulation();
                    varargout{1} = is.readBool();
                    is.endEncapsulation();
                else
                    self.throwUserException_(is);
                end
            end
            r = self.invokeAsync_('ice_isA', 'Nonmutating', true, os, 1, @unmarshal, varargin{:});
        end

        function r = ice_id(self, varargin)
            [ok, inStream] = self.invoke_('ice_id', 'Nonmutating', true, [], varargin{:});
            if ok
                inStream.startEncapsulation();
                r = inStream.readString();
                inStream.endEncapsulation();
            else
                self.throwUserException_(inStream);
            end
        end

        function r = ice_idAsync(self, varargin)
            function varargout = unmarshal(ok, is)
                if ok
                    is.startEncapsulation();
                    varargout{1} = is.readString();
                    is.endEncapsulation();
                else
                    self.throwUserException_(is);
                end
            end
            r = self.invokeAsync_('ice_id', 'Nonmutating', true, [], 1, @unmarshal, varargin{:});
        end

        function r = ice_ids(self, varargin)
            [ok, inStream] = self.invoke_('ice_ids', 'Nonmutating', true, [], varargin{:});
            if ok
                inStream.startEncapsulation();
                r = inStream.readStringSeq();
                inStream.endEncapsulation();
            else
                self.throwUserException_(inStream);
            end
        end

        function r = ice_idsAsync(self, varargin)
            function varargout = unmarshal(ok, is)
                if ok
                    is.startEncapsulation();
                    varargout{1} = is.readStringSeq();
                    is.endEncapsulation();
                else
                    self.throwUserException_(is);
                end
            end
            r = self.invokeAsync_('ice_ids', 'Nonmutating', true, [], 1, @unmarshal, varargin{:});
        end

        function r = ice_getIdentity(self)
            r = self.callMethodWithResult('ice_getIdentity');
        end

        function r = ice_identity(self, id)
            v = libpointer('voidPtr');
            self.callMethod('ice_identity', id, v);
            if isNull(v)
                r = self;
            else
                r = Ice.ObjectPrx(v); % Don't use newInstance_ here
            end
        end

        function r = ice_getContext(self)
            r = self.callMethodWithResult('ice_getContext');
        end

        function r = ice_context(self, ctx)
            v = libpointer('voidPtr');
            self.callMethod('ice_context', ctx, v);
            if isNull(v)
                r = self;
            else
                r = self.newInstance_(v);
            end
        end

        function r = ice_getFacet(self)
            r = self.callMethodWithResult('ice_getFacet');
        end

        function r = ice_facet(self, f)
            v = libpointer('voidPtr');
            self.callMethod('ice_facet', f, v);
            if isNull(v)
                r = self;
            else
                r = Ice.ObjectPrx(v); % Don't use newInstance_ here
            end
        end

        function r = ice_getAdapterId(self)
            r = self.callMethodWithResult('ice_getAdapterId');
        end

        function r = ice_adapterId(self, id)
            v = libpointer('voidPtr');
            self.callMethod('ice_adapterId', id, v);
            if isNull(v)
                r = self;
            else
                r = self.newInstance_(v);
            end
        end

        function r = ice_getEndpoints(self)
            r = self.callMethodWithResult('ice_getEndpoints');
        end

        function r = ice_endpoints(self, endpts)
            v = libpointer('voidPtr');
            self.callMethod('ice_endpoints', endpts, v);
            if isNull(v)
                r = self;
            else
                r = self.newInstance_(v);
            end
        end

        function r = ice_getLocatorCacheTimeout(self)
            v = libpointer('int32Ptr', 0);
            self.callMethod('ice_getLocatorCacheTimeout', v);
            r = v.Value;
        end

        function r = ice_locatorcacheTimeout(self, t)
            v = libpointer('voidPtr');
            self.callMethod('ice_locatorCacheTimeout', t, v);
            if isNull(v)
                r = self;
            else
                r = self.newInstance_(v);
            end
        end

        function r = ice_getInvocationTimeout(self)
            v = libpointer('int32Ptr', 0);
            self.callMethod('ice_getInvocationTimeout', v);
            r = v.Value;
        end

        function r = ice_invocationTimeout(self, t)
            v = libpointer('voidPtr');
            self.callMethod('ice_invocationTimeout', t, v);
            if isNull(v)
                r = self;
            else
                r = self.newInstance_(v);
            end
        end

        function r = ice_getConnectionId(self)
            r = self.callMethodWithResult('ice_getConnectionId');
        end

        function r = ice_connectionId(self, id)
            v = libpointer('voidPtr');
            self.callMethod('ice_connectionId', id, v);
            if isNull(v)
                r = self;
            else
                r = self.newInstance_(v);
            end
        end

        function r = ice_isConnectionCached(self)
            v = libpointer('uint8Ptr', 0);
            self.callMethod('ice_isConnectionCached', v);
            r = v.Value == 1;
        end

        function r = ice_connectionCached(self, b)
            v = libpointer('voidPtr');
            if b
                val = 1;
            else
                val = 0;
            end
            self.callMethod('ice_connectionCached', val, v);
            if isNull(v)
                r = self;
            else
                r = self.newInstance_(v);
            end
        end

        function r = ice_getEndpointSelection(self)
            v = libpointer('Ice_EndpointSelectionType', 'Random');
            self.callMethod('ice_getEndpointSelection', v);
            r = v.Value;
        end

        function r = ice_endpointSelection(self, t)
            v = libpointer('voidPtr');
            self.callMethod('ice_endpointSelection', t, v);
            if isNull(v)
                r = self;
            else
                r = self.newInstance_(v);
            end
        end

        function r = ice_getEncodingVersion(self)
            r = self.callMethodWithResult('ice_getEncodingVersion');
        end

        function r = ice_encodingVersion(self, ver)
            v = libpointer('voidPtr');
            self.callMethod('ice_encodingVersion', ver, v);
            if isNull(v)
                r = self;
            else
                r = self.newInstance_(v);
            end
        end

        function r = ice_getRouter(self)
            v = libpointer('voidPtr');
            self.callMethod('ice_getRouter', v);
            if isNull(v)
                r = [];
            else
                r = Ice.RouterPrx(v);
            end
        end

        function r = ice_router(self, rtr)
            v = libpointer('voidPtr');
            if isempty(rtr)
                impl = libpointer('voidPtr');
            else
                impl = rtr.impl;
            end
            self.callMethod('ice_router', impl, v);
            if isNull(v)
                r = self;
            else
                r = self.newInstance_(v);
            end
        end

        function r = ice_getLocator(self)
            v = libpointer('voidPtr');
            self.callMethod('ice_getLocator', v);
            if isNull(v)
                r = [];
            else
                r = Ice.LocatorPrx(v);
            end
        end

        function r = ice_locator(self, loc)
            v = libpointer('voidPtr');
            if isempty(loc)
                impl = libpointer('voidPtr');
            else
                impl = loc.impl;
            end
            self.callMethod('ice_locator', impl, v);
            if isNull(v)
                r = self;
            else
                r = self.newInstance_(v);
            end
        end

        function r = ice_isSecure(self)
            v = libpointer('uint8Ptr', 0);
            self.callMethod('ice_isSecure', v);
            r = v.Value == 1;
        end

        function r = ice_secure(self, b)
            v = libpointer('voidPtr');
            if b
                val = 1;
            else
                val = 0;
            end
            self.callMethod('ice_secure', val, v);
            if isNull(v)
                r = self;
            else
                r = self.newInstance_(v);
            end
        end

        function r = ice_isPreferSecure(self)
            v = libpointer('uint8Ptr', 0);
            self.callMethod('ice_isPreferSecure', v);
            r = v.Value == 1;
        end

        function r = ice_preferSecure(self, b)
            v = libpointer('voidPtr');
            if b
                val = 1;
            else
                val = 0;
            end
            self.callMethod('ice_preferSecure', val, v);
            if isNull(v)
                r = self;
            else
                r = self.newInstance_(v);
            end
        end

        function r = ice_isTwoway(self)
            v = libpointer('uint8Ptr', 0);
            self.callMethod('ice_isTwoway', v);
            r = v.Value == 1;
        end

        function r = ice_twoway(self)
            v = libpointer('voidPtr');
            self.callMethod('ice_twoway', v);
            if isNull(v)
                r = self;
            else
                r = self.newInstance_(v);
            end
        end

        function r = ice_isOneway(self)
            v = libpointer('uint8Ptr', 0);
            self.callMethod('ice_isOneway', v);
            r = v.Value == 1;
        end

        function r = ice_oneway(self)
            v = libpointer('voidPtr');
            self.callMethod('ice_oneway', v);
            if isNull(v)
                r = self;
            else
                r = self.newInstance_(v);
            end
        end

        function r = ice_isBatchOneway(self)
            v = libpointer('uint8Ptr', 0);
            self.callMethod('ice_isBatchOneway', v);
            r = v.Value == 1;
        end

        function r = ice_batchOneway(self)
            v = libpointer('voidPtr');
            self.callMethod('ice_batchOneway', v);
            if isNull(v)
                r = self;
            else
                r = self.newInstance_(v);
            end
        end

        function r = ice_isDatagram(self)
            v = libpointer('uint8Ptr', 0);
            self.callMethod('ice_isDatagram', v);
            r = v.Value == 1;
        end

        function r = ice_datagram(self)
            v = libpointer('voidPtr');
            self.callMethod('ice_datagram', v);
            if isNull(v)
                r = self;
            else
                r = self.newInstance_(v);
            end
        end

        function r = ice_isBatchDatagram(self)
            v = libpointer('uint8Ptr', 0);
            self.callMethod('ice_isBatchDatagram', v);
            r = v.Value == 1;
        end

        function r = ice_batchDatagram(self)
            v = libpointer('voidPtr');
            self.callMethod('ice_batchDatagram', v);
            if isNull(v)
                r = self;
            else
                r = self.newInstance_(v);
            end
        end

        function r = ice_compress(self, b)
            v = libpointer('voidPtr');
            if b
                val = 1;
            else
                val = 0;
            end
            self.callMethod('ice_compress', val, v);
            if isNull(v)
                r = self;
            else
                r = self.newInstance_(v);
            end
        end

        function r = ice_timeout(self, t)
            v = libpointer('voidPtr');
            self.callMethod('ice_timeout', t, v);
            if isNull(v)
                r = self;
            else
                r = self.newInstance_(v);
            end
        end

        function r = ice_getConnection(self)
            v = libpointer('voidPtr');
            self.callMethod('ice_getConnection', v);
            if isNull(v)
                r = [];
            else
                r = Ice.Connection(v);
            end
        end

        function r = ice_getConnectionAsync(self)
            future = libpointer('voidPtr');
            self.callMethod('ice_getConnectionAsync', future);
            assert(~isNull(future));
            function varargout = fetch(f)
                con = libpointer('voidPtr', 0); % Output param
                Ice.Util.callMethodOnType(f, 'Ice_GetConnectionFuture', 'fetch', con);
                assert(~isNull(con));
                varargout{1} = Ice.Connection(con);
            end
            r = Ice.Future(future, 'ice_getConnection', 1, 'Ice_GetConnectionFuture', @fetch);
        end

        function r = ice_getCachedConnection(self)
            v = libpointer('voidPtr');
            self.callMethod('ice_getCachedConnection', v);
            if isNull(v)
                r = [];
            else
                r = Ice.Connection(v);
            end
        end

        function ice_flushBatchRequests(self)
            self.callMethod('ice_flushBatchRequests');
        end

        function r = ice_flushBatchRequestsAsync(self)
            future = libpointer('voidPtr');
            self.callMethod('ice_flushBatchRequestsAsync', future);
            assert(~isNull(future));
            r = Ice.Future(future, 'ice_flushBatchRequests', 0, 'Ice_SentFuture', []);
        end
    end

    methods(Access=protected)
        function checkNoResponse_(self, ok, inStream)
            if self.ice_isTwoway()
                if ok == 0
                    try
                        self.throwUserException_(inStream);
                    catch ex
                        ex.throwAsCaller();
                    end
                else
                    inStream.skipEmptyEncapsulation();
                end
            end
        end

        function os = startWriteParams_(self)
            os = self.ice_createOutputStream();
            os.startEncapsulation();
        end

        function os = startWriteParamsWithFormat_(self, format)
            os = self.ice_createOutputStream();
            os.startEncapsulationWithFormat(format);
        end

        function endWriteParams_(self, os)
            os.endEncapsulation();
        end

        function [ok, inStream] = invoke_(self, op, mode, twowayOnly, os, varargin)
            try
                % Vararg accepted for optional context argument.
                if length(varargin) > 1
                    throw(MException('Ice:ArgumentException', 'one optional argument is allowed for request context'))
                end
                if twowayOnly && ~self.ice_isTwoway()
                    throw(Ice.TwowayOnlyException('', 'invocation requires twoway proxy', op));
                end
                if ~isempty(os)
                    outStream = os.impl;
                else
                    outStream = libpointer('voidPtr'); % Null pointer for output stream
                end
                okPtr = libpointer('uint8Ptr', 0); % Output param
                inStreamPtr = libpointer('voidPtr'); % Output param
                if length(varargin) == 1
                    self.callMethod('ice_invoke', op, mode, outStream, varargin{1}, okPtr, inStreamPtr);
                else
                    self.callMethod('ice_invokeNC', op, mode, outStream, okPtr, inStreamPtr);
                end
                ok = okPtr.Value == 1;
                inStream = [];
                if ~isNull(inStreamPtr)
                    inStream = Ice.InputStream(inStreamPtr);
                end
            catch ex
                ex.throwAsCaller();
            end
        end

        function fut = invokeAsync_(self, op, mode, twowayOnly, os, numOutArgs, unmarshalFunc, varargin)
            function varargout = fetch(f)
                okPtr = libpointer('uint8Ptr', 0); % Output param
                inStreamPtr = libpointer('voidPtr', 0); % Output param
                Ice.Util.callMethodOnType(f, 'Ice_InvocationFuture', 'stream', okPtr, inStreamPtr);
                ok = okPtr.Value == 1;
                assert(~isNull(inStreamPtr));
                inStream = Ice.InputStream(inStreamPtr);
                [varargout{1:numOutArgs}] = unmarshalFunc(ok, inStream);
            end
            try
                % Vararg accepted for optional context argument.
                if length(varargin) > 1
                    throw(MException('Ice:ArgumentException', 'one optional argument is allowed for request context'))
                end
                if twowayOnly && ~self.ice_isTwoway()
                    throw(Ice.TwowayOnlyException('', 'invocation requires twoway proxy', op));
                end
                if ~isempty(os)
                    outStream = os.impl;
                else
                    outStream = libpointer('voidPtr'); % Null pointer for output stream
                end
                futPtr = libpointer('voidPtr'); % Output param
                if length(varargin) == 1
                    self.callMethod('ice_invokeAsync', op, mode, outStream, varargin{1}, futPtr);
                else
                    self.callMethod('ice_invokeAsyncNC', op, mode, outStream, futPtr);
                end
                assert(~isNull(futPtr));
                fut = Ice.Future(futPtr, op, numOutArgs, 'Ice_InvocationFuture', @fetch);
            catch ex
                ex.throwAsCaller();
            end
        end

        function throwUserException_(self, inStream, varargin) % Varargs are user exception type names
            try
                inStream.throwUserException();
            catch ex
                if isa(ex, Ice.UserException)
                    for i = 1:length(varargin)
                        if isa(ex, varargin{i})
                            ex.throwAsCaller();
                        end
                    end
                    uue = Ice.UnknownUserException('', '', ex.ice_id());
                    uue.throwAsCaller();
                else
                    ex.throwAsCaller();
                end
            end
        end

        function r = newInstance_(self, impl)
            constructor = str2func(class(self)); % Obtain the constructor for this class
            r = constructor(impl); % Call the constructor
        end

        function r = clone_(self)
            implPtr = libpointer('voidPtr'); % Output param
            self.callMethod('clone', implPtr);
            r = implPtr;
        end
    end

    methods(Access=private)
        % We don't use the functions in Ice.Util because they use the most-derived class name, whereas we always
        % want to use Ice_ObjectPrx_xxx.
        function callMethod(self, fn, varargin)
            name = strcat('Ice_ObjectPrx_', fn);
            ex = calllib('icematlab', name, self.impl, varargin{:});
            if ~isempty(ex)
                ex.throwAsCaller();
            end
        end

        function r = callMethodWithResult(self, fn, varargin)
            name = strcat('Ice_ObjectPrx_', fn);
            result = calllib('icematlab', name, self.impl, varargin{:});
            if ~isempty(result.exception)
                result.exception.throwAsCaller();
            end
            r = result.result;
        end
    end

    methods(Static,Access=protected)
        function r = read_(inStream, cls)
            p = inStream.readProxy();
            if ~isempty(p)
                constructor = str2func(cls);
                r = constructor(p.clone_());
            else
                r = [];
            end
        end

        function r = checkedCast_(p, id, cls, varargin)
            %try
                hasFacet = false;
                facet = [];
                context = {};
                if length(varargin) == 1
                    facet = varargin{1};
                    hasFacet = true;
                elseif length(varargin) == 2
                    facet = varargin{1};
                    context = {varargin{2}};
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
                        r = constructor(p.clone_());
                    else
                        r = [];
                    end
                else
                    r = p;
                end
            %catch ex
            %    ex.throwAsCaller();
            %end
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
                    r = constructor(p.clone_());
                end
            else
                r = p;
            end
        end
    end
end
