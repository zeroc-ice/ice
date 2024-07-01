% FacetNotExistException   Summary of FacetNotExistException
%
% This exception is raised if no facet with the given name exists, but at least one facet with the given identity
% exists.

% Copyright (c) ZeroC, Inc. All rights reserved.

classdef FacetNotExistException < Ice.RequestFailedException
    methods
        function obj = FacetNotExistException(id, facet, operation, msg)
            assert(nargin == 4); % always created from the corresponding C++ exception
            obj = obj@Ice.RequestFailedException(id, facet, operation, 'Ice:FacetNotExistException', msg);
        end
    end
end
