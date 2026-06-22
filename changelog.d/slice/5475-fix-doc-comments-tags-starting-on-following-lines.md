- Fixed a bug that caused `@param` and `@throws` doc-comment tags to be incorrectly reported as "unknown tags".
  This only affected tags whose descriptions didn't start until the following line.
  