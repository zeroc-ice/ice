# **********************************************************************
#
# Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../..

LIBFILENAME     = $(call mklibfilename,IceSSL,$(VERSION))
SONAME          = $(call mksoname,IceSSL,$(SOVERSION))  
LIBNAME		= $(call mklibname,IceSSL)

TARGETS		= $(libdir)/$(LIBFILENAME) $(libdir)/$(SONAME) $(libdir)/$(LIBNAME)

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

CPPFLAGS	:= -I.. $(CPPFLAGS) -DICE_SSL_API_EXPORTS $(OPENSSL_FLAGS)
SLICE2CPPFLAGS	:= --ice --include-dir IceSSL --dll-export ICE_SSL_API $(SLICE2CPPFLAGS)

LINKWITH        := -lIceXML -lIce -lIceUtil $(OPENSSL_LIBS)

$(libdir)/$(LIBFILENAME): $(OBJS)
	rm -f $@
	$(call mkshlib,$@,$(SONAME),$(OBJS),$(LINKWITH))

$(libdir)/$(SONAME): $(libdir)/$(LIBFILENAME)
	rm -f $@
	ln -s $(LIBFILENAME) $@

$(libdir)/$(LIBNAME): $(libdir)/$(SONAME)
	rm -f $@
	ln -s $(SONAME) $@

install:: all
	$(INSTALL_LIBRARY) $(libdir)/$(LIBFILENAME) $(install_libdir)
	rm -f $(install_libdir)/$(SONAME)
	ln -s $(LIBFILENAME) $(install_libdir)/$(SONAME)
	rm -f $(install_libdir)/$(LIBNAME)
	ln -s $(SONAME) $(install_libdir)/$(LIBNAME)

include .depend
