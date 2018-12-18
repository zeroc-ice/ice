# **********************************************************************
#
# Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

all:
	dotnet msbuild msbuild/ice.proj /m

tests:
	dotnet msbuild msbuild/ice.proj /m

srcs:
	dotnet msbuild msbuild/ice.proj /t:BuildDist /m

distclean clean:
	dotnet msbuild msbuild/ice.proj /t:Clean /m

install::
	@echo nothing to install

test::
	@python ./allTests.py
