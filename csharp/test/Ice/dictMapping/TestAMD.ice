// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#pragma once

module Test
{

dictionary<int, int> NV;
dictionary<string, string> NR;
dictionary<string, NV> NDV;
dictionary<string, NR> NDR;

["clr:collection"] dictionary<int, int> OV;
["clr:collection"] dictionary<string, string> OR;
["clr:collection"] dictionary<string, OV> ODV;
["clr:collection"] dictionary<string, OR> ODR;

dictionary<string, ODV> NODV;
dictionary<string, ODR> NODR;

["clr:collection"] dictionary<string, NDV> ONDV;
["clr:collection"] dictionary<string, NDR> ONDR;

sequence<int> AIS;
["clr:collection"] sequence<int> CIS;
["clr:generic:List"] sequence<int> GIS;

sequence<string> ASS;
["clr:collection"] sequence<string> CSS;
["clr:generic:List"] sequence<string> GSS;

dictionary<string, AIS> NDAIS;
dictionary<string, CIS> NDCIS;
dictionary<string, GIS> NDGIS;

dictionary<string, ASS> NDASS;
dictionary<string, CSS> NDCSS;
dictionary<string, GSS> NDGSS;

["clr:collection"] dictionary<string, AIS> ODAIS;
["clr:collection"] dictionary<string, CIS> ODCIS;
["clr:collection"] dictionary<string, GIS> ODGIS;

["clr:collection"] dictionary<string, ASS> ODASS;
["clr:collection"] dictionary<string, CSS> ODCSS;
["clr:collection"] dictionary<string, GSS> ODGSS;

["amd"] class MyClass
{
    void shutdown();

    NV opNV(NV i, out NV o);
    NR opNR(NR i, out NR o);
    NDV opNDV(NDV i, out NDV o);
    NDR opNDR(NDR i, out NDR o);

    OV opOV(OV i, out OV o);
    OR opOR(OR i, out OR o);
    ODV opODV(ODV i, out ODV o);
    ODR opODR(ODR i, out ODR o);

    NODV opNODV(NODV i, out NODV o);
    NODR opNODR(NODR i, out NODR o);

    ONDV opONDV(ONDV i, out ONDV o);
    ONDR opONDR(ONDR i, out ONDR o);

    NDAIS opNDAIS(NDAIS i, out NDAIS o);
    NDCIS opNDCIS(NDCIS i, out NDCIS o);
    NDGIS opNDGIS(NDGIS i, out NDGIS o);

    NDASS opNDASS(NDASS i, out NDASS o);
    NDCSS opNDCSS(NDCSS i, out NDCSS o);
    NDGSS opNDGSS(NDGSS i, out NDGSS o);

    ODAIS opODAIS(ODAIS i, out ODAIS o);
    ODCIS opODCIS(ODCIS i, out ODCIS o);
    ODGIS opODGIS(ODGIS i, out ODGIS o);

    ODASS opODASS(ODASS i, out ODASS o);
    ODCSS opODCSS(ODCSS i, out ODCSS o);
    ODGSS opODGSS(ODGSS i, out ODGSS o);
};

};
