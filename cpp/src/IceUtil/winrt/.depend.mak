
$(ARCH)\$(CONFIG)\ArgVector.obj: \
	..\ArgVector.cpp \
    "..\..\..\src\IceUtil\ArgVector.h" \
    "$(includedir)\IceUtil\Config.h" \

$(ARCH)\$(CONFIG)\Cond.obj: \
	..\Cond.cpp \
    "$(includedir)\IceUtil\Cond.h" \
    "$(includedir)\IceUtil\Config.h" \
    "$(includedir)\IceUtil\Time.h" \
    "$(includedir)\IceUtil\ThreadException.h" \
    "$(includedir)\IceUtil\Exception.h" \

$(ARCH)\$(CONFIG)\ConvertUTF.obj: \
	..\ConvertUTF.cpp \
    "..\..\..\src\IceUtil\ConvertUTF.h" \
    "..\..\..\src\IceUtil\Unicode.h" \
    "$(includedir)\IceUtil\Config.h" \
    "$(includedir)\IceUtil\StringConverter.h" \
    "$(includedir)\IceUtil\Exception.h" \
    "$(includedir)\IceUtil\Shared.h" \
    "$(includedir)\IceUtil\Atomic.h" \
    "$(includedir)\IceUtil\Handle.h" \

$(ARCH)\$(CONFIG)\CountDownLatch.obj: \
	..\CountDownLatch.cpp \
    "$(includedir)\IceUtil\CountDownLatch.h" \
    "$(includedir)\IceUtil\Config.h" \
    "$(includedir)\IceUtil\ThreadException.h" \
    "$(includedir)\IceUtil\Exception.h" \
    "$(includedir)\IceUtil\Time.h" \

$(ARCH)\$(CONFIG)\Exception.obj: \
	..\Exception.cpp \
    "$(includedir)\IceUtil\Exception.h" \
    "$(includedir)\IceUtil\Config.h" \
    "$(includedir)\IceUtil\MutexPtrLock.h" \
    "$(includedir)\IceUtil\ThreadException.h" \
    "$(includedir)\IceUtil\Time.h" \
    "$(includedir)\IceUtil\Mutex.h" \
    "$(includedir)\IceUtil\Lock.h" \
    "$(includedir)\IceUtil\MutexProtocol.h" \
    "$(includedir)\IceUtil\StringUtil.h" \

$(ARCH)\$(CONFIG)\FileUtil.obj: \
	..\FileUtil.cpp \
    "$(includedir)\IceUtil\DisableWarnings.h" \
    "..\..\..\src\IceUtil\FileUtil.h" \
    "$(includedir)\IceUtil\Config.h" \
    "$(includedir)\IceUtil\Shared.h" \
    "$(includedir)\IceUtil\Atomic.h" \
    "$(includedir)\IceUtil\Handle.h" \
    "$(includedir)\IceUtil\Exception.h" \
    "$(includedir)\IceUtil\StringConverter.h" \

$(ARCH)\$(CONFIG)\InputUtil.obj: \
	..\InputUtil.cpp \
    "$(includedir)\IceUtil\InputUtil.h" \
    "$(includedir)\IceUtil\Config.h" \

$(ARCH)\$(CONFIG)\MutexProtocol.obj: \
	..\MutexProtocol.cpp \
    "$(includedir)\IceUtil\MutexProtocol.h" \
    "$(includedir)\IceUtil\Config.h" \

$(ARCH)\$(CONFIG)\Options.obj: \
	..\Options.cpp \
    "$(includedir)\IceUtil\Options.h" \
    "$(includedir)\IceUtil\Config.h" \
    "$(includedir)\IceUtil\RecMutex.h" \
    "$(includedir)\IceUtil\Lock.h" \
    "$(includedir)\IceUtil\ThreadException.h" \
    "$(includedir)\IceUtil\Exception.h" \
    "$(includedir)\IceUtil\Time.h" \
    "$(includedir)\IceUtil\MutexProtocol.h" \
    "$(includedir)\IceUtil\Shared.h" \
    "$(includedir)\IceUtil\Atomic.h" \
    "$(includedir)\IceUtil\Handle.h" \
    "$(includedir)\IceUtil\StringUtil.h" \

$(ARCH)\$(CONFIG)\OutputUtil.obj: \
	..\OutputUtil.cpp \
    "$(includedir)\IceUtil\OutputUtil.h" \
    "$(includedir)\IceUtil\Config.h" \
    "..\..\..\src\IceUtil\FileUtil.h" \
    "$(includedir)\IceUtil\Shared.h" \
    "$(includedir)\IceUtil\Atomic.h" \
    "$(includedir)\IceUtil\Handle.h" \
    "$(includedir)\IceUtil\Exception.h" \

$(ARCH)\$(CONFIG)\Random.obj: \
	..\Random.cpp \
    "$(includedir)\IceUtil\Random.h" \
    "$(includedir)\IceUtil\Config.h" \
    "$(includedir)\IceUtil\Exception.h" \
    "$(includedir)\IceUtil\Mutex.h" \
    "$(includedir)\IceUtil\Lock.h" \
    "$(includedir)\IceUtil\ThreadException.h" \
    "$(includedir)\IceUtil\Time.h" \
    "$(includedir)\IceUtil\MutexProtocol.h" \
    "$(includedir)\IceUtil\MutexPtrLock.h" \

$(ARCH)\$(CONFIG)\RecMutex.obj: \
	..\RecMutex.cpp \
    "$(includedir)\IceUtil\RecMutex.h" \
    "$(includedir)\IceUtil\Config.h" \
    "$(includedir)\IceUtil\Lock.h" \
    "$(includedir)\IceUtil\ThreadException.h" \
    "$(includedir)\IceUtil\Exception.h" \
    "$(includedir)\IceUtil\Time.h" \
    "$(includedir)\IceUtil\MutexProtocol.h" \
    "$(includedir)\IceUtil\AbstractMutex.h" \

