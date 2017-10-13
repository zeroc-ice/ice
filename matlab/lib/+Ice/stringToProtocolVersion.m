function r = stringToProtocolVersion(s)
    % stringToProtocolVersion  Converts a string to a protocol version.
    %
    % Parameters:
    %   s (char) - The string to convert.
    %
    % Returns (Ice.ProtocolVersion) - The converted protocol version.

    % Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.

    tokens = regexp(s, '^([0-9]+)\.([0-9]+)$', 'tokens');
    if isempty(tokens)
        throw(MException('Ice:ArgumentException', 'expecting a version in X.Y format'));
    end
    r = Ice.ProtocolVersion(str2num(tokens{1}{1}), str2num(tokens{1}{2}));
end
