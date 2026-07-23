function r = stringVersion()
    %STRINGVERSION Returns the Ice version in the form A.B.C, where A indicates the major version, B indicates the
    %   minor version, and C indicates the patch level.
    %
    %   Output Arguments
    %     r - The Ice version.
    %       character vector

    % Copyright (c) ZeroC, Inc.

    r = IceInternal.Util.callWithResult('Ice_stringVersion');
end
