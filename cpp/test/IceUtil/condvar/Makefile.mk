# **********************************************************************
#
# Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

$(test)_programs            = workqueue match

$(test)_workqueue_sources   = WorkQueue.cpp
$(test)_match_sources       = Match.cpp

tests += $(test)
