// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <GrammerUtil.h>

using namespace std;
using namespace Slice;

void __Ice::incRef(StringTok* p) { p -> __incRef(); }
void __Ice::decRef(StringTok* p) { p -> __decRef(); }
void __Ice::incRef(TypeStringListTok* p) { p -> __incRef(); }
void __Ice::decRef(TypeStringListTok* p) { p -> __decRef(); }
void __Ice::incRef(StringListTok* p) { p -> __incRef(); }
void __Ice::decRef(StringListTok* p) { p -> __decRef(); }
void __Ice::incRef(BoolTok* p) { p -> __incRef(); }
void __Ice::decRef(BoolTok* p) { p -> __decRef(); }
void __Ice::incRef(TypeListTok* p) { p -> __incRef(); }
void __Ice::decRef(TypeListTok* p) { p -> __decRef(); }
void __Ice::incRef(ClassListTok* p) { p -> __incRef(); }
void __Ice::decRef(ClassListTok* p) { p -> __decRef(); }
