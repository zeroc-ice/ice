function r = stringToIdentity(s)
    %STRINGTOIDENTITY Converts a string to an object identity.
    %
    %   Input Arguments
    %     s - The string to convert.
    %       character vector | string scalar
    %
    %   Output Arguments
    %     r - The converted object identity.
    %       Ice.Identity scalar

    % Copyright (c) ZeroC, Inc.

    arguments
        s (1, :) char
    end
    r = IceInternal.Util.callWithResult('Ice_stringToIdentity', s);
end
