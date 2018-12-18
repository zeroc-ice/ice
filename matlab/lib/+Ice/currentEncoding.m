function r = currentEncoding()
    % currentEncoding  Returns the supported Ice encoding version.
    %
    % Returns (Ice.EncodingVersion) - The Ice encoding version.

    % Copyright (c) 2003-present ZeroC, Inc. All rights reserved.

    r = IceInternal.Util.callWithResult('Ice_currentEncoding');
end
