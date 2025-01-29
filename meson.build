project('resize-rootfs',
	['cpp'],
	version: '0.1.0',
	default_options: [
		'werror=true',
		'warning_level=3',
	]
)

src = [
	'resize-rootfs/exec.cpp',
	'resize-rootfs/fdisk.cpp',
	'resize-rootfs/main.cpp',
	'resize-rootfs/mount.cpp',
	'resize-rootfs/process.cpp',
	'resize-rootfs/readable.cpp',
	'resize-rootfs/utils.cpp',
]

libfdisk = dependency('fdisk', required: true)
libmount = dependency('mount', required: true)
libfmt = dependency('fmt', required: true)
systemd = dependency('systemd', required: false)

executable(
	'resize-rootfs',
	src,
	dependencies: [
		libfdisk,
		libmount,
		libfmt,
	],
	link_language: 'cpp',
	install: true,
)

if systemd.found()
	systemd_system_unit_dir = systemd.get_variable('systemdsystemunitdir')
else
	systemd_system_unit_dir = get_option('libdir') / 'systemd' / 'system'
endif

install_data(
	'resize-rootfs.service',
	install_mode: 'rw-r--r--',
	install_dir: systemd_system_unit_dir,
)
