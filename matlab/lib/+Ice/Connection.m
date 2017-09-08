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
        function obj = Connection(impl)
            obj = obj@IceInternal.WrapperObject(impl);
        end
        function close(obj, mode)
            Ice.Util.callMethod(obj, 'close', mode);
        end
        function r = createProxy(obj, id)
            proxy = libpointer('voidPtr');
            Ice.Util.callMethod(obj, 'createProxy', id, proxy);
            r = Ice.ObjectPrx(proxy);
        end
        function r = getEndpoint(obj)
            throw(Ice.FeatureNotSupportedException('', '', 'getEndpoint'));
        end
        function flushBatchRequests(obj, compress)
            Ice.Util.callMethod(obj, 'flushBatchRequests', compress);
        end
        function r = flushBatchRequestsAsync(obj)
            future = libpointer('voidPtr');
            Ice.Util.callMethod(obj, 'flushBatchRequestsAsync', future);
            assert(~isNull(future));
            r = Ice.Future(future, 'flushBatchRequests', 0, 'Ice_SentFuture', []);
        end
        function heartbeat(obj)
            Ice.Util.callMethod(obj, 'heartbeat');
        end
        function r = heartbeatAsync(obj)
            future = libpointer('voidPtr');
            Ice.Util.callMethod(obj, 'heartbeatAsync', future);
            assert(~isNull(future));
            r = Ice.Future(future, 'heartbeat', 0, 'Ice_SentFuture', []);
        end
        function setACM(obj, timeout, close, heartbeat)
            Ice.Util.callMethod(obj, 'setACM', timeout, close, heartbeat);
        end
        function r = getACM(obj)
            r = Ice.Util.callMethodWithResult(obj, 'getACM');
        end
        function r = type(obj)
            r = Ice.Util.callMethodWithResult(obj, 'type');
        end
        function r = timeout(obj)
            t = libpointer('int32Ptr', 0);
            r = Ice.Util.callMethod(obj, 'timeout', t);
            r = t.Value;
        end
        function r = toString(obj)
            r = Ice.Util.callMethodWithResult(obj, 'toString');
        end
        function r = getInfo(obj)
            throw(Ice.FeatureNotSupportedException('', '', 'getInfo'));
        end
        function setBufferSize(obj, rcvSize, sndSize)
            Ice.Util.callMethod(obj, 'setBufferSize', rcvSize, sndSize);
        end
        function throwException(obj)
            Ice.Util.callMethod(obj, 'throwException');
        end
    end
end
