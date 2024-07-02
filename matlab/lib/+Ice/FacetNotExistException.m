% FacetNotExistException   Summary of FacetNotExistException
%
% This exception is raised if no facet with the given name exists, but at least one facet with the given identity
% exists.

% Copyright (c) ZeroC, Inc. All rights reserved.

classdef (Sealed) FacetNotExistException < Ice.RequestFailedException
    methods
        function obj = FacetNotExistException(id, facet, operation, what)
            if nargin == 0 % default constructor
                superArgs = {};
            else
                assert(nargin == 4, 'Invalid number of arguments');
                superArgs = {id, facet, operation, 'Ice:FacetNotExistException', what};
            end
            obj = obj@Ice.RequestFailedException(superArgs{:});
        end
    end
end
