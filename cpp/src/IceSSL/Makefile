#**********************************************************************
#
# Copyright (c) 2002
# Mutable Realms, Inc.
# Huntsville, AL, USA
#
# All Rights Reserved
#
# **********************************************************************

top_srcdir	= ../..

BASE		= libIceSSL.so
VERSIONED_BASE	= $(BASE).$(VERSION)

NAME		= $(top_srcdir)/lib/$(BASE)
VERSIONED_NAME	= $(top_srcdir)/lib/$(VERSIONED_BASE)

TARGETS		= $(NAME) $(VERSIONED_NAME)

OBJS		= Plugin.o \
                  Exception.o \
                  PluginBaseI.o \
                  OpenSSLPluginI.o \
                  SslEndpoint.o \
                  SslException.o \
                  SslConnector.o \
                  SslAcceptor.o \
		  SslTransceiver.o \
                  BaseCerts.o \
                  CertificateAuthority.o \
                  CertificateDesc.o \
                  ConfigParser.o \
                  ConfigParserErrorReporter.o \
                  SslConnection.o \
                  SslConnectionOpenSSLClient.o \
                  SslConnectionOpenSSL.o \
                  SslConnectionOpenSSLServer.o \
                  ContextOpenSSL.o \
                  ContextOpenSSLClient.o \
                  ContextOpenSSLServer.o \
                  GeneralConfig.o \
                  TempCerts.o \
                  CertificateVerifier.o \
                  CertificateVerifierOpenSSL.o \
                  OpenSSLJanitors.o \
                  RSACertificateGen.o \
                  Convert.o \
                  RSAPrivateKey.o \
                  RSAPublicKey.o \
                  RSAKeyPair.o \
                  DHParams.o \
                  OpenSSLUtils.o \
                  DefaultCertificateVerifier.o \
                  SingleCertificateVerifier.o \
                  TraceLevels.o

SRCS		= $(OBJS:.o=.cpp)

HDIR		= $(includedir)/IceSSL
SDIR		= $(slicedir)/IceSSL
SLICECMD	= $(SLICE2CPP) --ice --include-dir IceSSL --dll-export ICE_SSL_API -I$(slicedir)

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I.. $(OPENSSL_FLAGS) $(XERCESC_FLAGS) $(CPPFLAGS)

$(VERSIONED_NAME): $(OBJS)
	rm -f $@
	$(CXX) -Xlinker -soname -Xlinker $(VERSIONED_BASE) $(LDFLAGS) -shared -o $@ $(OBJS) \
	$(OPENSSL_LIBS) $(XERCESC_LIBS)

$(NAME): $(VERSIONED_NAME)
	rm -f $@
	ln -s $(VERSIONED_BASE) $@

$(HDIR)/Exception.h Exception.cpp: $(SDIR)/Exception.ice $(SLICE2CPP)
	rm -f $(HDIR)/Exception.h Exception.cpp
	$(SLICECMD) $(SDIR)/Exception.ice
	mv Exception.h $(HDIR)

clean::
	rm -f $(HDIR)/Exception.h Exception.cpp

$(HDIR)/CertificateVerifier.h CertificateVerifier.cpp: $(SDIR)/CertificateVerifier.ice $(SLICE2CPP)
	rm -f $(HDIR)/CertificateVerifier.h CertificateVerifier.cpp
	$(SLICECMD) $(SDIR)/CertificateVerifier.ice
	mv CertificateVerifier.h $(HDIR)

clean::
	rm -f $(HDIR)/CertificateVerifier.h CertificateVerifier.cpp

$(HDIR)/CertificateVerifierF.h: $(SDIR)/CertificateVerifierF.ice $(SLICE2CPP)
	rm -f $(HDIR)/CertificateVerifierF.h CertificateVerifierF.cpp
	$(SLICECMD) $(SDIR)/CertificateVerifierF.ice
	mv CertificateVerifierF.h $(HDIR)
	rm -f CertificateVerifierF.cpp

clean::
	rm -f $(HDIR)/CertificateVerifierF.h

$(HDIR)/Plugin.h Plugin.cpp: $(SDIR)/Plugin.ice $(SLICE2CPP)
	rm -f $(HDIR)/Plugin.h Plugin.cpp
	$(SLICECMD) $(SDIR)/Plugin.ice
	mv Plugin.h $(HDIR)

clean::
	rm -f $(HDIR)/Plugin.h Plugin.cpp

$(HDIR)/PluginF.h: $(SDIR)/PluginF.ice $(SLICE2CPP)
	rm -f $(HDIR)/PluginF.h PluginF.cpp
	$(SLICECMD) $(SDIR)/PluginF.ice
	mv PluginF.h $(HDIR)
	rm -f PluginF.cpp

clean::
	rm -f $(HDIR)/PluginF.h

include .depend
