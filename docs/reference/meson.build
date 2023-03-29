glib_prefix = glib_dep.get_pkgconfig_variable('prefix')
glib_docpath = join_paths(glib_prefix, 'share', 'ctk-doc', 'html')

ctk_prefix = ctk_dep.get_pkgconfig_variable('prefix')
ctk_docpath = join_paths(ctk_prefix, 'share', 'ctk-doc', 'html')

reference_private_h = [
  'config.h',
  'ctksource.h',
  'ctksourcebuffer-private.h',
  'ctksourcebufferinputstream.h',
  'ctksourcebufferinternal.h',
  'ctksourcebufferoutputstream.h',
  'ctksourcecompletioncontainer.h',
  'ctksourcecompletionmodel.h',
  'ctksourcecompletion-private.h',
  'ctksourcecompletionwordsbuffer.h',
  'ctksourcecompletionwordslibrary.h',
  'ctksourcecompletionwordsproposal.h',
  'ctksourcecompletionwordsutils.h',
  'ctksourcecontextengine.h',
  'ctksourceencoding-private.h',
  'ctksourceengine.h',
  'ctksourcegutter-private.h',
  'ctksourcegutterrendererlines.h',
  'ctksourcegutterrenderermarks.h',
  'ctksourcegutterrenderer-private.h',
  'ctksourceiter.h',
  'ctksourcelanguage-private.h',
  'ctksourcemarkssequence.h',
  'ctksourcepixbufhelper.h',
  'ctksourceregex.h',
  'ctksourcestyle-private.h',
  'ctksourcetypes-private.h',
  'ctksourceundomanagerdefault.h',
  'ctksourceutils-private.h',
]

reference_content_files = files([
  'lang-tutorial.xml',
  'porting-guide-3-to-4.xml',
])

reference_sources = [
  join_paths(srcdir, 'ctksourceview'),
  join_paths(builddir, 'ctksourceview'),
]

into_xml = configure_file(
         input: 'intro.xml.in',
         output: 'intro.xml',
  configuration: config_h
)

lang_reference_xml = configure_file(
          input: 'lang-reference.xml.in',
         output: 'lang-reference.xml',
  configuration: config_h
)

style_reference_xml = configure_file(
          input: 'style-reference.xml.in',
         output: 'style-reference.xml',
  configuration: config_h
)

reference_content_files += [
  into_xml,
  lang_reference_xml,
  style_reference_xml,
]

ctksourceview_docs_xml = configure_file(
          input: 'ctksourceview-docs.xml.in',
         output: 'ctksourceview-docs.xml',
  configuration: config_h
)

reference_scan_args = [
  '--rebuild-types',
  '--ignore-decorators=GTK_SOURCE_DEPRECATED\w*\s*\([^)]*\)|GTK_SOURCE_DEPRECATED\w*|GTK_SOURCE_AVAILABLE\w*|_GTK_SOURCE_EXTERN',
  '--deprecated-guards=GTKSOURCEVIEW_DISABLE_DEPRECATED',
  '--ignore-headers=' + ' '.join(reference_private_h),
]

reference_fixxref_args = [
  '--html-dir=@0@'.format(docpath),
  '--extra-dir=@0@'.format(join_paths(glib_docpath, 'glib')),
  '--extra-dir=@0@'.format(join_paths(glib_docpath, 'gobject')),
  '--extra-dir=@0@'.format(join_paths(glib_docpath, 'gio')),
  '--extra-dir=@0@'.format(join_paths(ctk_docpath, 'ctk')),
]

gnome.ctkdoc('@0@.0'.format(package_string),
           main_xml: join_paths(builddir, '@0@'.format(ctksourceview_docs_xml)),
  gobject_typesfile: 'ctksourceview-4.0.types',
            src_dir: reference_sources,
       dependencies: ctksource_dep,
      content_files: reference_content_files,
          scan_args: reference_scan_args,
       fixxref_args: reference_fixxref_args,
            install: true
)
