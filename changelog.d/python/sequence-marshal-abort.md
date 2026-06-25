- Fixed Ice for Python to reliably abort request marshaling when an invalid value is supplied for a sequence
  parameter (such as a non-sequence argument), instead of continuing with a pending Python exception and sending a
  corrupt or truncated request.
