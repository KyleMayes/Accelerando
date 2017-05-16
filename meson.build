project('accelerando', 'cpp')

# Flags

add_global_arguments('-std=c++1z', '-Wall', '-Wextra', '-pedantic', language : 'cpp')

if get_option('disable_exceptions')
    add_global_arguments('-fno-exceptions', '-DACCEL_NO_EXCEPTIONS', language : 'cpp')
endif

if get_option('disable_rtti')
    add_global_arguments('-fno-rtti', '-DACCEL_NO_RTTI', language : 'cpp')
endif

# Library

headers = include_directories('headers')
sources = [
    'sources/main.cpp',
]

accel = static_library('accel', sources, include_directories : headers)

# Examples

examples = ['example']

if get_option('examples')
    foreach example : examples
        source = 'examples/@0@.cpp'.format(example)
        executable(example, source, include_directories : headers, link_with : accel)
    endforeach
endif
