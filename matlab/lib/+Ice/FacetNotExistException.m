% FacetNotExistException   Summary of FacetNotExistException
%
% This exception is raised if no facet with the given name exists, but at least one facet with the given identity
% exists.

% Copyright (c) ZeroC, Inc. All rights reserved.
% Generated from LocalException.ice by slice2matlab version 3.7.10

classdef FacetNotExistException < Ice.RequestFailedException
    methods
        function obj = FacetNotExistException(errID, msg, id, facet, operation)
            if nargin <= 2
                id = Ice.Identity();
                facet = '';
                operation = '';
            end
            if nargin == 0 || isempty(errID)
                errID = 'Ice:FacetNotExistException';
            end
            if nargin < 2 || isempty(msg)
                msg = 'Ice.FacetNotExistException';
            end
            obj = obj@Ice.RequestFailedException(errID, msg, id, facet, operation);
        end
    end
end
