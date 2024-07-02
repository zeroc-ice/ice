% FacetNotExistException   Summary of FacetNotExistException
%
% This exception is raised if no facet with the given name exists, but at least one facet with the given identity
% exists.

% Copyright (c) ZeroC, Inc. All rights reserved.

classdef (Sealed) FacetNotExistException < Ice.RequestFailedException
end
