% ValueFactory   Summary of ValueFactory
%
% A factory for values. Value factories are used in several
% places, such as when Ice receives a class instance and
% when Freeze restores a persistent value. Value factories
% must be implemented by the application writer and registered
% with the communicator.

% Copyright (c) ZeroC, Inc. All rights reserved.

classdef (Abstract) ValueFactory < handle
    methods(Abstract)
        result = create(obj, type)
    end
end
