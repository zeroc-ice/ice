% DatagramLimitException   Summary of DatagramLimitException
%
% A datagram exceeds the configured size. This exception is raised if a datagram exceeds the configured send or
% receive buffer size, or exceeds the maximum payload size of a UDP packet (65507 bytes).

% Copyright (c) ZeroC, Inc. All rights reserved.
% Generated from LocalException.ice by slice2matlab version 3.7.10

classdef DatagramLimitException < Ice.ProtocolException
    methods
        function obj = DatagramLimitException(errID, msg, reason)
            if nargin <= 2
                reason = '';
            end
            if nargin == 0 || isempty(errID)
                errID = 'Ice:DatagramLimitException';
            end
            if nargin < 2 || isempty(msg)
                msg = 'Ice.DatagramLimitException';
            end
            obj = obj@Ice.ProtocolException(errID, msg, reason);
        end
    end
end
