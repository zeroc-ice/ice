function r = stringVersion()
    % stringVersion  Returns the Ice version in the form A.B.C, where A
    %   indicates the major version, B indicates the minor version, and
    %   C indicates the patch level.
    %
    % Returns (char) - The Ice version.

    % Copyright (c) ZeroC, Inc. All rights reserved.

    r = IceInternal.Util.callWithResult('Ice_stringVersion');
end
