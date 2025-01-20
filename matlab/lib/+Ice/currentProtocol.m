function r = currentProtocol()
    % currentProtocol  Returns the supported Ice protocol version.
    %
    % Returns (Ice.ProtocolVersion) - The supported Ice protocol version.

    % Copyright (c) ZeroC, Inc.

    r = IceInternal.Util.callWithResult('Ice_currentProtocol');
end
