% DatagramLimitException   Summary of DatagramLimitException
%
% A datagram exceeds the configured size.
%
% This exception is raised if a datagram exceeds the configured send or receive buffer
% size, or exceeds the maximum payload size of a UDP packet (65507 bytes).

% Copyright (c) ZeroC, Inc. All rights reserved.
% Generated from LocalException.ice by slice2matlab version 3.7.3

classdef DatagramLimitException < Ice.ProtocolException
    methods
        function obj = DatagramLimitException(ice_exid, ice_exmsg, reason)
            if nargin <= 2
                reason = '';
            end
            if nargin == 0 || isempty(ice_exid)
                ice_exid = 'Ice:DatagramLimitException';
            end
            if nargin < 2 || isempty(ice_exmsg)
                ice_exmsg = 'Ice.DatagramLimitException';
            end
            obj = obj@Ice.ProtocolException(ice_exid, ice_exmsg, reason);
        end
        function id = ice_id(~)
            id = '::Ice::DatagramLimitException';
        end
    end
end
