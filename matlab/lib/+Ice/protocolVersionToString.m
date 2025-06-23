function r = protocolVersionToString(v)
    %PROTOCOLVERSIONTOSTRING Converts a protocol version to a string.
    %
    %   Input Arguments
    %     v - The protocol version to convert.
    %       Ice.ProtocolVersion scalar
    %
    %   Output Arguments
    %     r - The protocol version as a string.
    %       character vector

    % Copyright (c) ZeroC, Inc.

    arguments
        v (1, 1) Ice.ProtocolVersion
    end
    r = sprintf('%d.%d', v.major, v.minor);
end
