project(
  'lua_basics',
  'c', 
  default_options : ['c_std=c23', 'buildtype=debug']
)

add_project_arguments(
  ['-Wall',
  '-Wextra',
  '-Wnull-dereference',
  '-fsanitize=address,undefined'],
  language: 'c'
)

add_project_link_arguments(
  ['-fsanitize=address,undefined'],
  language: 'c'
)

# dependency search order: pkgconf -> cmake -> wrap-subprojects
luajit_dep = dependency('luajit')
sdl3_dep = dependency('sdl3')
sdl3_image_dep = dependency('sdl3_image')
pcre2_dep = dependency('libpcre2-8')

subdir('src')
