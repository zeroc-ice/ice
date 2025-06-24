function r = stringToEncodingVersion(s)
    %STRINGTOENCODINGVERSION Converts a string to an encoding version.
    %
    %   Input Arguments
    %     s - The string to convert.
    %       character vector | string scalar
    %
    %   Output Arguments
    %     r - The converted encoding version
    %       Ice.EncodingVersion scalar.

    % Copyright (c) ZeroC, Inc.

    tokens = regexp(s, '^([0-9]+)\.([0-9]+)$', 'tokens');
    if isempty(tokens)
        error('Ice:ArgumentException', 'Expecting a version in X.Y format');
    end
    r = Ice.EncodingVersion(str2double(tokens{1}{1}), str2double(tokens{1}{2}));
end
