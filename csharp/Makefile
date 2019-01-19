#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

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
