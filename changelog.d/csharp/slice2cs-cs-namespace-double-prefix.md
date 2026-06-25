- Fixed a bug in `slice2cs` handling of `cs:namespace`: a nested module received the namespace prefix twice
  (e.g. `Foo.A.Foo.B` instead of `Foo.A.B`), producing C# that did not compile.
