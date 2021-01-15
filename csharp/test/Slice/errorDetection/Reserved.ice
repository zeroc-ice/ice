//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <include/IcePrefix.ice>        // No warnings should be caused by this file

module OK
{
const long PrxA = 0;
const long APrxA = 0;
const long prxB = 0;
const long Bprx = 0;
const long prx = 0;
const long PtrA = 0;
const long HelperA = 0;
const long aIce = 0;
}

module errors
{
const long Prx = 0;
const long abcPrx = 0;
const long Ptr = 0;
const long abcPtr = 0;
const long Helper = 0;
const long abcHelper = 0;
const long Ice = 0;
const long ice = 0;
const long icea = 0;
const long Iceblah = 0;
const long IceFoo = 0;
const long icecream = 0;
const long ICEpick = 0;
const long iCEaxe = 0;
}

module Ice {}
module IceFoo {}

module all::good::here {}
module an::iceberg::ahead {}
module aPtr::okay::bPrx::fine::cHelper {}

module _a {}           // Illegal leading underscore
module _true {}        // Illegal leading underscore
module \_true {}       // Illegal leading underscore

module b_ {}           // Illegal trailing underscore

module b__c {}         // Illegal double underscores
module b___c {}        // Illegal double underscores

module _a_ {}          // Illegal underscores
module a_b {}          // Illegal underscore
module a_b_c {}        // Illegal underscores
module _a__b__ {}      // Illegal underscores

// Ensure that warnings can be suppressed with local metadata.

[suppress-warning(reserved-identifier)]
module suppressed
{
const long PrxA = 0;
const long APrxA = 0;
const long prxB = 0;
const long Bprx = 0;
const long prx = 0;
const long PtrA = 0;
const long HelperA = 0;
const long aIce = 0;
}
