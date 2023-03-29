core_marshallers = gnome.genmarshal('ctksource-marshal',
             sources: ['ctksourcemarshalers.list'],
              prefix: '_ctk_source_marshal',
         skip_source: true,
  valist_marshallers: true,
)

core_public_h = files([
  'ctksource.h',
  'ctksourceautocleanups.h',
  'ctksourcebuffer.h',
  'ctksourcecompletion.h',
  'ctksourcecompletioncontext.h',
  'ctksourcecompletioninfo.h',
  'ctksourcecompletionitem.h',
  'ctksourcecompletionproposal.h',
  'ctksourcecompletionprovider.h',
  'ctksourceencoding.h',
  'ctksourcefile.h',
  'ctksourcefileloader.h',
  'ctksourcefilesaver.h',
  'ctksourcegutter.h',
  'ctksourcegutterrenderer.h',
  'ctksourcegutterrendererpixbuf.h',
  'ctksourcegutterrenderertext.h',
  'ctksourceinit.h',
  'ctksourcelanguage.h',
  'ctksourcelanguagemanager.h',
  'ctksourcemap.h',
  'ctksourcemark.h',
  'ctksourcemarkattributes.h',
  'ctksourceprintcompositor.h',
  'ctksourceregion.h',
  'ctksourcesearchcontext.h',
  'ctksourcesearchsettings.h',
  'ctksourcespacedrawer.h',
  'ctksourcestyle.h',
  'ctksourcestylescheme.h',
  'ctksourcestyleschemechooser.h',
  'ctksourcestyleschemechooserbutton.h',
  'ctksourcestyleschemechooserwidget.h',
  'ctksourcestyleschememanager.h',
  'ctksourcetag.h',
  'ctksourcetypes.h',
  'ctksourceundomanager.h',
  'ctksourceutils.h',
  'ctksourceview.h',
])

core_public_c = files([
  'ctksourcebuffer.c',
  'ctksourcecompletion.c',
  'ctksourcecompletioncontext.c',
  'ctksourcecompletioninfo.c',
  'ctksourcecompletionitem.c',
  'ctksourcecompletionproposal.c',
  'ctksourcecompletionprovider.c',
  'ctksourceencoding.c',
  'ctksourcefile.c',
  'ctksourcefileloader.c',
  'ctksourcefilesaver.c',
  'ctksourcegutter.c',
  'ctksourcegutterrenderer.c',
  'ctksourcegutterrendererpixbuf.c',
  'ctksourcegutterrenderertext.c',
  'ctksourceinit.c',
  'ctksourcelanguage.c',
  'ctksourcelanguagemanager.c',
  'ctksourcemap.c',
  'ctksourcemark.c',
  'ctksourcemarkattributes.c',
  'ctksourceprintcompositor.c',
  'ctksourceregion.c',
  'ctksourcesearchcontext.c',
  'ctksourcesearchsettings.c',
  'ctksourcespacedrawer.c',
  'ctksourcestyle.c',
  'ctksourcestylescheme.c',
  'ctksourcestyleschemechooser.c',
  'ctksourcestyleschemechooserbutton.c',
  'ctksourcestyleschemechooserwidget.c',
  'ctksourcestyleschememanager.c',
  'ctksourcetag.c',
  'ctksourceundomanager.c',
  'ctksourceutils.c',
  'ctksourceversion.c',
  'ctksourceview.c',
])

core_private_c = files([
  'ctksourcebufferinputstream.c',
  'ctksourcebufferinternal.c',
  'ctksourcebufferoutputstream.c',
  'ctksourcecompletioncontainer.c',
  'ctksourcecompletionmodel.c',
  'ctksourcecontextengine.c',
  'ctksourceengine.c',
  'ctksourcegutterrendererlines.c',
  'ctksourcegutterrenderermarks.c',
  'ctksourceiter.c',
  'ctksourcelanguage-parser-2.c',
  'ctksourcemarkssequence.c',
  'ctksourcepixbufhelper.c',
  'ctksourceregex.c',
  'ctksourceundomanagerdefault.c',
])

core_c_args = [
  '-DHAVE_CONFIG_H',
  '-DCTK_SOURCE_COMPILATION',
  '-DG_LOG_DOMAIN="CtkSourceView"',
]

core_deps = [
  libm_dep,
  fribidi_dep,
  glib_dep,
  gobject_dep,
  gio_dep,
  ctk_dep,
  libxml_dep,
]

if config_h.has('OS_OSX')
  core_c_args += '-xobjective-c'
  core_deps += [
    cocoa_dep,
  ]
endif

if cc.get_id() == 'msvc' and cc.version().version_compare('<18')
  # include our math.h to implement round() for pre-2013 Visual Studio
  ctksourceview_extra_include_dirs = include_directories('../msvc')
else
  ctksourceview_extra_include_dirs = []
endif

ctksourceview_include_dirs = [rootdir, ctksourceview_extra_include_dirs]

core_enums_header = '''

#if defined (CTK_SOURCE_COMPILATION) && defined (HAVE_CONFIG_H)
# include <config.h>
#endif

#if !defined (CTK_SOURCE_H_INSIDE) && !defined (CTK_SOURCE_COMPILATION)
# error "Only <ctksourceview/ctksource.h> can be included directly."
#endif

#include <ctksourceview/ctksourceversion.h>
'''

core_enums = gnome.mkenums_simple('ctksource-enumtypes',
            sources: core_public_h,
  identifier_prefix: 'CtkSource',
      symbol_prefix: 'ctk_source',
      header_prefix: core_enums_header,
          decorator: 'CTK_SOURCE_ENUM_EXTERN',
     install_header: true,
        install_dir: join_paths(pkgincludedir, 'ctksourceview'),
)

