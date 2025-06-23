% Copyright (c) ZeroC, Inc.

classdef (Sealed) FacetNotExistException < Ice.RequestFailedException
    %FACETNOTEXISTEXCEPTION This exception is raised if no facet with the given name exists, but at least one facet
    %   with the given identity exists.
end
