module Test
{

enum E { E3, E1, E2 };

sequence<string> BoolStringSeq;

sequence<byte> ByteByteSeq;
sequence<short> ByteShortSeq;
sequence<int> ByteIntSeq;
sequence<long> ByteLongSeq;
sequence<string> ByteStringSeq;

sequence<byte> ShortByteSeq;
sequence<short> ShortShortSeq;
sequence<int> ShortIntSeq;
sequence<long> ShortLongSeq;
sequence<string> ShortStringSeq;

sequence<byte> IntByteSeq;
sequence<short> IntShortSeq;
sequence<int> IntIntSeq;
sequence<long> IntLongSeq;
sequence<string> IntStringSeq;

sequence<byte> LongByteSeq;
sequence<short> LongShortSeq;
sequence<int> LongIntSeq;
sequence<long> LongLongSeq;
sequence<string> LongStringSeq;

sequence<float> FloatFloatSeq;
sequence<double> FloatDoubleSeq;
sequence<string> FloatStringSeq;

sequence<float> DoubleFloatSeq;
sequence<double> DoubleDoubleSeq;
sequence<string> DoubleStringSeq;

sequence<bool> StringBoolSeq;
sequence<byte> StringByteSeq;
sequence<short> StringShortSeq;
sequence<int> StringIntSeq;
sequence<long> StringLongSeq;
sequence<float> StringFloatSeq;
sequence<double> StringDoubleSeq;
sequence<string> StringStringSeq;
sequence<E> StringEnumSeq;
sequence<Object*> StringProxySeq;

dictionary<int, string> BoolStringDict;

dictionary<int, byte> LongByteDict;
dictionary<int, short> LongShortDict;
dictionary<int, int> LongIntDict;
dictionary<int, string> LongStringDict;

dictionary<int, float> DoubleFloatDict;
dictionary<int, string> DoubleStringDict;

dictionary<int, bool> StringBoolDict;
dictionary<int, long> StringLongDict;
dictionary<int, double> StringDoubleDict;
dictionary<int, E> StringEnumDict;

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

class Small1
{
    byte b;
    string intToString;
};
sequence<Small1> Small1Seq;
dictionary<int, Small1> IntSmall1Dict;

struct Small2
{
    Small1Seq seq;
};
sequence<Small2> Small2Seq;
dictionary<int, Small2> IntSmall2Dict;

class Small3(100)
{
    string name;
};

struct S
{
    //
    // Primitive transformations.
    //
    string boolToString;

    byte byteToByte;
    short byteToShort;
    int byteToInt;
    long byteToLong;
    string byteToString;

    byte shortToByte;
    short shortToShort;
    int shortToInt;
    long shortToLong;
    string shortToString;

    byte intToByte;
    short intToShort;
    int intToInt;
    long intToLong;
    string intToString;

    byte longToByte;
    short longToShort;
    int longToInt;
    long longToLong;
    string longToString;

    float floatToFloat;
    double floatToDouble;
    string floatToString;

    float doubleToFloat;
    double doubleToDouble;
    string doubleToString;

    bool stringToBool;
    byte stringToByte;
    short stringToShort;
    int stringToInt;
    long stringToLong;
    float stringToFloat;
    double stringToDouble;
    string stringToString;
    E stringToEnum;
    Object* stringToProxy;

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
