# **********************************************************************
#
# Copyright (c) 2001
# MutableRealms, Inc.
# Huntsville, AL, USA
#
# All Rights Reserved
#
# **********************************************************************

top_srcdir	= ../..

BASE		= libIce.so
VERSIONED_BASE	= $(BASE).$(VERSION)

NAME		= $(top_srcdir)/lib/$(BASE)
VERSIONED_NAME	= $(top_srcdir)/lib/$(VERSIONED_BASE)

TARGETS		= $(NAME) $(VERSIONED_NAME)

OBJS		= Shared.o \
		  Stream.o \
		  LocalException.o \
		  Pickler.o \
		  PicklerI.o \
		  Properties.o \
		  PropertiesI.o \
		  Logger.o \
		  LoggerI.o \
		  SysLoggerI.o \
		  TraceLevels.o \
		  TraceUtil.o \
		  Instance.o \
		  Communicator.o \
		  CommunicatorI.o \
		  ObjectAdapter.o \
		  ObjectAdapterI.o \
		  ObjectAdapterFactory.o \
		  ValueFactory.o \
		  ValueFactoryManager.o \
		  Endpoint.o \
		  Reference.o \
		  LocalObject.o \
		  Object.o \
		  ProxyFactory.o \
		  Proxy.o \
		  Outgoing.o \
		  Incoming.o \
		  Emitter.o \
		  Collector.o \
		  Network.o \
	          ThreadPool.o \
		  EventHandler.o \
		  Connector.o \
		  Acceptor.o \
		  Transceiver.o \
		  TcpConnector.o \
		  TcpAcceptor.o \
		  TcpTransceiver.o \
		  SslConnector.o \
		  SslAcceptor.o \
		  SslTransceiver.o \
		  UdpTransceiver.o

SRCS		= $(OBJS:.o=.cpp)

HDIR		= $(includedir)/Ice
IDIR		= $(slicedir)/Ice
SLICECMD	= $(SLICE) --include-dir Ice --dll-export ICE_API -I$(slicedir)

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I.. $(CPPFLAGS)

$(VERSIONED_NAME): $(OBJS)
	rm -f $@
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -shared -o $@ $(OBJS)

$(NAME): $(VERSIONED_NAME)
	rm -f $@
	ln -s $(VERSIONED_BASE) $@

$(HDIR)/Communicator.h Communicator.cpp: $(IDIR)/Communicator.ice $(SLICE)
	rm -f $(HDIR)/Communicator.h Communicator.cpp
	$(SLICECMD) $(IDIR)/Communicator.ice
	mv Communicator.h $(HDIR)

clean::
	rm -f $(HDIR)/Communicator.h Communicator.cpp

$(HDIR)/CommunicatorF.h: $(IDIR)/CommunicatorF.ice $(SLICE)
	rm -f $(HDIR)/CommunicatorF.h CommunicatorF.cpp
	$(SLICECMD) $(IDIR)/CommunicatorF.ice
	mv CommunicatorF.h $(HDIR)
	rm -f CommunicatorF.cpp

clean::
	rm -f $(HDIR)/CommunicatorF.h

$(HDIR)/Logger.h Logger.cpp: $(IDIR)/Logger.ice $(SLICE)
	rm -f $(HDIR)/Logger.h Logger.cpp
	$(SLICECMD) $(IDIR)/Logger.ice
	mv Logger.h $(HDIR)

clean::
	rm -f $(HDIR)/Logger.h Logger.cpp

$(HDIR)/LoggerF.h: $(IDIR)/LoggerF.ice $(SLICE)
	rm -f $(HDIR)/LoggerF.h LoggerF.cpp
	$(SLICECMD) $(IDIR)/LoggerF.ice
	mv LoggerF.h $(HDIR)
	rm -f LoggerF.cpp

clean::
	rm -f $(HDIR)/LoggerF.h

$(HDIR)/ObjectAdapter.h ObjectAdapter.cpp: $(IDIR)/ObjectAdapter.ice $(SLICE)
	rm -f $(HDIR)/ObjectAdapter.h ObjectAdapter.cpp
	$(SLICECMD) $(IDIR)/ObjectAdapter.ice
	mv ObjectAdapter.h $(HDIR)

clean::
	rm -f $(HDIR)/ObjectAdapter.h ObjectAdapter.cpp

$(HDIR)/ObjectAdapterF.h: $(IDIR)/ObjectAdapterF.ice $(SLICE)
	rm -f $(HDIR)/ObjectAdapterF.h ObjectAdapterF.cpp
	$(SLICECMD) $(IDIR)/ObjectAdapterF.ice
	mv ObjectAdapterF.h $(HDIR)
	rm -f ObjectAdapterF.cpp

clean::
	rm -f $(HDIR)/ObjectAdapterF.h

$(HDIR)/Pickler.h Pickler.cpp: $(IDIR)/Pickler.ice $(SLICE)
	rm -f $(HDIR)/Pickler.h Pickler.cpp
	$(SLICECMD) $(IDIR)/Pickler.ice
	mv Pickler.h $(HDIR)

clean::
	rm -f $(HDIR)/Pickler.h Pickler.cpp

$(HDIR)/PicklerF.h: $(IDIR)/PicklerF.ice $(SLICE)
	rm -f $(HDIR)/PicklerF.h PicklerF.cpp
	$(SLICECMD) $(IDIR)/PicklerF.ice
	mv PicklerF.h $(HDIR)
	rm -f PicklerF.cpp

$(HDIR)/Properties.h Properties.cpp: $(IDIR)/Properties.ice $(SLICE)
	rm -f $(HDIR)/Properties.h Properties.cpp
	$(SLICECMD) $(IDIR)/Properties.ice
	mv Properties.h $(HDIR)

clean::
	rm -f $(HDIR)/Properties.h Properties.cpp

clean::
	rm -f $(HDIR)/PicklerF.h

$(HDIR)/PropertiesF.h: $(IDIR)/PropertiesF.ice $(SLICE)
	rm -f $(HDIR)/PropertiesF.h PropertiesF.cpp
	$(SLICECMD) $(IDIR)/PropertiesF.ice
	mv PropertiesF.h $(HDIR)
	rm -f PropertiesF.cpp

clean::
	rm -f $(HDIR)/PropertiesF.h

$(HDIR)/ValueFactory.h ValueFactory.cpp: $(IDIR)/ValueFactory.ice $(SLICE)
	rm -f $(HDIR)/ValueFactory.h ValueFactory.cpp
	$(SLICECMD) $(IDIR)/ValueFactory.ice
	mv ValueFactory.h $(HDIR)

clean::
	rm -f $(HDIR)/ValueFactory.h ValueFactory.cpp

$(HDIR)/ValueFactoryF.h: $(IDIR)/ValueFactoryF.ice $(SLICE)
	rm -f $(HDIR)/ValueFactoryF.h ValueFactoryF.cpp
	$(SLICECMD) $(IDIR)/ValueFactoryF.ice
	mv ValueFactoryF.h $(HDIR)
	rm -f ValueFactoryF.cpp

clean::
	rm -f $(HDIR)/ValueFactoryF.h

include .depend
