{
  'target_defaults': {
    'defines': [
      'ICE_STATIC_LIBS',
      'SLICE_API_EXPORTS',
    ],
    'conditions': [
      ['OS=="win"', {
        'msvs_disabled_warnings': [
          4273,  # inconsistent dll linkage
          4250
        ],
      }]
    ]
  },

  'targets': [
    {
      'target_name': 'slice2js',
      'type': 'executable',
      'dependencies' : ['slice', 'iceutil'],

      'configurations': {
        'Release': {
          'msvs_settings': {
            'VCCLCompilerTool': {
                'RuntimeLibrary': '2',
                'ExceptionHandling': '1',
                'RuntimeTypeInfo' : 'true',
                'WarnAsError' : 'true'
              },
          },
        },
      },
      'sources':  [
        'src/slice2js/Gen.cpp',
        'src/slice2js/JsUtil.cpp',
        'src/slice2js/Main.cpp'
      ],
      'include_dirs' : [
          'include',
          'src/slice2js'
      ],
      'cflags_cc' : [
        '-fexceptions'
      ],
      'cflags_cc!' : [
        '-fno-rtti'
      ],
      'conditions': [
        ['OS=="win"', {
          'libraries': [
            '-lrpcrt4.lib', '-ladvapi32.lib', '-lDbgHelp.lib'
          ]
        }]
      ],
      'xcode_settings': {
        'GCC_ENABLE_CPP_RTTI': 'YES',
        "GCC_ENABLE_CPP_EXCEPTIONS": "YES",
        "MACOSX_DEPLOYMENT_TARGET":"10.9"
      }
    },

    {
      'target_name': 'slice',
      'type': 'static_library',
      'dependencies': [
        'mcpp/mcpp.gyp:mcpp',
        'iceutil'
      ],
      'configurations': {
        'Release': {
          'msvs_settings': {
            'VCCLCompilerTool': {
                'RuntimeLibrary': '2',
                'ExceptionHandling': '1',
                'RuntimeTypeInfo' : 'true',
                'WarnAsError' : 'true'
              }
          }
        }
      },
      'sources':  [
        'src/Slice/CPlusPlusUtil.cpp',
        'src/Slice/DotNetNames.cpp',
        'src/Slice/JavaUtil.cpp',
        'src/Slice/PHPUtil.cpp',
        'src/Slice/PythonUtil.cpp',
        'src/Slice/Util.cpp',
        'src/Slice/Checksum.cpp',
        'src/Slice/FileTracker.cpp',
        'src/Slice/MD5.cpp',
        'src/Slice/Parser.cpp',
        'src/Slice/RubyUtil.cpp',
        'src/Slice/CsUtil.cpp',
        'src/Slice/Grammar.cpp',
        'src/Slice/MD5I.cpp',
        'src/Slice/Preprocessor.cpp',
        'src/Slice/Scanner.cpp'
      ],
      'include_dirs' : [
          'include',
          'src'
      ],
      'cflags_cc' : [
        '-fexceptions'
      ],
      'cflags_cc!' : [
        '-fno-rtti'
      ],
      'xcode_settings': {
        'GCC_ENABLE_CPP_RTTI': 'YES',
        "GCC_ENABLE_CPP_EXCEPTIONS": "YES",
        "MACOSX_DEPLOYMENT_TARGET":"10.9"
      }
    },

    {
      'target_name': 'iceutil',
      'type': 'static_library',
      'configurations': {
        'Release': {
          'msvs_settings': {

            'VCCLCompilerTool': {
                'RuntimeLibrary': '2',
                'ExceptionHandling': '1',
                'RuntimeTypeInfo' : 'true',
                'WarnAsError' : 'true'
              },
          },
        },
      },
      'sources':  [
        'src/IceUtil/ArgVector.cpp',
        'src/IceUtil/Cond.cpp',
        'src/IceUtil/ConvertUTF.cpp',
        'src/IceUtil/CountDownLatch.cpp',
        'src/IceUtil/CtrlCHandler.cpp',
        'src/IceUtil/Exception.cpp',
        'src/IceUtil/FileUtil.cpp',
        'src/IceUtil/InputUtil.cpp',
        'src/IceUtil/MutexProtocol.cpp',
        'src/IceUtil/Options.cpp',
        'src/IceUtil/OutputUtil.cpp',
        'src/IceUtil/Random.cpp',
        'src/IceUtil/RecMutex.cpp',
        'src/IceUtil/SHA1.cpp',
        'src/IceUtil/Shared.cpp',
        'src/IceUtil/StringConverter.cpp',
        'src/IceUtil/StringUtil.cpp',
        'src/IceUtil/Thread.cpp',
        'src/IceUtil/ThreadException.cpp',
        'src/IceUtil/Time.cpp',
        'src/IceUtil/Timer.cpp',
        'src/IceUtil/Unicode.cpp',
        'src/IceUtil/UUID.cpp'
      ],
      'include_dirs' : [
          "src",
          "include",
      ],
      'cflags_cc' : [
        '-fexceptions'
      ],
      'cflags_cc!' : [
        '-fno-rtti'
      ],
      'xcode_settings': {
        'GCC_ENABLE_CPP_RTTI': 'YES',
        "GCC_ENABLE_CPP_EXCEPTIONS": "YES",
        "MACOSX_DEPLOYMENT_TARGET":"10.9"
      }
    }
  ]
}
