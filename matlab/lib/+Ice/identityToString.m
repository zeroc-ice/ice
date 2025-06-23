function r = identityToString(id, mode)
    %IDENTITYTOSTRING Converts an object identity to a string.
    %
    %   Input Arguments
    %     id - The object identity to convert.
    %       Ice.Identity scalar
    %     mode - Specifies how non-printable ASCII characters are escaped in the result. Defaults to
    %       Ice.ToStringMode.Unicode.
    %       Ice.ToStringMode scalar
    %
    %   Output Arguments
    %     r - The string representation of the object identity.
    %       character vector

    % Copyright (c) ZeroC, Inc.

    arguments
        id (1, 1) Ice.Identity
        mode (1, 1) Ice.ToStringMode = Ice.ToStringMode.Unicode
    end
    r = IceInternal.Util.callWithResult('Ice_identityToString', id, mode);
end
