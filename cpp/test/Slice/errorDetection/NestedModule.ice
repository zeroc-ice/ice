//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

// Case where the module chain is all good.
module All::Good::Here
{
    module Okay {}
    module Holder {}            // Error: 'Holder' suffix is reserved

    module Okay::Holder {}      // Error: 'Holder' suffix is reserved
}

// Case where the module chain is broken.
module A::__Iceberg::Ahead      // Error: illegal leading underscore
{
    module Okay {}
    module Holder {}            // Error: 'Holder' suffix is reserved

    module Okay::Holder {}      // Error: 'Holder' suffix is reserved
}

// Ensure that each module segment is individually checked for errors.
module APtr::Okay::BPrx::Fine::CHelper {}
