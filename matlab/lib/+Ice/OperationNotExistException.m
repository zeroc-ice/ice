% OperationNotExistException   Summary of OperationNotExistException
%
% This exception is raised if an operation for a given object does not exist on the server. Typically this is caused
% by either the client or the server using an outdated Slice specification.

% Copyright (c) ZeroC, Inc. All rights reserved.

classdef OperationNotExistException < Ice.RequestFailedException
    methods
        function obj = OperationNotExistException(id, facet, operation, msg)
            assert(nargin == 4); % always created from the corresponding C++ exception
            obj = obj@Ice.RequestFailedException(id, facet, operation, 'Ice:OperationNotExistException', msg);
        end
    end
end
