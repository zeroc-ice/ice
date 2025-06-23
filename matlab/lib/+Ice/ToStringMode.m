classdef ToStringMode < uint8
    %TOSTRINGMODE The output mode for xxxToString method such as identityToString and proxyToString.
    %   The actual encoding format for the string is the same for all modes: you don't need to specify an encoding
    %   format or mode when reading such a string.
    %
    %   ToStringMode Properties:
    %     Unicode - Characters with ordinal values greater than 127 are kept as-is in the resulting string.
    %     ASCII - Characters with ordinal values greater than 127 are encoded as universal character names in the resulting string.
    %     Compat - Characters with ordinal values greater than 127 are encoded as a sequence of UTF-8 bytes using octal escapes.

    % Copyright (c) ZeroC, Inc.

    enumeration
        %UNICODE Characters with ordinal values greater than 127 are kept as-is in the resulting string. Non-printable
        %   ASCII characters with ordinal values 127 and below are encoded as \\t, \\n (etc.) or \\unnnn.
        Unicode (0)

        %ASCII Characters with ordinal values greater than 127 are encoded as universal character names in the resulting
        %   string: \\unnnn for BMP characters and \\Unnnnnnnn for non-BMP characters. Non-printable ASCII characters
        %   with ordinal values 127 and below are encoded as \\t, \\n (etc.) or \\unnnn.
        ASCII (1)

        %COMPAT Characters with ordinal values greater than 127 are encoded as a sequence of UTF-8 bytes using octal
        %   escapes. Characters with ordinal values 127 and below are encoded as \\t, \\n (etc.) or an octal escape.
        %   Use this mode to generate strings compatible with Ice 3.6 and earlier.
        Compat (2)
    end
end
