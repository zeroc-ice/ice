function r = Unset()
    %UNSET Returns the sentinel value used to indicate an unset optional value.
    %   Compare a value against Ice.Unset with == or ~= to determine whether an optional value is set.
    %
    %   Output Arguments
    %     r - The unset sentinel value.

    % Copyright (c) ZeroC, Inc.

    r = IceInternal.UnsetI.Instance;
end