version_h = configuration_data()
version_h.set('CTK_SOURCE_MAJOR_VERSION', version_major)
version_h.set('CTK_SOURCE_MINOR_VERSION', version_minor)
version_h.set('CTK_SOURCE_MICRO_VERSION', version_micro)

ctksourceversion_h = configure_file(
          input: 'ctksourceversion.h.in',
         output: 'ctksourceversion.h',
  configuration: version_h,
    install_dir: join_paths(pkgincludedir, 'ctksourceview'),
)

ctksource_res = gnome.compile_resources(
  'ctksourceview-gresources',
  'ctksourceview.gresource.xml'
)

core_sources = [
  core_public_c,
  core_private_c,
  core_enums,
  ctksourceversion_h,
  core_marshallers,
  ctksource_res,
]

install_headers(
  core_public_h,
  install_dir: join_paths(pkgincludedir, 'ctksourceview'),
)

core_enums_h = core_enums.get(1)

# Unfortunately, Visual Studio builds must build the core
# sources twice, once for the CtkSourceView DLL, and once for
# the tests (static core lib), so that we can ensure that
# all the items from the core sources are properly linked into
# the final CtkSourceView DLL and exported from it.
core_lib = static_library(package_string + 'core', core_sources,
  include_directories: ctksourceview_include_dirs,
         dependencies: core_deps,
               c_args: core_c_args,
              install: false
)

core_dep = declare_dependency(
            link_with: core_lib,
  include_directories: rootdir,
         dependencies: core_deps,
              sources: [core_enums_h]
)

ctksource_deps = [
  core_dep,
]

ctksource_libs = [core_lib]

extra_public_sources = []

subdir('completion-providers')


# We can't use the static libs on Visual Studio builds
# to form our DLL here directly, so we must use
# extract_all_objects() to extract all the core and word
# completion provider objects that we previously built
# into static .lib's and link them into the main DLL
# instead
ctksource_objs = []
foreach int_lib : ctksource_libs
  ctksource_objs += int_lib.extract_all_objects()
endforeach

ctksource_lib = shared_library(package_string,
                version: lib_version,
        darwin_versions: lib_osx_version,
    include_directories: ctksourceview_include_dirs,
           dependencies: cc.get_id() == 'msvc' ? core_deps : ctksource_deps,
                objects: cc.get_id() == 'msvc' ? ctksource_objs : [],
             link_whole: cc.get_id() == 'msvc' ? [] : ctksource_libs,
                 c_args: core_c_args,
              link_args: release_link_args,
                install: true,
  gnu_symbol_visibility: 'hidden',
)

ctksource_dep_sources = [
  core_enums_h,
]

if generate_gir
  ctksource_gir_sources = [
    core_public_c,
    core_public_h,
    core_enums_h,
    extra_public_sources,
  ]

  ctksource_gir = gnome.generate_gir(ctksource_lib,
                sources: ctksource_gir_sources,
              nsversion: api_version,
              namespace: 'CtkSource',
          symbol_prefix: 'ctk_source',
      identifier_prefix: 'CtkSource',
        export_packages: [package_string],
              link_with: ctksource_lib,
               includes: [ 'Gdk-3.0', 'Ctk-3.0' ],
                install: true,
        install_dir_gir: girdir,
    install_dir_typelib: typelibdir,
             extra_args: [ '--c-include=ctksourceview/ctksource.h',
	                   '--warn-all' ],
  )

  ctksource_dep_sources += [
    ctksource_gir,
  ]

  if generate_vapi
    configure_file(
        input: 'CtkSource.metadata.in',
       output: 'CtkSource-@0@.metadata'.format(api_version),
         copy: true,
    )

    ctksource_vapi = gnome.generate_vapi(package_string,
            sources: ctksource_gir.get(0),
      metadata_dirs: join_paths(builddir, 'ctksourceview'),
            install: true,
        install_dir: vapidir,
           packages: [ 'atk',
                       'gdk-3.0',
		       'gdk-pixbuf-2.0',
		       'gio-2.0',
		       'ctk+-3.0',
		       'pango',
		       'cairo' ],
    )
  endif
endif

# MSVC builds: Only link to the dependent packages/.lib's
ctksource_dep = declare_dependency(
            link_with: ctksource_lib,
  include_directories: rootdir,
         dependencies: cc.get_id() == 'msvc' ? core_deps : ctksource_deps,
              sources: ctksource_dep_sources,
)

ctksource_pc_reqs = [
  'glib-2.0 @0@'.format(glib_req),
  'gobject-2.0 @0@'.format(glib_req),
  'gio-2.0 @0@'.format(glib_req),
  'ctk+-3.0 @0@'.format(ctk_req),
]

ctksource_pc_private_reqs = []
ctksource_lib_private = []

if libxml_dep.type_name() == 'pkgconfig'
  ctksource_pc_private_reqs += [
    'libxml-2.0 @0@'.format(libxml_req)
  ]
else
  if libxml_dep.type_name() == 'library'
    ctksource_lib_private += libxml_dep
  endif
endif

ctksource_pc_private_reqs += [
  'fribidi @0@'.format(fribidi_req),
]

ctksource_pc = pkg.generate(
              name: package_string,
       description: 'Source code editing Ctk+ widget.',
          requires: ctksource_pc_reqs,
  requires_private: ctksource_pc_private_reqs,
           version: version,
           subdirs: package_string,
         libraries: ctksource_lib,
 libraries_private: ctksource_lib_private,
       install_dir: pkgconfigdir,
)
