% Observer   Summary of Observer
%
% The object observer interface used by instrumented objects to notify the observer of their existence.

% Copyright (c) ZeroC, Inc. All rights reserved.
% Generated from Instrumentation.ice by slice2matlab version 3.7.10

classdef (Abstract) Observer < handle
    methods(Abstract)
        attach(obj)
        detach(obj)
        failed(obj, exceptionName)
    end
end
