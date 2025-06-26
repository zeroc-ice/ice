function r = intVersion()
    %INTVERSION Returns the Ice version as an integer in the form AABBCC, where AA indicates the major version, BB
    %   indicates the minor version, and CC indicates the patch level. For example, for Ice 3.8.1, the returned
    %   value is 30801.
    %
    %   Output Arguments
    %     r - The Ice version
    %       int32 scalar

    % Copyright (c) ZeroC, Inc.

    v = libpointer('int32Ptr', 0);
    IceInternal.Util.call('Ice_intVersion', v);
    r = v.Value;
end
