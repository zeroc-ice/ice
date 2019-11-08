% FacetNotExistException   Summary of FacetNotExistException
%
% This exception is raised if no facet with the given name exists,
% but at least one facet with the given identity exists.

% Copyright (c) ZeroC, Inc. All rights reserved.
% Generated from LocalException.ice by slice2matlab version 3.7.3

classdef FacetNotExistException < Ice.RequestFailedException
    methods
        function obj = FacetNotExistException(ice_exid, ice_exmsg, id, facet, operation)
            if nargin <= 2
                id = Ice.Identity();
                facet = '';
                operation = '';
            end
            if nargin == 0 || isempty(ice_exid)
                ice_exid = 'Ice:FacetNotExistException';
            end
            if nargin < 2 || isempty(ice_exmsg)
                ice_exmsg = 'Ice.FacetNotExistException';
            end
            obj = obj@Ice.RequestFailedException(ice_exid, ice_exmsg, id, facet, operation);
        end
        function id = ice_id(~)
            id = '::Ice::FacetNotExistException';
        end
    end
end
