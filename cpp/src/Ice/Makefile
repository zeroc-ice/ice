#**********************************************************************
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

OBJS		= Initialize.o \
		  Application.o \
		  Exception.o \
		  LocalException.o \
		  Properties.o \
		  PropertiesI.o \
		  Logger.o \
		  LoggerUtil.o \
		  LoggerI.o \
		  SysLoggerI.o \
		  Stream.o \
		  BuiltinSequences.o \
		  BasicStream.o \
		  TraceLevels.o \
		  TraceUtil.o \
		  Instance.o \
		  Communicator.o \
		  CommunicatorI.o \
		  ObjectFactory.o \
		  ObjectFactoryManager.o \
		  ObjectAdapter.o \
		  ObjectAdapterI.o \
		  ObjectAdapterFactory.o \
		  ServantLocator.o \
		  RequestQueue.o \
		  UserExceptionFactory.o \
		  UserExceptionFactoryManager.o \
		  Endpoint.o \
		  Reference.o \
		  ReferenceFactory.o \
		  Identity.o \
		  IdentityUtil.o \
		  Current.o \
		  Router.o \
		  RouterInfo.o \
		  RoutingTable.o \
		  Connection.o \
		  ConnectionFactory.o \
		  LocalObject.o \
		  Object.o \
		  ProxyFactory.o \
		  Proxy.o \
		  Outgoing.o \
		  Incoming.o \
		  Direct.o \
		  Network.o \
	          ThreadPool.o \
		  EventHandler.o \
		  Connector.o \
		  Acceptor.o \
		  Transceiver.o \
		  TcpConnector.o \
		  TcpAcceptor.o \
		  TcpTransceiver.o \
                  SecurityException.o \
                  SecurityException2.o \
                  SslConnector.o \
                  SslAcceptor.o \
		  SslTransceiver.o \
                  SslBaseCerts.o \
                  SslCertificateAuthority.o \
                  SslCertificateDesc.o \
                  SslConfig.o \
                  SslConfigErrorReporter.o \
                  SslConnection.o \
                  SslConnectionOpenSSLClient.o \
                  SslConnectionOpenSSL.o \
                  SslConnectionOpenSSLServer.o \
                  SslFactory.o \
                  SslGeneralConfig.o \
                  SslSystem.o \
                  SslSystemOpenSSL.o \
                  SslTempCerts.o \
                  SslCertificateVerifier.o \
                  SslCertificateVerifierOpenSSL.o \
                  SslJanitors.o \
                  SslRSACertificateGen.o \
                  SslIceUtils.o \
                  SslRSAPrivateKey.o \
                  SslRSAPublicKey.o \
                  SslRSAKeyPair.o \
                  SslOpenSSLUtils.o \
		  UdpTransceiver.o \
                  Cryptor.o \
                  CryptKey.o \
                  MessageAuthenticator.o \
                  SUdpClient.o \
                  SUdpTransceiver.o \
                  SUdpControlChannel.o \
                  SUdpServerControlChannel.o \
                  SUdpClientControlChannel.o \
                  SecureUdp.o

SRCS		= $(OBJS:.o=.cpp)

HDIR		= $(includedir)/Ice
SDIR		= $(slicedir)/Ice
SLICECMD	= $(SLICE2CPP) --include-dir Ice --dll-export ICE_API -I$(slicedir)

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I.. $(CPPFLAGS) -I$(OPENSSL_HOME)/include -I$(XERCESC_HOME)/include/xercesc

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

$(HDIR)/SecurityException.h SecurityException.cpp: $(SDIR)/SecurityException.ice $(SLICE2CPP)
	rm -f $(HDIR)/SecurityException.h SecurityException.cpp
	$(SLICECMD) $(SDIR)/SecurityException.ice
	mv SecurityException.h $(HDIR)

clean::
	rm -f $(HDIR)/SecurityException.h SecurityException.cpp

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

$(HDIR)/BuiltinSequences.h BuiltinSequences.cpp: $(SDIR)/BuiltinSequences.ice $(SLICE2CPP)
	rm -f $(HDIR)/BuiltinSequences.h BuiltinSequences.cpp
	$(SLICECMD) $(SDIR)/BuiltinSequences.ice
	mv BuiltinSequences.h $(HDIR)

clean::
	rm -f $(HDIR)/BuiltinSequences.h BuiltinSequences.cpp

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

$(HDIR)/SecureUdp.h SecureUdp.cpp: $(SDIR)/SecureUdp.ice $(SLICE2CPP)
	rm -f $(HDIR)/SecureUdp.h SecureUdp.cpp
	$(SLICECMD) $(SDIR)/SecureUdp.ice
	mv SecureUdp.h $(HDIR)

clean::
	rm -f $(HDIR)/SecureUdp.h SecureUdp.cpp

$(HDIR)/SecureUdpF.h: $(SDIR)/SecureUdpF.ice $(SLICE2CPP)
	rm -f $(HDIR)/SecureUdpF.h SecureUdpF.cpp
	$(SLICECMD) $(SDIR)/SecureUdpF.ice
	mv SecureUdpF.h $(HDIR)
	rm -f SecureUdpF.cpp

clean::
	rm -f $(HDIR)/SecureUdpF.h

$(HDIR)/SslCertificateVerifier.h SslCertificateVerifier.cpp: $(SDIR)/SslCertificateVerifier.ice $(SLICE2CPP)
	rm -f $(HDIR)/SslCertificateVerifier.h SslCertificateVerifier.cpp
	$(SLICECMD) $(SDIR)/SslCertificateVerifier.ice
	mv SslCertificateVerifier.h $(HDIR)

clean::
	rm -f $(HDIR)/SslCertificateVerifier.h SslCertificateVerifier.cpp

