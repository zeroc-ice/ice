// **********************************************************************
//
// Copyright (c) 2003-2012 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#pragma once

//
// COMPILERFIX: With VC++ flex generated files doesn't compile because
// yywrap is called without args.
//
// The macro could have a different name if flex use "%option prefix".
//
#ifdef _MSC_VER
#   ifdef slice_wrap
#      undef slice_wrap
#      define slice_wrap() 1
#   endif
#   ifdef freeze_script_wrap
#      undef freeze_script_wrap
#      define freeze_script_wrap() 1
#   endif
#   undef yywrap
#   define yywrap() 1
    //
    // We define YY_NO_UNISTD_H so unistd.h doesn't
    // get included
    //
#   define YY_NO_UNISTD_H
#endif
