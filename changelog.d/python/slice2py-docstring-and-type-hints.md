- Fixed `slice2py` to escape doc-comment text when emitting Python docstrings, so that comments containing a
  triple-quote sequence or a backslash escape (such as `\u`) no longer produce invalid generated Python.
- Fixed `slice2py` to honor parameter metadata (such as `["python:numpy.ndarray"]`) in the type hints of generated
  proxy methods, matching the servant side, instead of dropping the metadata and leaving the added import unused.
