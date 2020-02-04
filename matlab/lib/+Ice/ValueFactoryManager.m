% ValueFactoryManager   Summary of ValueFactoryManager
%
% A value factory manager maintains a collection of value factories.
% An application can supply a custom implementation during communicator
% initialization, otherwise Ice provides a default implementation.
%
% See also Ice.ValueFactory

% Copyright (c) ZeroC, Inc. All rights reserved.

classdef (Abstract) ValueFactoryManager < handle
    methods(Abstract)
        add(obj, factory, id)
        result = find(obj, id)
    end
end
