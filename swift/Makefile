#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

CONFIG ?= Debug

all:
	cd .. && carthage update --cache-builds
	xcodebuild -project ice.xcodeproj -scheme "TestDriver macOS" -configuration $(CONFIG) build

distclean clean:
	xcodebuild -project ice.xcodeproj -scheme "TestDriver macOS" -configuration $(CONFIG) clean

install::
	@echo nothing to install

test::
	@python ./allTests.py --config $(CONFIG)
