//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;

namespace ZeroC.Ice.Test.SeqMapping.Serialize
{
    [Serializable]
    public class Small // Fewer than 254 bytes with a BinaryFormatter.
    {
        public int i;
    }

    [Serializable]
    public class Large // More than 254 bytes with a BinaryFormatter.
    {
        public double d1;
        public double d2;
        public double d3;
        public double d4;
        public double d5;
        public double d6;
        public double d7;
        public double d8;
        public double d9;
        public double d10;
        public double d11;

        public string s1 = "";

        // Use as data member of Large to ensure that the class serialization will take more than 254 bytes
        public static readonly string LargeString =
            @"Lorem ipsum dolor sit amet, consectetur adipiscing elit. Sed sit amet purus consectetur,
 blandit est eget, eleifend odio. Phasellus augue quam, bibendum id velit eget, tempus aliquet felis.
 Mauris malesuada elementum feugiat. Aenean risus diam, pretium id.";
    }

    [Serializable]
    public class Struct // Used to test that null members marshal correctly.
    {
        public object? o;
        public object? o2;
        public string? s;
        public string? s2;
    }

}
