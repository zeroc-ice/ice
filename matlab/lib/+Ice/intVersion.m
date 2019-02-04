function r = intVersion()
    % intVersion  Returns the Ice version as an integer in the form
    %   A.BB.CC, where A indicates the major version, BB indicates the
    %   minor version, and CC indicates the patch level. For example,
    %   for Ice 3.3.1, the returned value is 30301.
    %
    % Returns (int32) - The Ice version.

    % Copyright (c) ZeroC, Inc. All rights reserved.

    v = libpointer('int32Ptr', 0);
    IceInternal.Util.call('Ice_intVersion', v);
    r = v.Value;
end
