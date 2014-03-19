# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

#
# TODO: 
#
# 1. We probably would like to take advantage of the Ice Make.rules
# files.
#

top_srcdir	= ../..

TARGETS =  Service \
	   Consumer \
	   Supplier \
	   Destroyer \
	   Notifier

IDL_FILES = Perf
IDL_SRC = PerfC.cpp PerfS.cpp

SUPPLIER_OBJS = WorkerThread.o \
                PerfC.o \
		PerfS.o \
		SyncC.o \
		SyncS.o \
                Supplier.o

CONSUMER_OBJS = WorkerThread.o \
                PerfC.o \
                PerfS.o \
                Consumer.o

SERVICE_OBJS =  WorkerThread.o \
		Service.o

DESTROYER_OBJS = Destroyer.o

NOTIFIER_OBJS =  Notifier.o \
	         SyncC.o 

SRCS = $(NOTIFIER_OBJS:.o=.cpp) $(DESTROYER_OBJS:.o=.cpp) $(SERVICE_OBJS:.o=.cpp) \
	$(CONSUMER_OBJS:.o=.cpp) $(SUPPLIER_OBJS:.o=.cpp)

include $(top_srcdir)/config/Make.rules
include $(top_srcdir)/config/Make.tao.rules


TAO_CLIENTLIBS = -lTAO 
TAO_POALIBS = -lTAO_PortableServer
EVENT_LIBS = -lTAO_CosEvent -lTAO_CosEvent_Skel -lTAO_Messaging
SERVICE_LIBS = -lTAO_CosEvent_Serv

#----------------------------------------------------------------------------
#       Local targets
#----------------------------------------------------------------------------

.PRECIOUS: PerfC.cpp PerfC.i PerfC.h
.PRECIOUS: PerfS.cpp PerfS.i PerfS.h
.PRECIOUS: PerfS_T.cpp PerfS_T.i PerfS_T.h

all:: $(TARGETS)

clean::
	-$(RM) -rf $(SUPPLIER_OBJS) $(CONSUMER_OBJS) $(SERVICE_OBJS) \
	$(DESTROYER_OBJS) $(NOTIFIER_OBJS) $(TARGETS)

realclean: clean
	-$(RM) -rf PerfC.* PerfS.* PerfS_T.* SyncC.* SyncS.* SyncS_T.* 

Supplier: $(addprefix $(VDIR),$(SUPPLIER_OBJS))
	$(LINK.cc) $(LDFLAGS) -o $@ $^ $(EVENT_LIBS) $(TAO_POALIBS) $(TAO_CLIENTLIBS) $(ICE_LIBS)

Consumer: $(addprefix $(VDIR),$(CONSUMER_OBJS))
	$(LINK.cc) $(LDFLAGS) -o $@ $^ $(EVENT_LIBS) $(TAO_POALIBS) $(TAO_CLIENTLIBS) $(ICE_LIBS)

Service: $(addprefix $(VDIR),$(SERVICE_OBJS))
	$(LINK.cc) $(LDFLAGS) -o $@ $^ $(EVENT_LIBS) $(SERVICE_LIBS) $(TAO_POALIBS) $(TAO_CLIENTLIBS)

Destroyer: $(addprefix $(VDIR),$(DESTROYER_OBJS))
	$(LINK.cc) $(LDFLAGS) -o $@ $^ -lTAO_CosEvent $(TAO_CLIENTLIBS)

Notifier: $(addprefix $(VDIR),$(NOTIFIER_OBJS))
	$(LINK.cc) $(LDFLAGS) -o $@ $^ -lTAO_AnyTypeCode $(TAO_CLIENTLIBS)

PerfS.cpp PerfC.cpp: Perf.idl
	$(TAO_ROOT)/bin/tao_idl -Ge 1 $^

SyncC.cpp SyncS.cpp: Sync.idl
	$(TAO_ROOT)/bin/tao_idl -Ge 1 $^

include .depend
