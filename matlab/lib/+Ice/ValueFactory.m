% ValueFactory   Summary of ValueFactory
%
% A factory for class instances.

% Copyright (c) ZeroC, Inc. All rights reserved.

classdef (Abstract) ValueFactory < handle
    methods(Abstract)
        result = create(obj, type)
    end
end
