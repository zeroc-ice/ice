function r = encodingVersionToString(v)
    % encodingVersionToString  Converts an encoding version to a string.
    %
    % Parameters:
    %   v (Ice.EncodingVersion) - The encoding version to convert.
    %
    % Returns (char) - The encoding version as a string.

    % Copyright (c) ZeroC, Inc.

    arguments
        v (1, 1) Ice.EncodingVersion
    end
    r = sprintf('%d.%d', v.major, v.minor);
end
