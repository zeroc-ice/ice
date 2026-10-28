- The Slice tools in the `ZeroC.Ice.Cpp` NuGet package no longer add the `.ice` files under the project directory as
  `SliceCompile` items when a C++ project sets `EnableDefaultItems` to `true`, and the `EnableDefaultSliceCompileItems`
  property no longer has any effect. List the Slice files to compile as `SliceCompile` items, as Visual Studio does when
  you add a `.ice` file to the project.
