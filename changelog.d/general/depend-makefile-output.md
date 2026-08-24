- The Makefile dependency output of the Slice compilers (`--depend`) now escapes file names holding
  Make-significant characters: spaces, `#`, and `$`.
- `--depend --depend-file FILE` now writes a rule for every Slice file passed to the compiler; previously the
  dependency file only kept the last Slice file's rule.
