function r = protocolVersionToString(v)
    % protocolVersionToString  Converts a protocol version to a string.
    %
    % Parameters:
    %   v (Ice.ProtocolVersion) - The protocol version to convert.
    %
    % Returns (char) - The protocol version as a string.

    % Copyright (c) ZeroC, Inc. All rights reserved.

    if ~isa(v, 'Ice.ProtocolVersion')
        throw(MException('Ice:ArgumentException', 'expecting an Ice.ProtocolVersion'));
    end
    r = sprintf('%d.%d', v.major, v.minor);
end
