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

void IceInternal::incRef(StringTok* p) { p->__incRef(); }
void IceInternal::decRef(StringTok* p) { p->__decRef(); }
void IceInternal::incRef(TypeStringListTok* p) { p->__incRef(); }
void IceInternal::decRef(TypeStringListTok* p) { p->__decRef(); }
void IceInternal::incRef(StringListTok* p) { p->__incRef(); }
void IceInternal::decRef(StringListTok* p) { p->__decRef(); }
void IceInternal::incRef(BoolTok* p) { p->__incRef(); }
void IceInternal::decRef(BoolTok* p) { p->__decRef(); }
void IceInternal::incRef(TypeListTok* p) { p->__incRef(); }
void IceInternal::decRef(TypeListTok* p) { p->__decRef(); }
void IceInternal::incRef(ClassListTok* p) { p->__incRef(); }
void IceInternal::decRef(ClassListTok* p) { p->__decRef(); }
