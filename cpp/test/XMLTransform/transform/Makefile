# **********************************************************************
#
# Copyright (c) 2002
# ZeroC, Inc.
# Billerica, MA, USA
#
# All Rights Reserved.
#
# Ice is free software; you can redistribute it and/or modify it under
# the terms of the GNU General Public License version 2 as published by
# the Free Software Foundation.
#
# **********************************************************************

top_srcdir	= ../../..

POPULATE	= populate
VALIDATE	= validate

TARGETS		= $(POPULATE) \
		  $(VALIDATE)

POPULATE_OBJS	= Populate.o \
		  IntByteMap.o \
		  IntShortMap.o \
		  IntIntMap.o \
		  IntLongMap.o \
		  IntFloatMap.o \
		  IntDoubleMap.o \
		  IntSeq1MapOld.o \
		  IntSeq2MapOld.o \
		  IntSeq3MapOld.o \
		  IntSeq4MapOld.o \
		  IntE1MapOld.o \
		  IntD1MapOld.o \
		  IntS1MapOld.o \
		  IntC1MapOld.o \
		  TestOld.o

VALIDATE_OBJS	= Validate.o \
		  IntByteMap.o \
		  IntShortMap.o \
		  IntIntMap.o \
		  IntLongMap.o \
		  IntFloatMap.o \
		  IntDoubleMap.o \
		  IntSeq1MapNew.o \
		  IntSeq2MapNew.o \
		  IntSeq3MapNew.o \
		  IntSeq4MapNew.o \
		  IntE1MapNew.o \
		  IntD1MapNew.o \
		  IntS1MapNew.o \
		  IntC1MapNew.o \
		  TestNew.o

SRCS		= $(POPULATE_OBJS:.o=.cpp) \
		  $(VALIDATE_OBJS:.o=.cpp)

SCHEMAS		= TestOld.xsd \
		  TestNew.xsd \
		  Identity.xsd

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I. -I../../include $(XERCESC_FLAGS) $(DB_FLAGS) $(CPPFLAGS)

