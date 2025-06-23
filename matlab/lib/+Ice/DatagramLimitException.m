classdef (Sealed) DatagramLimitException < Ice.ProtocolException
    %DATAGRAMLIMITEXCEPTION A datagram exceeds the configured size. This exception is raised if a datagram exceeds
    %   the configured send or receive buffer size, or exceeds the maximum payload size of a UDP packet (65507 bytes).

    % Copyright (c) ZeroC, Inc.
end
