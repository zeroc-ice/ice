function r = currentProtocolEncoding()
    % currentProtocolEncoding  Returns the encoding version for the Ice protocol.
    %
    % Returns (Ice.EncodingVersion) - The encoding version for the Ice protocol.

    % Copyright (c) 2003-present ZeroC, Inc. All rights reserved.

    r = IceInternal.Util.callWithResult('Ice_currentProtocolEncoding');
end
