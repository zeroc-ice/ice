module Test
{

enum E { E1, E2, E3};

class MainObject
{
    bool boolToString;
    byte byteToShort;
    short shortToByte;
    int intToShort;
    long longToInt;
    float floatToString;
    double doubleToFloat;
    string stringToEnum;
    E renamed;
};

class DerivedMainObject extends MainObject
{
    string name;
};

class FacetObject
{
    double doubleToString;
};

class DerivedFacetObject extends FacetObject
{
    long count;
};

};
