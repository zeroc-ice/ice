- Fixed `slice2java` to generate a `@Deprecated` annotation based on `["deprecated"]` metadata, rather than from the
  `@deprecated` doc-comment tag. Now the `@deprecated` doc-comment tag only affects the generated doc-comment.
  