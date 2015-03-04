module Test
{

enum E { E1, E2, E3};

class MainObject
{
    string boolToString;
    short byteToShort;
    byte shortToByte;
    short intToShort;
    int longToInt;
    string floatToString;
    float doubleToFloat;
    E stringToEnum;
    E newname;
};

/*
 * DerivedMainObject has been removed.
 *
class DerivedMainObject extends MainObject
{
    string name;
};
 */

/*
 * FacetObject has been renamed to RenamedFacetObject.
 */
class RenamedFacetObject
{
    string doubleToString;
};

/*
 * DerivedFacetObject has been removed.
 *
class DerivedFacetObject extends FacetObject
{
    long count;
};
 */

};
