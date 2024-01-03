% InvocationObserver   Summary of InvocationObserver
%
% The invocation observer to instrument invocations on proxies. A proxy invocation can either result in a collocated
% or remote invocation. If it results in a remote invocation, a sub-observer is requested for the remote invocation.

% Copyright (c) ZeroC, Inc. All rights reserved.
% Generated from Instrumentation.ice by slice2matlab version 3.7.10

classdef (Abstract) InvocationObserver < Ice.Instrumentation.Observer
    methods(Abstract)
        retried(obj)
        userException(obj)
        result = getRemoteObserver(obj, con, endpt, requestId, size)
        result = getCollocatedObserver(obj, adapter, requestId, size)
    end
end
