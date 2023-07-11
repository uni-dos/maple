maple_sources = [
    'cursor.c',
    'maple.c',
    'output.c',
    'seat.c',
    'view.c',
    'xdg.c',
    'xwayland.c',
]

maple_dependencies  = [
    libinput_dep,
    parson_dep,
    server_protos,
    wlroots_dep
]

executable(
    meson.project_name(),
    maple_sources,
    include_directories : [inc_dir],
    dependencies : maple_dependencies,
    install : true,
)
