// Copyright (c) ZeroC, Inc.

// Case where the module chain is all good.
module All::Good::Here
{
    module Okay {}
    module Helper {}            // Error: 'Helper' suffix is reserved

    module Okay::Helper {}      // Error: 'Helper' suffix is reserved
}

// Case where the module chain is broken.
module An::__Iceberg::Ahead      // Error: illegal leading underscore
{
    module Okay {}
    module Helper {}            // Error: 'Helper' suffix is reserved

    module Okay::Helper {}      // Error: 'Helper' suffix is reserved
}

// Ensure that each module segment is individually checked for errors.
module APtr::Okay::BPrx::Fine::CHelper {}

// Ensure that modules cannot start with a '::' character.
module ::No::Leading::Helper {}
