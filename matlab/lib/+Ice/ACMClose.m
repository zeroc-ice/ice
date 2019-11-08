% ACMClose   Summary of ACMClose
%
% Specifies the close semantics for Active Connection Management.
%
% ACMClose Properties:
%   CloseOff - Disables automatic connection closure.
%   CloseOnIdle - Gracefully closes a connection that has been idle for the configured timeout period.
%   CloseOnInvocation - Forcefully closes a connection that has been idle for the configured timeout period, but only if the connection has pending invocations.
%   CloseOnInvocationAndIdle - Combines the behaviors of CloseOnIdle and CloseOnInvocation.
%   CloseOnIdleForceful - Forcefully closes a connection that has been idle for the configured timeout period, regardless of whether the connection has pending invocations or dispatch.

% Copyright (c) ZeroC, Inc. All rights reserved.
% Generated from Connection.ice by slice2matlab version 3.7.3

classdef ACMClose < uint8
    enumeration
        % Disables automatic connection closure.
        CloseOff (0)
        % Gracefully closes a connection that has been idle for the configured timeout period.
        CloseOnIdle (1)
        % Forcefully closes a connection that has been idle for the configured timeout period,
        % but only if the connection has pending invocations.
        CloseOnInvocation (2)
        % Combines the behaviors of CloseOnIdle and CloseOnInvocation.
        CloseOnInvocationAndIdle (3)
        % Forcefully closes a connection that has been idle for the configured timeout period,
        % regardless of whether the connection has pending invocations or dispatch.
        CloseOnIdleForceful (4)
    end
    methods(Static)
        function r = ice_getValue(v)
            switch v
                case 0
                    r = Ice.ACMClose.CloseOff;
                case 1
                    r = Ice.ACMClose.CloseOnIdle;
                case 2
                    r = Ice.ACMClose.CloseOnInvocation;
                case 3
                    r = Ice.ACMClose.CloseOnInvocationAndIdle;
                case 4
                    r = Ice.ACMClose.CloseOnIdleForceful;
                otherwise
                    throw(Ice.MarshalException('', '', sprintf('enumerator value %d is out of range', v)));
            end
        end
    end
end
