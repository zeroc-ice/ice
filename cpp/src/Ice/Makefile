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

OBJS		= Application.o \
		  Exception.o \
		  LocalException.o \
		  Properties.o \
		  PropertiesI.o \
		  Logger.o \
		  LoggerI.o \
		  Stream.o \
		  StreamI.o \
		  BasicStream.o \
		  SysLoggerI.o \
		  TraceLevels.o \
		  TraceUtil.o \
		  Instance.o \
		  Communicator.o \
		  CommunicatorI.o \
		  ObjectAdapter.o \
		  ObjectAdapterI.o \
		  ObjectAdapterFactory.o \
		  ServantFactory.o \
		  ServantFactoryManager.o \
		  Endpoint.o \
		  Reference.o \
		  LocalObject.o \
		  Object.o \
		  ProxyFactory.o \
		  Proxy.o \
		  Outgoing.o \
		  Incoming.o \
		  Direct.o \
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
SDIR		= $(slicedir)/Ice
SLICECMD	= $(SLICE2CPP) --include-dir Ice --dll-export ICE_API -I$(slicedir)

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I.. $(CPPFLAGS)

$(VERSIONED_NAME): $(OBJS)
	rm -f $@
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -shared -o $@ $(OBJS)

$(NAME): $(VERSIONED_NAME)
	rm -f $@
	ln -s $(VERSIONED_BASE) $@

$(HDIR)/LocalException.h LocalException.cpp: $(SDIR)/LocalException.ice $(SLICE2CPP)
	rm -f $(HDIR)/LocalException.h LocalException.cpp
	$(SLICECMD) $(SDIR)/LocalException.ice
	mv LocalException.h $(HDIR)

clean::
	rm -f $(HDIR)/LocalException.h LocalException.cpp

$(HDIR)/Communicator.h Communicator.cpp: $(SDIR)/Communicator.ice $(SLICE2CPP)
	rm -f $(HDIR)/Communicator.h Communicator.cpp
	$(SLICECMD) $(SDIR)/Communicator.ice
	mv Communicator.h $(HDIR)

clean::
	rm -f $(HDIR)/Communicator.h Communicator.cpp

$(HDIR)/CommunicatorF.h: $(SDIR)/CommunicatorF.ice $(SLICE2CPP)
	rm -f $(HDIR)/CommunicatorF.h CommunicatorF.cpp
	$(SLICECMD) $(SDIR)/CommunicatorF.ice
	mv CommunicatorF.h $(HDIR)
	rm -f CommunicatorF.cpp

clean::
	rm -f $(HDIR)/CommunicatorF.h

$(HDIR)/Logger.h Logger.cpp: $(SDIR)/Logger.ice $(SLICE2CPP)
	rm -f $(HDIR)/Logger.h Logger.cpp
	$(SLICECMD) $(SDIR)/Logger.ice
	mv Logger.h $(HDIR)

clean::
	rm -f $(HDIR)/Logger.h Logger.cpp

$(HDIR)/LoggerF.h: $(SDIR)/LoggerF.ice $(SLICE2CPP)
	rm -f $(HDIR)/LoggerF.h LoggerF.cpp
	$(SLICECMD) $(SDIR)/LoggerF.ice
	mv LoggerF.h $(HDIR)
	rm -f LoggerF.cpp

clean::
	rm -f $(HDIR)/LoggerF.h

$(HDIR)/Stream.h Stream.cpp: $(SDIR)/Stream.ice $(SLICE2CPP)
	rm -f $(HDIR)/Stream.h Stream.cpp
	$(SLICECMD) $(SDIR)/Stream.ice
	mv Stream.h $(HDIR)

clean::
	rm -f $(HDIR)/Stream.h Stream.cpp

$(HDIR)/StreamF.h: $(SDIR)/StreamF.ice $(SLICE2CPP)
	rm -f $(HDIR)/StreamF.h StreamF.cpp
	$(SLICECMD) $(SDIR)/StreamF.ice
	mv StreamF.h $(HDIR)
	rm -f StreamF.cpp

clean::
	rm -f $(HDIR)/StreamF.h

$(HDIR)/ObjectAdapter.h ObjectAdapter.cpp: $(SDIR)/ObjectAdapter.ice $(SLICE2CPP)
	rm -f $(HDIR)/ObjectAdapter.h ObjectAdapter.cpp
	$(SLICECMD) $(SDIR)/ObjectAdapter.ice
	mv ObjectAdapter.h $(HDIR)

clean::
	rm -f $(HDIR)/ObjectAdapter.h ObjectAdapter.cpp

$(HDIR)/ObjectAdapterF.h: $(SDIR)/ObjectAdapterF.ice $(SLICE2CPP)
	rm -f $(HDIR)/ObjectAdapterF.h ObjectAdapterF.cpp
	$(SLICECMD) $(SDIR)/ObjectAdapterF.ice
	mv ObjectAdapterF.h $(HDIR)
	rm -f ObjectAdapterF.cpp

clean::
	rm -f $(HDIR)/ObjectAdapterF.h

$(HDIR)/Properties.h Properties.cpp: $(SDIR)/Properties.ice $(SLICE2CPP)
	rm -f $(HDIR)/Properties.h Properties.cpp
	$(SLICECMD) $(SDIR)/Properties.ice
	mv Properties.h $(HDIR)

clean::
	rm -f $(HDIR)/Properties.h Properties.cpp

$(HDIR)/PropertiesF.h: $(SDIR)/PropertiesF.ice $(SLICE2CPP)
	rm -f $(HDIR)/PropertiesF.h PropertiesF.cpp
	$(SLICECMD) $(SDIR)/PropertiesF.ice
	mv PropertiesF.h $(HDIR)
	rm -f PropertiesF.cpp

clean::
	rm -f $(HDIR)/PropertiesF.h

$(HDIR)/ServantFactory.h ServantFactory.cpp: $(SDIR)/ServantFactory.ice $(SLICE2CPP)
	rm -f $(HDIR)/ServantFactory.h ServantFactory.cpp
	$(SLICECMD) $(SDIR)/ServantFactory.ice
	mv ServantFactory.h $(HDIR)

clean::
	rm -f $(HDIR)/ServantFactory.h ServantFactory.cpp

$(HDIR)/ServantFactoryF.h: $(SDIR)/ServantFactoryF.ice $(SLICE2CPP)
	rm -f $(HDIR)/ServantFactoryF.h ServantFactoryF.cpp
	$(SLICECMD) $(SDIR)/ServantFactoryF.ice
	mv ServantFactoryF.h $(HDIR)
	rm -f ServantFactoryF.cpp

clean::
	rm -f $(HDIR)/ServantFactoryF.h

include .depend
