% ObjectNotExistException   Summary of ObjectNotExistException
%
% This exception is raised if an object does not exist on the server,
% that is, if no facets with the given identity exist.

% Copyright (c) ZeroC, Inc. All rights reserved.
% Generated from LocalException.ice by slice2matlab version 3.7.3

classdef ObjectNotExistException < Ice.RequestFailedException
    methods
        function obj = ObjectNotExistException(ice_exid, ice_exmsg, id, facet, operation)
            if nargin <= 2
                id = Ice.Identity();
                facet = '';
                operation = '';
            end
            if nargin == 0 || isempty(ice_exid)
                ice_exid = 'Ice:ObjectNotExistException';
            end
            if nargin < 2 || isempty(ice_exmsg)
                ice_exmsg = 'Ice.ObjectNotExistException';
            end
            obj = obj@Ice.RequestFailedException(ice_exid, ice_exmsg, id, facet, operation);
        end
        function id = ice_id(~)
            id = '::Ice::ObjectNotExistException';
        end
    end
end
