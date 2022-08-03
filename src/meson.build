maple_sources = [
    'maple.c',
    'maple_server.c'
]

maple_dependencies  = [
    parson_dep,
    server_protos,
    wlroots_dep
]

executable(
    meson.project_name(),
    maple_sources,
    dependencies : maple_dependencies,
    install : true,
)
