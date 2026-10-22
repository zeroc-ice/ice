// Copyright (c) ZeroC, Inc.

#include "include/IcePrefix.ice"        // No error should be caused by this file

module OK
{
    const long PrxA = 0;
    const long APrxA = 0;
    const long prxB = 0;
    const long Bprx = 0;
    const long prx = 0;
    const long PtrA = 0;
    const long HelperA = 0;
    const long HolderA = 0;
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
    const long Holder = 0;
    const long abcHolder = 0;
}

module Modules
{
    module ModuleHolder {}              // Error: 'Holder' suffix is reserved
    module _Module {}                   // Error: illegal leading underscore
}

module Classes
{
    class ClassPtr {}                   // Error: 'Ptr' suffix is reserved
    class Class_ {}                     // Error: illegal trailing underscore

    class DeclHelper;                   // Error: 'Helper' suffix is reserved
    class De__cl;                       // Error: illegal double underscore
}

module Interfaces
{
    interface InterfacePrx {}           // Error: 'Prx' suffix is reserved
    interface _Interface {}             // Error: illegal leading underscore

    interface DeclHolder;               // Error: 'Holder' suffix is reserved
    interface Decl_;                    // Error: illegal trailing underscore
}

module Exceptions
{
    exception ExceptionHelper {}        // Error: 'Helper' suffix is reserved
    exception Exce__ption {}            // Error: illegal double underscore
}

module Structs
{
    struct StructPtr { int i; }         // Error: 'Ptr' suffix is reserved
    struct _Struct { int i; }           // Error: illegal leading underscore
}

module Sequences
{
    sequence<int> SequenceHelper;       // Error: 'Helper' suffix is reserved
    sequence<int> Sequence_;            // Error: illegal trailing underscore
}

module Dictionaries
{
    dictionary<int, int> DictionaryHolder;  // Error: 'Holder' suffix is reserved
    dictionary<int, int> Dictio__nary;      // Error: illegal double underscore
}

module Enums
{
    enum EnumPrx { A }                  // Error: 'Prx' suffix is reserved
    enum _Enum { B }                    // Error: illegal leading underscore
}

module Enumerators
{
    enum E1 { EnumeratorPtr }           // Error: 'Ptr' suffix is reserved
    enum E2 { Enumerator_ }             // Error: illegal trailing underscore
}

module Constants
{
    const long ConstantPrx = 0;         // Error: 'Prx' suffix is reserved
    const long Cons__tant = 0;          // Error: illegal double underscore
}

module DataMembers
{
    class C
    {
        int memberHolder;               // Error: 'Holder' suffix is reserved
        int _member;                    // Error: illegal leading underscore
    }

    struct S
    {
        int memberPrx;                  // Error: 'Prx' suffix is reserved
        int member_;                    // Error: illegal trailing underscore
    }

    exception E
    {
        int memberPtr;                  // Error: 'Ptr' suffix is reserved
        int mem__ber;                   // Error: illegal double underscore
    }
}

module Operations
{
    interface I
    {
        void opHelper();                // Error: 'Helper' suffix is reserved
        void op_();                     // Error: illegal trailing underscore

        void op1(int paramHolder);      // Error: 'Holder' suffix is reserved
        void op2(out int pa__ram);      // Error: illegal double underscore
    }
}
