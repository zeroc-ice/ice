function r = identityToString(id, mode)
    % identityToString  Converts an object identity to a string.
    %
    % Parameters:
    %   id (Ice.Identity) - The object identity to convert.
    %   mode (Ice.ToStringMode) - Optional argument specifying if and how
    %     non-printable ASCII characters are escaped in the result.
    %
    % Returns (char) - The string representation of the object identity.

    % Copyright (c) ZeroC, Inc.

    arguments
        id (1, 1) Ice.Identity
        mode (1, 1) Ice.ToStringMode = Ice.ToStringMode.Unicode
    end
    r = IceInternal.Util.callWithResult('Ice_identityToString', id, mode);
end
