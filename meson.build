project('qemacs  ', 'c')

sources = files([
    'qe.c',
    'parser.c',
    'charset.c',
    'buffer.c',
    'input.c',
    'display.c',
    'util.c',
    'hex.c',
    'list.c',
    'cutils.c'
])

if get_option('tiny').disabled()
    sources += files(['extras.c', 'variables.c', 'charsetjis.c', 'charsetmore.c'])
endif

# all other modes
sources += files([
    'unihex.c',
    'bufed.c',
    'clang.c',
    'htmlsrc.c',
    'lisp.c',
    'makemode.c',
    'markdown.c',
    'orgmode.c',
    'perl.c',
    'script.c',
    'extra-modes.c'])

if host_machine.system() != 'windows'
    sources += files(['shell.c', 'dired.c', 'latex-mode.c', 'archive.c'])
endif

conf_data = configuration_data()
conf_data.set_quoted('QE_VERSION', '5.0')
conf_data.set_quoted('CONFIG_QE_DATADIR', '/')
conf_data.set_quoted('CONFIG_QE_PREFIX', '/')


configure_file(input: 'config.h.in',
               output: 'config.h',
               configuration : conf_data)
configuration_inc = include_directories('.')

treesitter_lang_deps = []
if get_option('treesitter').enabled()
    treesitter_dep = dependency('tree-sitter', required: true)
    if not treesitter_dep.found()
      treesitter = subproject('tree-sitter')
    endif

    if treesitter_dep.found()
        sources += files(['treesitter.c'])
    endif

    foreach lang : ['go', 'json']
      treesitter_lang_deps += dependency('tree-sitter-@0@'.format(lang),
                                         fallback : ['tree-sitter-@0@'.format(lang),
                                                     'tree_sitter_@0@_dep'.format(lang)])
    endforeach
endif

gtkdep = dependency('', required: false)
if get_option('gtk').enabled()
    gtkdep = dependency('gtk+-3.0', required: true)
    if gtkdep.found()
        sources += files(['gtk.c'])
    endif
else
    if host_machine.system() == 'linux'
        sources += files(['unix.c'])
    endif
endif

if host_machine.system() == 'linux'
    sources += files(['tty.c'])
endif

# this has to be done once all source files are known
# FIXME do this properly
grep = find_program('grep')
modinit_gen = custom_target('modinit_gen',
                        command: [grep, '-h', '^qe_module_init', sources],
                        output : 'allmodules.txt',
                        capture: true)
sources += files(['qeend.c'])

executable('qe',
           [sources, modinit_gen],
           dependencies : [gtkdep, treesitter_dep] + treesitter_lang_deps,
           include_directories : configuration_inc,
           c_args : '-DHAVE_QE_CONFIG_H')
