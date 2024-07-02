% OperationNotExistException   Summary of OperationNotExistException
%
% This exception is raised if an operation for a given object does not exist on the server. Typically this is caused
% by either the client or the server using an outdated Slice specification.

% Copyright (c) ZeroC, Inc. All rights reserved.

classdef (Sealed) OperationNotExistException < Ice.RequestFailedException
    methods
        function obj = OperationNotExistException(id, facet, operation, what)
            if nargin == 0 % default constructor
                superArgs = {};
            else
                assert(nargin == 4, 'Invalid number of arguments');
                superArgs = {id, facet, operation, 'Ice:OperationNotExistException', what};
            end
            obj = obj@Ice.RequestFailedException(superArgs{:});
        end
    end
end
