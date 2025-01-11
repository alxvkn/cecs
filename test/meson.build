tests = executable(
  'tests', 'test.c',
  include_directories : include_dirs,
  link_with : libcecs,
  link_args : '-lm',
  dependencies: dependency('sdl2'),
)

test('tests', tests)