$(HDIR)/SslCertificateVerifierF.h: $(SDIR)/SslCertificateVerifierF.ice $(SLICE2CPP)
	rm -f $(HDIR)/SslCertificateVerifierF.h SslCertificateVerifierF.cpp
	$(SLICECMD) $(SDIR)/SslCertificateVerifierF.ice
	mv SslCertificateVerifierF.h $(HDIR)
	rm -f SslCertificateVerifierF.cpp

clean::
	rm -f $(HDIR)/SslCertificateVerifierF.h

$(HDIR)/ServantLocator.h ServantLocator.cpp: $(SDIR)/ServantLocator.ice $(SLICE2CPP)
	rm -f $(HDIR)/ServantLocator.h ServantLocator.cpp
	$(SLICECMD) $(SDIR)/ServantLocator.ice
	mv ServantLocator.h $(HDIR)

clean::
	rm -f $(HDIR)/ServantLocator.h ServantLocator.cpp

$(HDIR)/ServantLocatorF.h: $(SDIR)/ServantLocatorF.ice $(SLICE2CPP)
	rm -f $(HDIR)/ServantLocatorF.h ServantLocatorF.cpp
	$(SLICECMD) $(SDIR)/ServantLocatorF.ice
	mv ServantLocatorF.h $(HDIR)
	rm -f ServantLocatorF.cpp

clean::
	rm -f $(HDIR)/ServantLocatorF.h

$(HDIR)/RequestQueue.h RequestQueue.cpp: $(SDIR)/RequestQueue.ice $(SLICE2CPP)
	rm -f $(HDIR)/RequestQueue.h RequestQueue.cpp
	$(SLICECMD) $(SDIR)/RequestQueue.ice
	mv RequestQueue.h $(HDIR)

clean::
	rm -f $(HDIR)/RequestQueue.h RequestQueue.cpp

$(HDIR)/RequestQueueF.h: $(SDIR)/RequestQueueF.ice $(SLICE2CPP)
	rm -f $(HDIR)/RequestQueueF.h RequestQueueF.cpp
	$(SLICECMD) $(SDIR)/RequestQueueF.ice
	mv RequestQueueF.h $(HDIR)
	rm -f RequestQueueF.cpp

clean::
	rm -f $(HDIR)/RequestQueueF.h

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

$(HDIR)/ObjectFactory.h ObjectFactory.cpp: $(SDIR)/ObjectFactory.ice $(SLICE2CPP)
	rm -f $(HDIR)/ObjectFactory.h ObjectFactory.cpp
	$(SLICECMD) $(SDIR)/ObjectFactory.ice
	mv ObjectFactory.h $(HDIR)

clean::
	rm -f $(HDIR)/ObjectFactory.h ObjectFactory.cpp

$(HDIR)/ObjectFactoryF.h: $(SDIR)/ObjectFactoryF.ice $(SLICE2CPP)
	rm -f $(HDIR)/ObjectFactoryF.h ObjectFactoryF.cpp
	$(SLICECMD) $(SDIR)/ObjectFactoryF.ice
	mv ObjectFactoryF.h $(HDIR)
	rm -f ObjectFactoryF.cpp

clean::
	rm -f $(HDIR)/ObjectFactoryF.h

$(HDIR)/UserExceptionFactory.h UserExceptionFactory.cpp: $(SDIR)/UserExceptionFactory.ice $(SLICE2CPP)
	rm -f $(HDIR)/UserExceptionFactory.h UserExceptionFactory.cpp
	$(SLICECMD) $(SDIR)/UserExceptionFactory.ice
	mv UserExceptionFactory.h $(HDIR)

clean::
	rm -f $(HDIR)/UserExceptionFactory.h UserExceptionFactory.cpp

$(HDIR)/UserExceptionFactoryF.h: $(SDIR)/UserExceptionFactoryF.ice $(SLICE2CPP)
	rm -f $(HDIR)/UserExceptionFactoryF.h UserExceptionFactoryF.cpp
	$(SLICECMD) $(SDIR)/UserExceptionFactoryF.ice
	mv UserExceptionFactoryF.h $(HDIR)
	rm -f UserExceptionFactoryF.cpp

clean::
	rm -f $(HDIR)/UserExceptionFactoryF.h

$(HDIR)/Identity.h Identity.cpp: $(SDIR)/Identity.ice $(SLICE2CPP)
	rm -f $(HDIR)/Identity.h Identity.cpp
	$(SLICECMD) $(SDIR)/Identity.ice
	mv Identity.h $(HDIR)

clean::
	rm -f $(HDIR)/Identity.h Identity.cpp

$(HDIR)/Current.h Current.cpp: $(SDIR)/Current.ice $(SLICE2CPP)
	rm -f $(HDIR)/Current.h Current.cpp
	$(SLICECMD) $(SDIR)/Current.ice
	mv Current.h $(HDIR)

clean::
	rm -f $(HDIR)/Current.h Current.cpp

$(HDIR)/Router.h Router.cpp: $(SDIR)/Router.ice $(SLICE2CPP)
	rm -f $(HDIR)/Router.h Router.cpp
	$(SLICECMD) $(SDIR)/Router.ice
	mv Router.h $(HDIR)

clean::
	rm -f $(HDIR)/Router.h Router.cpp

$(HDIR)/RouterF.h: $(SDIR)/RouterF.ice $(SLICE2CPP)
	rm -f $(HDIR)/RouterF.h RouterF.cpp
	$(SLICECMD) $(SDIR)/RouterF.ice
	mv RouterF.h $(HDIR)
	rm -f RouterF.cpp

clean::
	rm -f $(HDIR)/RouterF.h

include .depend
