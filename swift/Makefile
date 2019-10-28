#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

top_srcdir := ..

include $(top_srcdir)/config/Make.rules

CONFIG ?= $(if $(filter $(OPTIMIZE),no),Debug,Release)
PROJECT ?= $(if $(filter $(ICE_BIN_DIST), swift all),ice-test.xcodeproj,ice.xcodeproj)

# $(call make-xcodebuild-rule,$1=rule,$2=platform,$3=scheme,$4=target)
define make-xcodebuild-rule
$1:: $(PROJECT)
	xcodebuild -project $(PROJECT) \
                   -scheme "$3 $(if $(filter $2,macosx),macOS,iOS)" \
                   -configuration $(CONFIG) \
                   -sdk $2 $4
endef

ice.xcodeproj: Rakefile | carthage
	rake icesdistproj

ice-test.xcodeproj: Rakefile carthage
	rake icebdistproj

all:: tests

# Workaround for carthage sporadic SEGFAULT (https://github.com/Carthage/Carthage/issues/2760).
# If it SEGFAULT we retry.
carthage:
	cd .. && carthage update --cache-builds || if [ $$? = 139 ]; then carthage update --cache-builds; fi

tests srcs:: carthage

install::
	@echo nothing to install

project:
	@echo Project is $(PROJECT)

$(foreach p,$(platforms),$(eval $(call make-xcodebuild-rule,srcs,$p,Ice,build)))
$(foreach p,$(platforms),$(eval $(call make-xcodebuild-rule,tests,$p,TestDriver,build)))

$(foreach p,$(platforms),$(eval $(call make-xcodebuild-rule,clean,$p,Ice,clean)))
$(foreach p,$(platforms),$(eval $(call make-xcodebuild-rule,clean,$p,TestDriver,clean)))

$(foreach p,$(supported-platforms),$(eval $(call make-xcodebuild-rule,distclean,$p,Ice,clean)))
$(foreach p,$(supported-platforms),$(eval $(call make-xcodebuild-rule,distclean,$p,TestDriver,clean)))

test::
	@python ./allTests.py --config $(CONFIG)
