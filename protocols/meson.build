# labwc and wayfire use this to generate the protocol files. This seems a bit cleaner compared to vivarium

# seems to be /usr/share/wayland-protocols
wl_protocol_dir = wayland_protos_dep.get_variable(pkgconfig: 'pkgdatadir') # location of the protocol xml files

wayland_scanner = find_program('wayland-scanner') # this will generate c/h files from the xml protocols

# define how to generate the header files for the protocol
wayland_scanner_server = generator(
	wayland_scanner,
	output: '@BASENAME@-protocol.h',
	arguments: ['server-header', '@INPUT@', '@OUTPUT@'],
)

# define how to generate the c (source) code for the protocol
wayland_scanner_code = generator(
	wayland_scanner,
	output: '@BASENAME@-protocol.c',
	arguments: ['private-code', '@INPUT@', '@OUTPUT@'],
)

# define the protocols we want
# wlr protocols need to manually be added
server_protocols = [
	[wl_protocol_dir, 'stable/xdg-shell/xdg-shell.xml'],
	[wl_protocol_dir, 'staging/ext-idle-notify/ext-idle-notify-v1.xml'],
	[wl_protocol_dir, 'staging/ext-session-lock/ext-session-lock-v1.xml'],
	[wl_protocol_dir, 'unstable/pointer-constraints/pointer-constraints-unstable-v1.xml'],
	['wlr-gamma-control-unstable-v1.xml'],
	['wlr-layer-shell-unstable-v1.xml'],
	['wlr-output-management-unstable-v1.xml'],
	['wlr-output-power-management-unstable-v1.xml']
]

server_protos_src = []
server_protos_headers = []

# actually generate the headers and sources
foreach proto : server_protocols
	xml = join_paths(proto)
	server_protos_src += wayland_scanner_code.process(xml)
	server_protos_headers += wayland_scanner_server.process(xml)
endforeach

lib_server_protos = static_library(
	'wl_server_protos', # name of the dependency
	server_protos_src + server_protos_headers,
	dependencies: [wayland_server_dep]
)

# need to make sure the project can locate & use the created code
server_protos = declare_dependency(
	link_with: lib_server_protos,
	sources: server_protos_headers,
)
