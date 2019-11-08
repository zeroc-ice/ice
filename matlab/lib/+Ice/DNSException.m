% DNSException   Summary of DNSException
%
% This exception indicates a DNS problem. For details on the cause,
% DNSException.error should be inspected.
%
% DNSException Properties:
%   error - The error number describing the DNS problem.
%   host - The host name that could not be resolved.

% Copyright (c) ZeroC, Inc. All rights reserved.
% Generated from LocalException.ice by slice2matlab version 3.7.3

classdef DNSException < Ice.LocalException
    properties
        % error - The error number describing the DNS problem. For C++ and Unix,
        % this is equivalent to h_errno. For C++ and
        % Windows, this is the value returned by
        % WSAGetLastError().
        error int32
        % host - The host name that could not be resolved.
        host char
    end
    methods
        function obj = DNSException(ice_exid, ice_exmsg, error, host)
            if nargin <= 2
                error = 0;
                host = '';
            end
            if nargin == 0 || isempty(ice_exid)
                ice_exid = 'Ice:DNSException';
            end
            if nargin < 2 || isempty(ice_exmsg)
                ice_exmsg = 'Ice.DNSException';
            end
            obj = obj@Ice.LocalException(ice_exid, ice_exmsg);
            obj.error = error;
            obj.host = host;
        end
        function id = ice_id(~)
            id = '::Ice::DNSException';
        end
    end
end
