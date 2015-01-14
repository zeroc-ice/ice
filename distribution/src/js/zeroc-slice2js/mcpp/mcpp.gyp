{
  'targets': [
    {
      'target_name': 'mcpp',
      'product_prefix' : 'lib',
      'type': 'static_library',
      'sources':  [
        'src/directive.c',
        'src/eval.c',
        'src/expand.c',
        'src/main.c',
        'src/mbchar.c',
        'src/support.c',
        'src/system.c',

        # 'src/cc1.c',
        # 'src/preproc.c',
      ],
      'include_dirs' : [
        'src',
        'config/<(OS)/<(target_arch)'
      ],
      'defines' : [
        'HAVE_CONFIG_H',
        'MCPP_LIB=1'
      ],
      'configurations': {
        'Release': {
          'msvs_settings': {
            'VCCLCompilerTool': {
                'RuntimeLibrary': '2',
                'ExceptionHandling': '1',
                'RuntimeTypeInfo' : 'true'
              },
          },
          'msvs_disabled_warnings': [
            4018,
            4090,
            4101,
            4102,
            4133,
            4146,
            4244,
            4267
          ]
        }
      },
      'conditions': [
        ['OS=="mac"', {
          'xcode_settings': {
            "MACOSX_DEPLOYMENT_TARGET":"10.9",
            'OTHER_CFLAGS': [
              '-fno-common',
              '-stdlib=libstdc++',
              '-w'
            ]
          }
        }],
        ['OS=="linux"', {
          'cflags' : [
            '-fPIC',
            '-w'
          ]
        }]
      ]
    }
  ]
}
