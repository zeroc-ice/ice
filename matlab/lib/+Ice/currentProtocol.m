function r = currentProtocol()
    % currentProtocol  Returns the supported Ice protocol version.
    %
    % Returns (Ice.ProtocolVersion) - The supported Ice protocol version.

    % Copyright (c) 2003-present ZeroC, Inc. All rights reserved.

    r = IceInternal.Util.callWithResult('Ice_currentProtocol');
end
