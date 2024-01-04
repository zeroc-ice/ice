% CFNetworkException   Summary of CFNetworkException
%
% This exception indicates CFNetwork errors.
%
% CFNetworkException Properties:
%   domain - The domain of the error.

% Copyright (c) ZeroC, Inc. All rights reserved.
% Generated from LocalException.ice by slice2matlab version 3.7.10

classdef CFNetworkException < Ice.SocketException
    properties
        % domain - The domain of the error.
        domain char
    end
    methods
        function obj = CFNetworkException(ice_exid, ice_exmsg, error, domain)
            if nargin <= 2
                error = 0;
                domain = '';
            end
            if nargin == 0 || isempty(ice_exid)
                ice_exid = 'Ice:CFNetworkException';
            end
            if nargin < 2 || isempty(ice_exmsg)
                ice_exmsg = 'Ice.CFNetworkException';
            end
            obj = obj@Ice.SocketException(ice_exid, ice_exmsg, error);
            obj.domain = domain;
        end
        function id = ice_id(~)
            id = '::Ice::CFNetworkException';
        end
    end
end
