function r = Unset()
    %UNSET This function returns a singleton instance that we use as a sentinel value to indicate an unset optional
    %   value.
    %
    %   Output Arguments
    %     r - The unset sentinel value.

    % Copyright (c) ZeroC, Inc.

    r = IceInternal.UnsetI.Instance;
end
