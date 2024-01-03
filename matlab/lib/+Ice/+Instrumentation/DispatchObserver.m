% DispatchObserver   Summary of DispatchObserver
%
% The dispatch observer to instrument servant dispatch.

% Copyright (c) ZeroC, Inc. All rights reserved.
% Generated from Instrumentation.ice by slice2matlab version 3.7.10

classdef (Abstract) DispatchObserver < Ice.Instrumentation.Observer
    methods(Abstract)
        userException(obj)
        reply(obj, size)
    end
end
