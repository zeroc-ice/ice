function r = stringToIdentity(s)
    % stringToIdentity  Converts a string to an object identity.
    %
    % Parameters:
    %   s (char) - The string to convert.
    %
    % Returns (Ice.Identity) - The converted object identity.

    % Copyright (c) ZeroC, Inc.

    r = IceInternal.Util.callWithResult('Ice_stringToIdentity', s);
end
