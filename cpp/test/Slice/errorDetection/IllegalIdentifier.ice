
// NOTE: Keep the initial blank line. This is necessary to avoid a problem
// with system-provided mcpp versions that don't include ZeroC's BOM patch.

//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

//
// Check that 'œ' is properly rejected in identifiers
//

module Test
{
   interface foo
   {
      int scoped::name();
   }

   interface Œuvre
   {
      void cœur();
   }
}

//
// Check that UTF-8 BOM is rejected when not at start of file
//
﻿
