//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

[[suppress-warning(reserved-identifier)]]

module ZeroC::Ice::Test::DictMapping
{

dictionary<int, int> NV;
dictionary<string, string> NR;
dictionary<string, NV> NDV;
dictionary<string, NR> NDR;

sequence<int> AIS;
[cs:generic(List)] sequence<int> GIS;

sequence<string> ASS;
[cs:generic(List)] sequence<string> GSS;

dictionary<string, AIS> NDAIS;
dictionary<string, GIS> NDGIS;

dictionary<string, ASS> NDASS;
dictionary<string, GSS> NDGSS;

interface MyClass
{
    void shutdown();

    (NV r1, NV r2) opNV(NV i);
    (NR r1, NR r2) opNR(NR i);
    (NDV r1, NDV r2) opNDV(NDV i);
    (NDR r1, NDR r2) opNDR(NDR i);

    (NDAIS r1, NDAIS r2) opNDAIS(NDAIS i);
    (NDGIS r1, NDGIS r2) opNDGIS(NDGIS i);

    (NDASS r1, NDASS r2) opNDASS(NDASS i);
    (NDGSS r1, NDGSS r2) opNDGSS(NDGSS i);
}

}
