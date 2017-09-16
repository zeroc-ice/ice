%{
**********************************************************************

Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.

This copy of Ice is licensed to you under the terms described in the
ICE_LICENSE file included in this distribution.

**********************************************************************
%}
%
% Ice version 3.7.0
%

classdef Connection < IceInternal.WrapperObject
    methods
        function obj = Connection(impl, communicator)
            obj = obj@IceInternal.WrapperObject(impl);
            obj.communicator = communicator;
        end
        function close(obj, mode)
            obj.call_('close', mode);
        end
        function f = closeAsync(obj)
            future = libpointer('voidPtr');
            obj.call_('closeAsync', future);
            assert(~isNull(future));
            f = Ice.Future(future, 'close', 0, 'Ice_SimpleFuture', []);
        end
        function r = createProxy(obj, id)
            proxy = libpointer('voidPtr');
            obj.call_('createProxy', id, proxy);
            r = Ice.ObjectPrx(proxy, obj.communicator);
        end
        function r = getEndpoint(obj)
            throw(Ice.FeatureNotSupportedException('', '', 'getEndpoint'));
        end
        function flushBatchRequests(obj, compress)
            obj.call_('flushBatchRequests', compress);
        end
        function r = flushBatchRequestsAsync(obj)
            future = libpointer('voidPtr');
            obj.call_('flushBatchRequestsAsync', future);
            assert(~isNull(future));
            r = Ice.Future(future, 'flushBatchRequests', 0, 'Ice_SimpleFuture', []);
        end
        function heartbeat(obj)
            obj.call_('heartbeat');
        end
        function r = heartbeatAsync(obj)
            future = libpointer('voidPtr');
            obj.call_('heartbeatAsync', future);
            assert(~isNull(future));
            r = Ice.Future(future, 'heartbeat', 0, 'Ice_SimpleFuture', []);
        end
        function setACM(obj, timeout, close, heartbeat)
            if timeout == Ice.Unset
                timeout = [];
            end
            if close == Ice.Unset
                close = [];
            end
            if heartbeat == Ice.Unset
                heartbeat = [];
            end
            obj.call_('setACM', timeout, close, heartbeat);
        end
        function r = getACM(obj)
            r = obj.callWithResult_('getACM');
            if isempty(r.timeout)
                r.timeout = Ice.Unset;
            end
            if isempty(r.close)
                r.close = Ice.Unset;
            end
            if isempty(r.heartbeat)
                r.heartbeat = Ice.Unset;
            end
        end
        function r = type(obj)
            r = obj.callWithResult_('type');
        end
        function r = timeout(obj)
            t = libpointer('int32Ptr', 0);
            r = obj.call_('timeout', t);
            r = t.Value;
        end
        function r = toString(obj)
            r = obj.callWithResult_('toString');
        end
        function r = getInfo(obj)
            throw(Ice.FeatureNotSupportedException('', '', 'getInfo'));
        end
        function setBufferSize(obj, rcvSize, sndSize)
            obj.call_('setBufferSize', rcvSize, sndSize);
        end
        function throwException(obj)
            obj.call_('throwException');
        end
    end

    properties(Access=private)
        communicator % The communicator wrapper
    end
end
