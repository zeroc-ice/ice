function r = stringToEncodingVersion(s)
    % stringToEncodingVersion  Converts a string to an encoding version.
    %
    % Parameters:
    %   s (char) - The string to convert.
    %
    % Returns (Ice.EncodingVersion) - The converted encoding version.

    % Copyright (c) ZeroC, Inc. All rights reserved.

    tokens = regexp(s, '^([0-9]+)\.([0-9]+)$', 'tokens');
    if isempty(tokens)
        throw(MException('Ice:ArgumentException', 'expecting a version in X.Y format'));
    end
    r = Ice.EncodingVersion(str2num(tokens{1}{1}), str2num(tokens{1}{2}));
end
