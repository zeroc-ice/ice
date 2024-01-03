% ChildInvocationObserver   Summary of ChildInvocationObserver
%
% The child invocation observer to instrument remote or collocated invocations.

% Copyright (c) ZeroC, Inc. All rights reserved.
% Generated from Instrumentation.ice by slice2matlab version 3.7.10

classdef (Abstract) ChildInvocationObserver < Ice.Instrumentation.Observer
    methods(Abstract)
        reply(obj, size)
    end
end
