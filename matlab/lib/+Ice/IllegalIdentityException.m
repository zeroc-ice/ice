% IllegalIdentityException   Summary of IllegalIdentityException
%
% This exception is raised if an illegal identity is encountered.
%
% IllegalIdentityException Properties:
%   id - The illegal identity.

% Copyright (c) ZeroC, Inc. All rights reserved.
% Generated from LocalException.ice by slice2matlab version 3.7.3

classdef IllegalIdentityException < Ice.LocalException
    properties
        % id - The illegal identity.
        id Ice.Identity
    end
    methods
        function obj = IllegalIdentityException(ice_exid, ice_exmsg, id)
            if nargin <= 2
                id = Ice.Identity();
            end
            if nargin == 0 || isempty(ice_exid)
                ice_exid = 'Ice:IllegalIdentityException';
            end
            if nargin < 2 || isempty(ice_exmsg)
                ice_exmsg = 'Ice.IllegalIdentityException';
            end
            obj = obj@Ice.LocalException(ice_exid, ice_exmsg);
            obj.id = id;
        end
        function id = ice_id(~)
            id = '::Ice::IllegalIdentityException';
        end
    end
end
