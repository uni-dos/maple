project(
    'maple', 'c',
    version : '0.0.0-alpha',
    meson_version: '>=0.60.2',
    default_options : [
        'c_std=c17',
        'warning_level=2'
    ]
)


add_project_arguments(
    [
        '-DWLR_USE_UNSTABLE'
    
    ], 
    language : 'c')

wayland_server_dep = dependency('wayland-server')
wayland_protos_dep = dependency('wayland-protocols')

wlroots_ver = ['>=0.15.0', '<0.16.0']

# use wlroots on system if correct version else build from source
# subproject name then the dependency name
wlroots_dep = dependency('wlroots', version : wlroots_ver, 
    fallback : ['wlroots', 'wlroots'], 
    default_options : ['examples=false', 'default_library=static'])

# will either use cairo or thorvg for graphics

xkbcommon_dep = dependency('xkbcommon')

parson_dep = dependency('parson', fallback : ['parson', 'parson'],
    default_options : ['default_library=static'])

subdir('protocols')
subdir('src')


