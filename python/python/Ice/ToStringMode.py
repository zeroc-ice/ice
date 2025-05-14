# Copyright (c) ZeroC, Inc.

from .EnumBase import EnumBase

class ToStringMode(EnumBase):
    """
    The output mode for xxxToString method such as identityToString and proxyToString. The actual encoding format for
    the string is the same for all modes: you don't need to specify an encoding format or mode when reading such a
    string.

    Enumerators:

    - Unicode:  Characters with ordinal values greater than 127 are kept as-is in the resulting string. Non-printable ASCII
      characters with ordinal values 127 and below are encoded as \\t, \\n (etc.) or \\unnnn.

    - ASCII:  Characters with ordinal values greater than 127 are encoded as universal character names in the resulting
      string \\unnnn for BMP characters and \\Unnnnnnnn for non-BMP characters. Non-printable ASCII characters
      with ordinal values 127 and below are encoded as \\t, \\n (etc.) or \\unnnn.

    - Compat: Characters with ordinal values greater than 127 are encoded as a sequence of UTF-8 bytes using octal escapes.
      Characters with ordinal values 127 and below are encoded as \\t, \\n (etc.) or an octal escape. Use this mode
      to generate strings compatible with Ice 3.6 and earlier.
    """

    def __init__(self, _n, _v):
        EnumBase.__init__(self, _n, _v)

    def valueOf(self, value):
        """
        Get the enumerator corresponding to the given value.

        Parameters
        ----------
        value : int
            The enumerator's value.

        Returns
        -------
        ToStringMode or None
            The enumerator corresponding to the given value, or None if no such enumerator exists.
        """
        return self._enumerators[value] if value in self._enumerators else None

    valueOf = classmethod(valueOf)

ToStringMode.Unicode = ToStringMode("Unicode", 0)
ToStringMode.ASCII = ToStringMode("ASCII", 1)
ToStringMode.Compat = ToStringMode("Compat", 2)
ToStringMode._enumerators = {
    0: ToStringMode.Unicode,
    1: ToStringMode.ASCII,
    2: ToStringMode.Compat,
}
