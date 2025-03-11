// Copyright (c) ZeroC, Inc.

#pragma once

["cs:namespace:Ice.dictMapping"]
module Test
{
    dictionary<int, int> NV;
    dictionary<string, string> NR;
    dictionary<string, NV> NDV;
    dictionary<string, NR> NDR;
    sequence<int> IntSeq;
    struct TEstNv
    {
        NV d;
        IntSeq s;
    }

    sequence<int> AIS;
    ["cs:generic:List"] sequence<int> GIS;

    sequence<string> ASS;
    ["cs:generic:List"] sequence<string> GSS;

    dictionary<string, AIS> NDAIS;
    dictionary<string, GIS> NDGIS;

    dictionary<string, ASS> NDASS;
    dictionary<string, GSS> NDGSS;

    interface MyClass
    {
        void shutdown();

        NV opNV(NV i, out NV o);
        NR opNR(NR i, out NR o);
        NDV opNDV(NDV i, out NDV o);
        NDR opNDR(NDR i, out NDR o);

        NDAIS opNDAIS(NDAIS i, out NDAIS o);
        NDGIS opNDGIS(NDGIS i, out NDGIS o);

        NDASS opNDASS(NDASS i, out NDASS o);
        NDGSS opNDGSS(NDGSS i, out NDGSS o);
    }
}
