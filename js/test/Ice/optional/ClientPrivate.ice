//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

#include <Test.ice>

module Test
{

//
// The server doesn't know this class.
//
// We don't define it in JavaScript to allow the cross tests to work when running
// only the JavaScript server within the browser (which loads ClientPrivate.js for
// the server as well)
//
// class D extends B
// {
//     string ds;
//     tag(990) StringSeq seq;
//     tag(1000) A ao;
// }

//
// This class is a hack that allows us to invoke the opClassAndUnknownOptional operation
// on the server and pass an optional argument. This isn't necessary in other language
// mappings where the public stream API is available.
//
interface Initial2
{
    void opClassAndUnknownOptional(A p, tag(1) Object o);

    void opVoid(tag(1) int a, tag(2) string v);
}

}
