project('rz-t8x', 'c')

rz_asm_dep = dependency('rz_asm')
rizin = find_program('rizin', required: true)
plugins_dir = run_command(rizin, '-H', 'RZ_USER_PLUGINS', check: true).stdout().strip()
# Uncomment line below to install to system instead of user home
# plugins_dir = get_option('prefix') / rz_asm_dep.get_variable(pkgconfig: 'plugindir', cmake: 'rz_asm_PLUGINDIR')
message('Plugins install directory: ' + plugins_dir)
incdir = include_directories('libz/asm/arch/t8x')

library('t8x_asm',
  ['libz/asm/arch/t8x/t8x_disas.c'],
  dependencies: [rz_asm_dep],
  install: true,
  install_dir: plugins_dir,
)

library('t8x_analsys',
  ['libz/analysis/p/analysis_t8x.c'],
  dependencies: [rz_asm_dep],
  install: true,
  install_dir: plugins_dir,
  include_directories: incdir
)
