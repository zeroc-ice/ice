% ConnectionState   Summary of ConnectionState
%
% The state of an Ice connection.
%
% ConnectionState Properties:
%   ConnectionStateValidating - The connection is being validated.
%   ConnectionStateHolding - The connection is holding the reception of new messages.
%   ConnectionStateActive - The connection is active and can send and receive messages.
%   ConnectionStateClosing - The connection is being gracefully shutdown and waits for the peer to close its end of the connection.
%   ConnectionStateClosed - The connection is closed and waits for potential dispatch to be finished before being destroyed and detached from the observer.

% Copyright (c) ZeroC, Inc. All rights reserved.
% Generated from Instrumentation.ice by slice2matlab version 3.7.10

classdef ConnectionState < uint8
    enumeration
        % The connection is being validated.
        ConnectionStateValidating (0)
        % The connection is holding the reception of new messages.
        ConnectionStateHolding (1)
        % The connection is active and can send and receive messages.
        ConnectionStateActive (2)
        % The connection is being gracefully shutdown and waits for the peer to close its end of the connection.
        ConnectionStateClosing (3)
        % The connection is closed and waits for potential dispatch to be finished before being destroyed and detached
        % from the observer.
        ConnectionStateClosed (4)
    end
    methods(Static)
        function r = ice_getValue(v)
            switch v
                case 0
                    r = Ice.Instrumentation.ConnectionState.ConnectionStateValidating;
                case 1
                    r = Ice.Instrumentation.ConnectionState.ConnectionStateHolding;
                case 2
                    r = Ice.Instrumentation.ConnectionState.ConnectionStateActive;
                case 3
                    r = Ice.Instrumentation.ConnectionState.ConnectionStateClosing;
                case 4
                    r = Ice.Instrumentation.ConnectionState.ConnectionStateClosed;
                otherwise
                    throw(Ice.MarshalException('', '', sprintf('enumerator value %d is out of range', v)));
            end
        end
    end
end
