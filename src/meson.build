raider_sources = [
  'main.c',
  'raider-window.c',
  'raider-window.h',
  'raider.c',
  'raider.h',
  'raider-preferences.c',
  'raider-preferences.h',
  'raider-file-row.c',
  'raider-file-row.h'
]

raider_deps = [
  dependency('gio-2.0', version: '>= 2.50'),
  dependency('gtk+-3.0', version: '>= 3.22'),
  dependency('libhandy-1', version: '>= 1.1.90'),
  dependency('libdazzle-1.0', version: '>= 3.37.0')
]

gnome = import('gnome')

raider_sources += gnome.compile_resources('raider-resources',
  'raider.gresource.xml',
  c_name: 'raider'
)

executable('raider', raider_sources,
  dependencies: raider_deps,
  install: true,
)
