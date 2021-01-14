// Copyright (c) ZeroC, Inc. All rights reserved.

using System;

var s = new Bom.Bar(); // just to make sure we actually compile the Slice files.
s.X = 5;

Console.WriteLine("testing circular Slice file includes and Slice file with BOM... ok");
return 0;
