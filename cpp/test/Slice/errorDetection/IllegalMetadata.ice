// Copyright (c) ZeroC, Inc.

// Error: metadata cannot be applied to nested module definitions.
// Make sure we only emit 1 error for all the metadata; not 1 error per metadata here.
["deprecated"] ["fake-metadata", "deprecated"]
module Foo::Bar {}

// Error: metadata cannot be applied to nested module definitions.
// Make sure we emit the error even for language-specific metadata.
["cpp:type:string"]
module A::B::C {}
