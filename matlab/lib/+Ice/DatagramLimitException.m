classdef (Sealed) DatagramLimitException < Ice.ProtocolException
    %DATAGRAMLIMITEXCEPTION The exception that is thrown when a datagram exceeds the configured send or receive
    %   buffer size, or exceeds the maximum payload size of a UDP packet (65507 bytes).

    % Copyright (c) ZeroC, Inc.
end
