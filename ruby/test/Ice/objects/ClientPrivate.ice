//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

module Test
{

class Empty
{
}

class AlsoEmpty
{
}

interface UnexpectedObjectExceptionTest
{
    Empty op();
}

//
// Remaining definitions are here to ensure that the generated code compiles.
//

class COneMember
{
    Empty e;
}

class CTwoMembers
{
    Empty e1;
    Empty e2;
}

exception EOneMember
{
    Empty e;
}

exception ETwoMembers
{
    Empty e1;
    Empty e2;
}

struct SOneMember
{
    Empty e;
}

struct STwoMembers
{
    Empty e1;
    Empty e2;
}

dictionary<int, COneMember> DOneMember;
dictionary<int, CTwoMembers> DTwoMembers;

}
