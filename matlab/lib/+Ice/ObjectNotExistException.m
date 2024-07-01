% ObjectNotExistException   Summary of ObjectNotExistException
%
% This exception is raised if an object does not exist on the server, that is, if no facets with the given identity
% exist.

% Copyright (c) ZeroC, Inc. All rights reserved.

classdef ObjectNotExistException < Ice.RequestFailedException
    methods
        function obj = ObjectNotExistException(id, facet, operation, msg)
            assert(nargin == 4); % always created from the corresponding C++ exception
            obj = obj@Ice.RequestFailedException(id, facet, operation, 'Ice:ObjectNotExistException', msg);
        end
    end
end
