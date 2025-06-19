function r = stringToIdentity(s)
    % stringToIdentity  Converts a string to an object identity.
    %
    % Parameters:
    %   s (char) - The string to convert.
    %
    % Returns (Ice.Identity) - The converted object identity.

    % Copyright (c) ZeroC, Inc.

    arguments
        s (1, :) char
    end
    r = IceInternal.Util.callWithResult('Ice_stringToIdentity', s);
end
