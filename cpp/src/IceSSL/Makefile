#**********************************************************************
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

top_srcdir	= ../..

NAME		= $(top_srcdir)/lib/libIceSSL.$(LIBEXT)
VERSIONED_NAME	= $(NAME).$(VERSION)

TARGETS		= $(NAME) $(VERSIONED_NAME)

OBJS		= Plugin.o \
                  Exception.o \
                  OpenSSLPluginI.o \
                  SslEndpoint.o \
                  SslException.o \
                  SslConnector.o \
                  SslAcceptor.o \
		  SslTransceiver.o \
		  SslClientTransceiver.o \
		  SslServerTransceiver.o \
                  BaseCerts.o \
                  CertificateAuthority.o \
                  CertificateDesc.o \
                  ConfigParser.o \
                  ConfigParserErrorReporter.o \
                  Context.o \
                  ClientContext.o \
                  ServerContext.o \
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

SLICE_SRCS	= $(SDIR)/Exception.ice \
		  $(SDIR)/Plugin.ice \
		  $(SDIR)/PluginF.ice \
	 	  $(SDIR)/CertificateVerifier.ice \
	 	  $(SDIR)/CertificateVerifierF.ice

HDIR		= $(includedir)/IceSSL
SDIR		= $(slicedir)/IceSSL

SLICECMD	= $(SLICE2CPP) --ice --include-dir IceSSL --dll-export ICE_SSL_API $(ICECPPFLAGS)

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I.. $(OPENSSL_FLAGS) $(XERCESC_FLAGS) $(CPPFLAGS)

$(VERSIONED_NAME): $(OBJS)
	rm -f $@
	$(CXX) $(SHLIB_LDFLAGS) $(LDFLAGS) -o $@ $(OBJS) $(OPENSSL_LIBS) $(XERCESC_LIBS)

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
