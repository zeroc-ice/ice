# **********************************************************************
#
# Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../..

LIBFILENAME     = $(call mklibfilename,IceSSL,$(VERSION))
SONAME          = $(call mksoname,IceSSL,$(SOVERSION))  
LIBNAME		= $(call mklibname,IceSSL)

TARGETS		= $(call mklibtargets,$(libdir)/$(LIBFILENAME),$(libdir)/$(SONAME),$(libdir)/$(LIBNAME))

OBJS		= BaseCerts.o \
                  CertificateAuthority.o \
                  CertificateDesc.o \
                  CertificateVerifierF.o \
                  CertificateVerifier.o \
                  CertificateVerifierOpenSSL.o \
                  ClientContext.o \
                  ConfigParser.o \
                  Context.o \
                  Convert.o \
                  DefaultCertificateVerifier.o \
                  DHParams.o \
                  Exception.o \
                  GeneralConfig.o \
                  OpenSSLJanitors.o \
                  OpenSSLPluginI.o \
                  OpenSSLUtils.o \
    		  PluginF.o \
    		  Plugin.o \
                  RSACertificateGen.o \
                  RSAKeyPair.o \
                  RSAPrivateKey.o \
                  RSAPublicKey.o \
                  ServerContext.o \
                  SingleCertificateVerifier.o \
                  SslAcceptor.o \
		  SslClientTransceiver.o \
                  SslConnector.o \
                  SslEndpoint.o \
                  SslException.o \
		  SslServerTransceiver.o \
		  SslTransceiver.o \
                  TempCerts.o \
                  TraceLevels.o

SRCS		= $(OBJS:.o=.cpp)

SLICE_SRCS	= $(SDIR)/Exception.ice \
		  $(SDIR)/PluginF.ice \
		  $(SDIR)/Plugin.ice \
	 	  $(SDIR)/CertificateVerifierF.ice \
	 	  $(SDIR)/CertificateVerifier.ice

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
	$(call installlib,$(install_libdir),$(libdir),$(LIBFILENAME),$(SONAME),$(LIBNAME))

include .depend
