% ConnectionClose   Summary of ConnectionClose
%
% Determines the behavior when manually closing a connection.
%
% ConnectionClose Properties:
%   Forcefully - Close the connection immediately without sending a close connection protocol message to the peer and waiting for the peer to acknowledge it.
%   Gracefully - Close the connection by notifying the peer but do not wait for pending outgoing invocations to complete.
%   GracefullyWithWait - Wait for all pending invocations to complete before closing the connection.

% Copyright (c) ZeroC, Inc. All rights reserved.

classdef ConnectionClose < uint8
    enumeration
        % Close the connection immediately without sending a close connection protocol message to the peer and waiting
        % for the peer to acknowledge it.
        Forcefully (0)
        % Close the connection by notifying the peer but do not wait for pending outgoing invocations to complete. On the
        % server side, the connection will not be closed until all incoming invocations have completed.
        Gracefully (1)
        % Wait for all pending invocations to complete before closing the connection.
        GracefullyWithWait (2)
    end
end
