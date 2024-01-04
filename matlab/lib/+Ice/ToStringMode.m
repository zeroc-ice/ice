% ToStringMode   Summary of ToStringMode
%
% The central object in Ice. One or more communicators can be instantiated for an Ice application. Communicator
% instantiation is language-specific, and not specified in Slice code.
%
% ToStringMode Properties:
%   Unicode - Characters with ordinal values greater than 127 are kept as-is in the resulting string.
%   ASCII - Characters with ordinal values greater than 127 are encoded as universal character names in the resulting string: \\unnnn for BMP characters and \\Unnnnnnnn for non-BMP characters.
%   Compat - Characters with ordinal values greater than 127 are encoded as a sequence of UTF-8 bytes using octal escapes.
%
% See also Ice.Logger, Ice.ObjectAdapter, Ice.Properties, Ice.ValueFactory, The output mode for xxxToString method such as identityToString and proxyToString. The actual encoding format for, the string is the same for all modes: you don't need to specify an encoding format or mode when reading such a, string.

% Copyright (c) ZeroC, Inc. All rights reserved.
% Generated from Communicator.ice by slice2matlab version 3.7.10

classdef ToStringMode < uint8
    enumeration
        % Characters with ordinal values greater than 127 are kept as-is in the resulting string. Non-printable ASCII
        % characters with ordinal values 127 and below are encoded as \\t, \\n (etc.) or \\unnnn.
        Unicode (0)
        % Characters with ordinal values greater than 127 are encoded as universal character names in the resulting
        % string: \\unnnn for BMP characters and \\Unnnnnnnn for non-BMP characters. Non-printable ASCII characters
        % with ordinal values 127 and below are encoded as \\t, \\n (etc.) or \\unnnn.
        ASCII (1)
        % Characters with ordinal values greater than 127 are encoded as a sequence of UTF-8 bytes using octal escapes.
        % Characters with ordinal values 127 and below are encoded as \\t, \\n (etc.) or an octal escape. Use this mode
        % to generate strings compatible with Ice 3.6 and earlier.
        Compat (2)
    end
    methods(Static)
        function r = ice_getValue(v)
            switch v
                case 0
                    r = Ice.ToStringMode.Unicode;
                case 1
                    r = Ice.ToStringMode.ASCII;
                case 2
                    r = Ice.ToStringMode.Compat;
                otherwise
                    throw(Ice.MarshalException('', '', sprintf('enumerator value %d is out of range', v)));
            end
        end
    end
end
