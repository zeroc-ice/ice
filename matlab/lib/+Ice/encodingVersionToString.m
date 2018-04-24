function r = encodingVersionToString(v)
    % encodingVersionToString  Converts an encoding version to a string.
    %
    % Parameters:
    %   v (Ice.EncodingVersion) - The encoding version to convert.
    %
    % Returns (char) - The encoding version as a string.

    % Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.

    if ~isa(v, 'Ice.EncodingVersion')
        throw(MException('Ice:ArgumentException', 'expecting an Ice.EncodingVersion'));
    end
    r = sprintf('%d.%d', v.major, v.minor);
end
