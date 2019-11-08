% RequestFailedException   Summary of RequestFailedException
%
% This exception is raised if a request failed. This exception, and
% all exceptions derived from RequestFailedException, are
% transmitted by the Ice protocol, even though they are declared
% local.
%
% RequestFailedException Properties:
%   id - The identity of the Ice Object to which the request was sent.
%   facet - The facet to which the request was sent.
%   operation - The operation name of the request.

% Copyright (c) ZeroC, Inc. All rights reserved.
% Generated from LocalException.ice by slice2matlab version 3.7.3

classdef RequestFailedException < Ice.LocalException
    properties
        % id - The identity of the Ice Object to which the request was sent.
        id Ice.Identity
        % facet - The facet to which the request was sent.
        facet char
        % operation - The operation name of the request.
        operation char
    end
    methods
        function obj = RequestFailedException(ice_exid, ice_exmsg, id, facet, operation)
            if nargin <= 2
                id = Ice.Identity();
                facet = '';
                operation = '';
            end
            if nargin == 0 || isempty(ice_exid)
                ice_exid = 'Ice:RequestFailedException';
            end
            if nargin < 2 || isempty(ice_exmsg)
                ice_exmsg = 'Ice.RequestFailedException';
            end
            obj = obj@Ice.LocalException(ice_exid, ice_exmsg);
            obj.id = id;
            obj.facet = facet;
            obj.operation = operation;
        end
        function id = ice_id(~)
            id = '::Ice::RequestFailedException';
        end
    end
end
