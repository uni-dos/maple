project(
    'maple', 'c',
    version : '0.0.0-alpha',
    meson_version: '>=0.60.2',
    default_options : [
        'c_std=c2x',
        'warning_level=2'
    ]
)


add_project_arguments(
    [
        '-DWLR_USE_UNSTABLE',
        '-D_POSIX_C_SOURCE=200809L',

    ],
    language : 'c')


inc_dir = include_directories('include')

wayland_server_dep = dependency('wayland-server')
wayland_protos_dep = dependency('wayland-protocols')

wlroots_ver = ['>=0.17.0', '<0.18.0']

# use wlroots on system if correct version else build from source
# subproject name then the dependency name
wlroots_dep = dependency('wlroots', version : wlroots_ver,
    fallback : ['wlroots', 'wlroots'],
    default_options : ['examples=false', 'default_library=static', 'xwayland=enabled'])


xkbcommon_dep = dependency('xkbcommon')
libinput_dep = dependency('libinput')

parson_dep = dependency('parson', fallback : ['parson', 'parson'],
    default_options : ['default_library=static'])

subdir('protocols')
subdir('src')


