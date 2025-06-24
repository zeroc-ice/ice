function r = stringToProtocolVersion(s)
    %STRINGTOPROTOCOLVERSION Converts a string to a protocol version.
    %
    %   Input Arguments
    %     s - The string to convert.
    %       character vector | string scalar
    %
    %   Output Arguments
    %     r - The converted protocol version.
    %       Ice.ProtocolVersion scalar

    % Copyright (c) ZeroC, Inc.

    arguments
        s (1, :) char
    end

    tokens = regexp(s, '^([0-9]+)\.([0-9]+)$', 'tokens');
    if isempty(tokens)
        error('Ice:ArgumentException', 'Expecting a version in X.Y format');
    end
    r = Ice.ProtocolVersion(str2double(tokens{1}{1}), str2double(tokens{1}{2}));
end
