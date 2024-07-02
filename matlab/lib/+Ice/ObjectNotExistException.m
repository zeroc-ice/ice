% ObjectNotExistException   Summary of ObjectNotExistException
%
% This exception is raised if an object does not exist on the server, that is, if no facets with the given identity
% exist.

% Copyright (c) ZeroC, Inc. All rights reserved.

classdef (Sealed) ObjectNotExistException < Ice.RequestFailedException
    methods
        function obj = ObjectNotExistException(id, facet, operation, what)
            if nargin == 0 % default constructor
                superArgs = {};
            else
                assert(nargin == 4, 'Invalid number of arguments');
                superArgs = {id, facet, operation, 'Ice:ObjectNotExistException', what};
            end
            obj = obj@Ice.RequestFailedException(superArgs{:});
        end
    end
end
