% NoEndpointException   Summary of NoEndpointException
%
% This exception is raised if no suitable endpoint is available.
%
% NoEndpointException Properties:
%   proxy - The stringified proxy for which no suitable endpoint is available.

% Copyright (c) ZeroC, Inc. All rights reserved.
% Generated from LocalException.ice by slice2matlab version 3.7.3

classdef NoEndpointException < Ice.LocalException
    properties
        % proxy - The stringified proxy for which no suitable endpoint is
        % available.
        proxy char
    end
    methods
        function obj = NoEndpointException(ice_exid, ice_exmsg, proxy)
            if nargin <= 2
                proxy = '';
            end
            if nargin == 0 || isempty(ice_exid)
                ice_exid = 'Ice:NoEndpointException';
            end
            if nargin < 2 || isempty(ice_exmsg)
                ice_exmsg = 'Ice.NoEndpointException';
            end
            obj = obj@Ice.LocalException(ice_exid, ice_exmsg);
            obj.proxy = proxy;
        end
        function id = ice_id(~)
            id = '::Ice::NoEndpointException';
        end
    end
end