$(POPULATE): $(POPULATE_OBJS) $(SCHEMAS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(POPULATE_OBJS) -lFreeze -lIceXML -lXMLTransform $(LIBS) $(XERCESC_LIBS) $(DB_LIBS)

$(VALIDATE): $(VALIDATE_OBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(VALIDATE_OBJS) -lFreeze -lIceXML -lXMLTransform $(LIBS) $(XERCESC_LIBS) $(DB_LIBS)

IntByteMap.h IntByteMap.cpp: $(SLICE2FREEZE)
	rm -f IntByteMap.h IntByteMap.cpp
	$(SLICE2FREEZE) --dict IntByteMap,int,byte IntByteMap

IntShortMap.h IntShortMap.cpp: $(SLICE2FREEZE)
	rm -f IntShortMap.h IntShortMap.cpp
	$(SLICE2FREEZE) --dict IntShortMap,int,short IntShortMap

IntIntMap.h IntIntMap.cpp: $(SLICE2FREEZE)
	rm -f IntIntMap.h IntIntMap.cpp
	$(SLICE2FREEZE) --dict IntIntMap,int,int IntIntMap

IntLongMap.h IntLongMap.cpp: $(SLICE2FREEZE)
	rm -f IntLongMap.h IntLongMap.cpp
	$(SLICE2FREEZE) --dict IntLongMap,int,long IntLongMap

IntFloatMap.h IntFloatMap.cpp: $(SLICE2FREEZE)
	rm -f IntFloatMap.h IntFloatMap.cpp
	$(SLICE2FREEZE) --dict IntFloatMap,int,float IntFloatMap

IntDoubleMap.h IntDoubleMap.cpp: $(SLICE2FREEZE)
	rm -f IntDoubleMap.h IntDoubleMap.cpp
	$(SLICE2FREEZE) --dict IntDoubleMap,int,double IntDoubleMap

IntSeq1MapOld.h IntSeq1MapOld.cpp: $(SLICE2FREEZE)
	rm -f IntSeq1MapOld.h IntSeq1MapOld.cpp
	$(SLICE2FREEZE) --dict IntSeq1Map,int,Test::Seq1 IntSeq1MapOld TestOld.ice

IntSeq1MapNew.h IntSeq1MapNew.cpp: $(SLICE2FREEZE)
	rm -f IntSeq1MapNew.h IntSeq1MapNew.cpp
	$(SLICE2FREEZE) --dict IntSeq1Map,int,Test::Seq1 IntSeq1MapNew TestNew.ice

IntSeq2MapOld.h IntSeq2MapOld.cpp: $(SLICE2FREEZE)
	rm -f IntSeq2MapOld.h IntSeq2MapOld.cpp
	$(SLICE2FREEZE) --dict IntSeq2Map,int,Test::Seq2 IntSeq2MapOld TestOld.ice

IntSeq2MapNew.h IntSeq2MapNew.cpp: $(SLICE2FREEZE)
	rm -f IntSeq2MapNew.h IntSeq2MapNew.cpp
	$(SLICE2FREEZE) --dict IntSeq2Map,int,Test::Seq2 IntSeq2MapNew TestNew.ice

IntSeq3MapOld.h IntSeq3MapOld.cpp: $(SLICE2FREEZE)
	rm -f IntSeq3MapOld.h IntSeq3MapOld.cpp
	$(SLICE2FREEZE) --dict IntSeq3Map,int,Test::Seq3 IntSeq3MapOld TestOld.ice

IntSeq3MapNew.h IntSeq3MapNew.cpp: $(SLICE2FREEZE)
	rm -f IntSeq3MapNew.h IntSeq3MapNew.cpp
	$(SLICE2FREEZE) --dict IntSeq3Map,int,Test::Seq3 IntSeq3MapNew TestNew.ice

IntSeq4MapOld.h IntSeq4MapOld.cpp: $(SLICE2FREEZE)
	rm -f IntSeq4MapOld.h IntSeq4MapOld.cpp
	$(SLICE2FREEZE) --dict IntSeq4Map,int,Test::Seq4 IntSeq4MapOld TestOld.ice

IntSeq4MapNew.h IntSeq4MapNew.cpp: $(SLICE2FREEZE)
	rm -f IntSeq4MapNew.h IntSeq4MapNew.cpp
	$(SLICE2FREEZE) --dict IntSeq4Map,int,Test::Seq4 IntSeq4MapNew TestNew.ice

IntE1MapOld.h IntE1MapOld.cpp: $(SLICE2FREEZE)
	rm -f IntE1MapOld.h IntE1MapOld.cpp
	$(SLICE2FREEZE) --dict IntE1Map,int,Test::E1 IntE1MapOld TestOld.ice

IntE1MapNew.h IntE1MapNew.cpp: $(SLICE2FREEZE)
	rm -f IntE1MapNew.h IntE1MapNew.cpp
	$(SLICE2FREEZE) --dict IntE1Map,int,Test::E1 IntE1MapNew TestNew.ice

IntD1MapOld.h IntD1MapOld.cpp: $(SLICE2FREEZE)
	rm -f IntD1MapOld.h IntD1MapOld.cpp
	$(SLICE2FREEZE) --dict IntD1Map,int,Test::D1 IntD1MapOld TestOld.ice

IntD1MapNew.h IntD1MapNew.cpp: $(SLICE2FREEZE)
	rm -f IntD1MapNew.h IntD1MapNew.cpp
	$(SLICE2FREEZE) --dict IntD1Map,int,Test::D1 IntD1MapNew TestNew.ice

IntS1MapOld.h IntS1MapOld.cpp: $(SLICE2FREEZE)
	rm -f IntS1MapOld.h IntS1MapOld.cpp
	$(SLICE2FREEZE) --dict IntS1Map,int,Test::S1 IntS1MapOld TestOld.ice

IntS1MapNew.h IntS1MapNew.cpp: $(SLICE2FREEZE)
	rm -f IntS1MapNew.h IntS1MapNew.cpp
	$(SLICE2FREEZE) --dict IntS1Map,int,Test::S1 IntS1MapNew TestNew.ice

IntC1MapOld.h IntC1MapOld.cpp: $(SLICE2FREEZE)
	rm -f IntC1MapOld.h IntC1MapOld.cpp
	$(SLICE2FREEZE) --dict IntC1Map,int,Test::C1 IntC1MapOld TestOld.ice

IntC1MapNew.h IntC1MapNew.cpp: $(SLICE2FREEZE)
	rm -f IntC1MapNew.h IntC1MapNew.cpp
	$(SLICE2FREEZE) --dict IntC1Map,int,Test::C1 IntC1MapNew TestNew.ice

TestOld.xsd: TestOld.ice $(SLICE2XSD)
	$(SLICE2XSD) TestOld.ice

TestNew.xsd: TestNew.ice $(SLICE2XSD)
	$(SLICE2XSD) TestNew.ice

Identity.xsd: $(top_srcdir)/slice/Ice/Identity.ice $(SLICE2XSD)
	$(SLICE2XSD) --ice -I$(top_srcdir)/slice $(top_srcdir)/slice/Ice/Identity.ice

TestOld.h TestOld.cpp: TestOld.ice
	rm -f TestOld.h TestOld.cpp
	$(SLICE2CPP) TestOld.ice

TestNew.h TestNew.cpp: TestNew.ice
	rm -f TestNew.h TestNew.cpp
	$(SLICE2CPP) TestNew.ice

clean::
	rm -f IntByteMap.h IntByteMap.cpp
	rm -f IntShortMap.h IntShortMap.cpp
	rm -f IntIntMap.h IntIntMap.cpp
	rm -f IntLongMap.h IntLongMap.cpp
	rm -f IntFloatMap.h IntFloatMap.cpp
	rm -f IntDoubleMap.h IntDoubleMap.cpp
	rm -f old.xsd new.xsd
	rm -f TestOld.h TestOld.cpp TestOld.xsd
	rm -f TestNew.h TestNew.cpp TestNew.xsd
	rm -f IntSeq1MapOld.h IntSeq1MapOld.cpp
	rm -f IntSeq1MapNew.h IntSeq1MapNew.cpp
	rm -f IntSeq2MapOld.h IntSeq2MapOld.cpp
	rm -f IntSeq2MapNew.h IntSeq2MapNew.cpp
	rm -f IntSeq3MapOld.h IntSeq3MapOld.cpp
	rm -f IntSeq3MapNew.h IntSeq3MapNew.cpp
	rm -f IntSeq4MapOld.h IntSeq4MapOld.cpp
	rm -f IntSeq4MapNew.h IntSeq4MapNew.cpp
	rm -f IntE1MapOld.h IntE1MapOld.cpp
	rm -f IntE1MapNew.h IntE1MapNew.cpp
	rm -f IntD1MapOld.h IntD1MapOld.cpp
	rm -f IntD1MapNew.h IntD1MapNew.cpp
	rm -f IntS1MapOld.h IntS1MapOld.cpp
	rm -f IntS1MapNew.h IntS1MapNew.cpp
	rm -f IntC1MapOld.h IntC1MapOld.cpp
	rm -f IntC1MapNew.h IntC1MapNew.cpp

include .depend
