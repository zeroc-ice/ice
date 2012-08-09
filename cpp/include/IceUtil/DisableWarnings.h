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
// This header file disables various annoying compiler warnings that
// we don't want.
//
// IMPORTANT: Do *not* include this header file in another public header file!
//            Doing this may potentially disable the warnings in the source
//            code of our customers, which would be bad. Only include this
//            header file in Ice *source* files!
//

#if defined(_MSC_VER)
#    define _CRT_SECURE_NO_DEPRECATE 1  // C4996 '<C function>' was declared deprecated/
#    pragma warning( 4 : 4996 ) // C4996 'std::<function>' was declared deprecated
#    pragma warning( 4 : 4800 ) // C4800 forcing value to bool 'true' or 'false' (performance warning)

#endif
