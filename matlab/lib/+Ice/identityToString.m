function r = identityToString(id, varargin)
    % identityToString  Converts an object identity to a string.
    %
    % Parameters:
    %   id (Ice.Identity) - The object identity to convert.
    %   mode (Ice.ToStringMode) - Optional argument specifying if and how
    %     non-printable ASCII characters are escaped in the result.
    %
    % Returns (char) - The string representation of the object identity.

    % Copyright (c) ZeroC, Inc. All rights reserved.

    if length(varargin) == 1
        mode = varargin{1};
    elseif length(varargin) == 0
        mode = Ice.ToStringMode.Unicode;
    elseif length(varargin) > 2
        throw(MException('Ice:ArgumentException', 'too many arguments'));
    end
    r = IceInternal.Util.callWithResult('Ice_identityToString', id, int32(mode));
end
