function r = encodingVersionToString(v)
    %ENCODINGVERSIONTOSTRING Converts an encoding version to a string.
    %
    %   Input Arguments
    %     v - The encoding version to convert.
    %       Ice.EncodingVersion scalar
    %
    %   Output Arguments
    %     r - The encoding version as a string.
    %       character vector

    % Copyright (c) ZeroC, Inc.

    arguments
        v (1, 1) Ice.EncodingVersion
    end
    r = sprintf('%d.%d', v.major, v.minor);
end
