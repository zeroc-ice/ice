function r = stringToEncodingVersion(s)
    % stringToEncodingVersion  Converts a string to an encoding version.
    %
    % Parameters:
    %   s (char) - The string to convert.
    %
    % Returns (Ice.EncodingVersion) - The converted encoding version.

    % Copyright (c) ZeroC, Inc.

    tokens = regexp(s, '^([0-9]+)\.([0-9]+)$', 'tokens');
    if isempty(tokens)
        error('Ice:ArgumentException', 'Expecting a version in X.Y format');
    end
    r = Ice.EncodingVersion(str2double(tokens{1}{1}), str2double(tokens{1}{2}));
end
