- Fixed the marshaling of `float` values: a finite element of a `float` sequence or an integer outside the float range
  now raises `ValueError`, like an out-of-range floating-point `float` value. Previously, such values were silently
  marshaled as infinity.