$(ARCH)\$(CONFIG)\SHA1.obj: \
	..\SHA1.cpp \
    "$(includedir)\IceUtil\SHA1.h" \
    "$(includedir)\IceUtil\Config.h" \
    "$(includedir)\IceUtil\UniquePtr.h" \

$(ARCH)\$(CONFIG)\Shared.obj: \
	..\Shared.cpp \
    "$(includedir)\IceUtil\Shared.h" \
    "$(includedir)\IceUtil\Config.h" \
    "$(includedir)\IceUtil\Atomic.h" \

$(ARCH)\$(CONFIG)\StringConverter.obj: \
	..\StringConverter.cpp \
    "$(includedir)\IceUtil\StringConverter.h" \
    "$(includedir)\IceUtil\Config.h" \
    "$(includedir)\IceUtil\Exception.h" \
    "$(includedir)\IceUtil\Shared.h" \
    "$(includedir)\IceUtil\Atomic.h" \
    "$(includedir)\IceUtil\Handle.h" \
    "$(includedir)\IceUtil\MutexPtrLock.h" \
    "$(includedir)\IceUtil\ThreadException.h" \
    "$(includedir)\IceUtil\Time.h" \
    "$(includedir)\IceUtil\Mutex.h" \
    "$(includedir)\IceUtil\Lock.h" \
    "$(includedir)\IceUtil\MutexProtocol.h" \
    "$(includedir)\IceUtil\ScopedArray.h" \
    "$(includedir)\IceUtil\StringUtil.h" \
    "..\..\..\src\IceUtil\Unicode.h" \

$(ARCH)\$(CONFIG)\StringUtil.obj: \
	..\StringUtil.cpp \
    "$(includedir)\IceUtil\StringUtil.h" \
    "$(includedir)\IceUtil\Config.h" \
    "$(includedir)\IceUtil\StringConverter.h" \
    "$(includedir)\IceUtil\Exception.h" \
    "$(includedir)\IceUtil\Shared.h" \
    "$(includedir)\IceUtil\Atomic.h" \
    "$(includedir)\IceUtil\Handle.h" \
    "$(includedir)\IceUtil\ScopedArray.h" \

$(ARCH)\$(CONFIG)\Thread.obj: \
	..\Thread.cpp \
    "$(includedir)\IceUtil\Thread.h" \
    "$(includedir)\IceUtil\Config.h" \
    "$(includedir)\IceUtil\Shared.h" \
    "$(includedir)\IceUtil\Atomic.h" \
    "$(includedir)\IceUtil\Handle.h" \
    "$(includedir)\IceUtil\Exception.h" \
    "$(includedir)\IceUtil\Mutex.h" \
    "$(includedir)\IceUtil\Lock.h" \
    "$(includedir)\IceUtil\ThreadException.h" \
    "$(includedir)\IceUtil\Time.h" \
    "$(includedir)\IceUtil\MutexProtocol.h" \

$(ARCH)\$(CONFIG)\ThreadException.obj: \
	..\ThreadException.cpp \
    "$(includedir)\IceUtil\ThreadException.h" \
    "$(includedir)\IceUtil\Exception.h" \
    "$(includedir)\IceUtil\Config.h" \
    "$(includedir)\IceUtil\Time.h" \

$(ARCH)\$(CONFIG)\Time.obj: \
	..\Time.cpp \
    "$(includedir)\IceUtil\DisableWarnings.h" \
    "$(includedir)\IceUtil\Exception.h" \
    "$(includedir)\IceUtil\Config.h" \
    "$(includedir)\IceUtil\Time.h" \

$(ARCH)\$(CONFIG)\Timer.obj: \
	..\Timer.cpp \
    "$(includedir)\IceUtil\Timer.h" \
    "$(includedir)\IceUtil\Shared.h" \
    "$(includedir)\IceUtil\Config.h" \
    "$(includedir)\IceUtil\Atomic.h" \
    "$(includedir)\IceUtil\Thread.h" \
    "$(includedir)\IceUtil\Handle.h" \
    "$(includedir)\IceUtil\Exception.h" \
    "$(includedir)\IceUtil\Mutex.h" \
    "$(includedir)\IceUtil\Lock.h" \
    "$(includedir)\IceUtil\ThreadException.h" \
    "$(includedir)\IceUtil\Time.h" \
    "$(includedir)\IceUtil\MutexProtocol.h" \
    "$(includedir)\IceUtil\Monitor.h" \
    "$(includedir)\IceUtil\Cond.h" \

$(ARCH)\$(CONFIG)\Unicode.obj: \
	..\Unicode.cpp \
    "..\..\..\src\IceUtil\Unicode.h" \
    "$(includedir)\IceUtil\Config.h" \
    "$(includedir)\IceUtil\StringConverter.h" \
    "$(includedir)\IceUtil\Exception.h" \
    "$(includedir)\IceUtil\Shared.h" \
    "$(includedir)\IceUtil\Atomic.h" \
    "$(includedir)\IceUtil\Handle.h" \
    "..\..\..\src\IceUtil\ConvertUTF.h" \

$(ARCH)\$(CONFIG)\UUID.obj: \
	..\UUID.cpp \
    "$(includedir)\IceUtil\UUID.h" \
    "$(includedir)\IceUtil\Config.h" \
    "$(includedir)\IceUtil\Random.h" \
    "$(includedir)\IceUtil\Exception.h" \
