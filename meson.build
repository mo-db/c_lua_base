project('lua_basics', 'c', 
  default_options : ['c_std=c23', 'buildtype=debug'])

# dependency search order: pkgconf -> cmake -> wrap-subprojects
luajit_dep = dependency('luajit')
sdl3_dep = dependency('sdl3')
sdl3_image_dep = dependency('sdl3_image')

# fmt_dep = dependency('fmt')
# ask for imgui, but fall back to subproject (subprojects/imgui/meson.build)
# imgui_dep = dependency('imgui', fallback: ['imgui-1.92.3', 'imgui_dep'])

subdir('src')
