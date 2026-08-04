- Fixed the marshaling of `float` sequences: a finite element value outside the float range now raises `ValueError`,
  same as the marshaling of an individual `float` value. Previously, such elements were silently marshaled as infinity.
