#**********************************************************************
#
# Copyright (c) 2003
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

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I.. $(OPENSSL_FLAGS) $(XERCESC_FLAGS) $(CPPFLAGS)
SLICE2CPPFLAGS	:= --ice --include-dir IceSSL --dll-export ICE_SSL_API $(SLICE2CPPFLAGS)

$(VERSIONED_NAME): $(OBJS)
	rm -f $@
	$(CXX) $(SHLIB_LDFLAGS) $(LDFLAGS) -o $@ $(OBJS) $(OPENSSL_LIBS) $(XERCESC_LIBS)

install:: all
	$(INSTALL_LIBRARY) $(VERSIONED_NAME) $(install_libdir)
	rm -f $(install_libdir)/$(notdir $(NAME))
	ln -s $(notdir $(VERSIONED_NAME)) $(install_libdir)/$(notdir $(NAME))

include .depend
