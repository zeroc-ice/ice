module Test
{

enum E { E1, E2, E3 };

sequence<bool> BoolStringSeq;

sequence<byte> ByteByteSeq;
sequence<byte> ByteShortSeq;
sequence<byte> ByteIntSeq;
sequence<byte> ByteLongSeq;
sequence<byte> ByteStringSeq;

sequence<short> ShortByteSeq;
sequence<short> ShortShortSeq;
sequence<short> ShortIntSeq;
sequence<short> ShortLongSeq;
sequence<short> ShortStringSeq;

sequence<int> IntByteSeq;
sequence<int> IntShortSeq;
sequence<int> IntIntSeq;
sequence<int> IntLongSeq;
sequence<int> IntStringSeq;

sequence<long> LongByteSeq;
sequence<long> LongShortSeq;
sequence<long> LongIntSeq;
sequence<long> LongLongSeq;
sequence<long> LongStringSeq;

sequence<float> FloatFloatSeq;
sequence<float> FloatDoubleSeq;
sequence<float> FloatStringSeq;

sequence<double> DoubleFloatSeq;
sequence<double> DoubleDoubleSeq;
sequence<double> DoubleStringSeq;

sequence<string> StringBoolSeq;
sequence<string> StringByteSeq;
sequence<string> StringShortSeq;
sequence<string> StringIntSeq;
sequence<string> StringLongSeq;
sequence<string> StringFloatSeq;
sequence<string> StringDoubleSeq;
sequence<string> StringStringSeq;
sequence<string> StringEnumSeq;
sequence<string> StringProxySeq;

dictionary<int, bool> BoolStringDict;

dictionary<int, long> LongByteDict;
dictionary<int, long> LongShortDict;
dictionary<int, long> LongIntDict;
dictionary<int, long> LongStringDict;

dictionary<int, double> DoubleFloatDict;
dictionary<int, double> DoubleStringDict;

dictionary<int, string> StringBoolDict;
dictionary<int, string> StringLongDict;
dictionary<int, string> StringDoubleDict;
dictionary<int, string> StringEnumDict;

class C
{
    BoolStringDict boolToStringDict;
};

class D extends C
{
    StringByteSeq stringToByteSeq;
    Object obj;
};

class F
{
    StringEnumSeq stringToEnumSeq;
};

struct Small1
{
    byte b;
    int intToString;
};
sequence<Small1> Small1Seq;
dictionary<int, Small1> IntSmall1Dict;

class Small2
{
    Small1Seq seq;
};
sequence<Small2> Small2Seq;
dictionary<int, Small2> IntSmall2Dict;

class Small3(99)
{
    string name;
};

struct S
{
    //
    // Primitive transformations.
    //
    bool boolToString;

    byte byteToByte;
    byte byteToShort;
    byte byteToInt;
    byte byteToLong;
    byte byteToString;

    short shortToByte;
    short shortToShort;
    short shortToInt;
    short shortToLong;
    short shortToString;

    int intToByte;
    int intToShort;
    int intToInt;
    int intToLong;
    int intToString;

    long longToByte;
    long longToShort;
    long longToInt;
    long longToLong;
    long longToString;

    float floatToFloat;
    float floatToDouble;
    float floatToString;

    double doubleToFloat;
    double doubleToDouble;
    double doubleToString;

    string stringToBool;
    string stringToByte;
    string stringToShort;
    string stringToInt;
    string stringToLong;
    string stringToFloat;
    string stringToDouble;
    string stringToString;
    string stringToEnum;
    string stringToProxy;

    //
    // Sequence transformations.
    //
    BoolStringSeq boolSeqToStringSeq;

    ByteByteSeq byteSeqToByteSeq;
    ByteShortSeq byteSeqToShortSeq;
    ByteIntSeq byteSeqToIntSeq;
    ByteLongSeq byteSeqToLongSeq;
    ByteStringSeq byteSeqToStringSeq;

    ShortByteSeq shortSeqToByteSeq;
    ShortShortSeq shortSeqToShortSeq;
    ShortIntSeq shortSeqToIntSeq;
    ShortLongSeq shortSeqToLongSeq;
    ShortStringSeq shortSeqToStringSeq;

    IntByteSeq intSeqToByteSeq;
    IntShortSeq intSeqToShortSeq;
    IntIntSeq intSeqToIntSeq;
    IntLongSeq intSeqToLongSeq;
    IntStringSeq intSeqToStringSeq;

    LongByteSeq longSeqToByteSeq;
    LongShortSeq longSeqToShortSeq;
    LongIntSeq longSeqToIntSeq;
    LongLongSeq longSeqToLongSeq;
    LongStringSeq longSeqToStringSeq;

    FloatFloatSeq floatSeqToFloatSeq;
    FloatDoubleSeq floatSeqToDoubleSeq;
    FloatStringSeq floatSeqToStringSeq;

    DoubleFloatSeq doubleSeqToFloatSeq;
    DoubleDoubleSeq doubleSeqToDoubleSeq;
    DoubleStringSeq doubleSeqToStringSeq;

    StringBoolSeq stringSeqToBoolSeq;
    StringByteSeq stringSeqToByteSeq;
    StringShortSeq stringSeqToShortSeq;
    StringIntSeq stringSeqToIntSeq;
    StringLongSeq stringSeqToLongSeq;
    StringFloatSeq stringSeqToFloatSeq;
    StringDoubleSeq stringSeqToDoubleSeq;
    StringStringSeq stringSeqToStringSeq;
    StringEnumSeq stringSeqToEnumSeq;
    StringProxySeq stringSeqToProxySeq;

    //
    // Dictionary transformations.
    //
    BoolStringDict boolToStringDict;

    LongByteDict longToByteDict;
    LongShortDict longToShortDict;
    LongIntDict longToIntDict;
    LongStringDict longToStringDict;

    DoubleFloatDict doubleToFloatDict;
    DoubleStringDict doubleToStringDict;

    StringBoolDict stringToBoolDict;
    StringLongDict stringToLongDict;
    StringDoubleDict stringToDoubleDict;
    StringEnumDict stringToEnumDict;

    //
    // Struct<->class transformations.
    //
    Small2Seq classToStructSeq;
    IntSmall1Dict small1Dict;
    IntSmall2Dict small2Dict;

    //
    // Object transformations.
    //
    Object nilObject;
    Object baseObject;
    C cObject;
    C dAsCObject;
    D dObject;
    Small3 compactObject;
};

};
