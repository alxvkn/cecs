project('cecs', 'c')

include_dirs = [
  include_directories('include'),
]

subdir('src')

libcecs = static_library('cecs', sources, include_directories : include_dirs)

subdir('test')
