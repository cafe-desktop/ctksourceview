completionwords_c_args = [
  '-DHAVE_CONFIG_H',
  '-DGTK_SOURCE_COMPILATION',
  '-DG_LOG_DOMAIN="GtkSourceView"',
]

completionwords_public_h = [
  'ctksourcecompletionwords.h',
]

completionwords_public_c = [
  'ctksourcecompletionwords.c',
  'ctksourcecompletionwordsbuffer.c',
  'ctksourcecompletionwordslibrary.c',
  'ctksourcecompletionwordsproposal.c',
  'ctksourcecompletionwordsutils.c',
]

install_headers(
  completionwords_public_h,
  install_dir: join_paths(
    pkgincludedir,
    'ctksourceview',
    'completion-providers',
    'words'
  )
)

completionwords_lib = static_library(
  package_string + 'completionwords',
  completionwords_public_c,
  include_directories: rootdir,
  dependencies: core_dep,
  c_args: completionwords_c_args,
  install: false,
)

ctksource_libs += [
  completionwords_lib
]

completionwords_dep = declare_dependency(
  link_with: completionwords_lib,
  include_directories: rootdir,
  dependencies: core_dep,
)

ctksource_deps += completionwords_dep

extra_public_sources += files([
  'ctksourcecompletionwords.c',
  'ctksourcecompletionwords.h',
])
