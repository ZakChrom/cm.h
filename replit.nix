{ pkgs }: {
	deps = [
        pkgs.imagemagick6_light
        pkgs.wget
        pkgs.curl.dev
        pkgs.python39Full
        pkgs.clang_12
		pkgs.ccls
		pkgs.gdb
		pkgs.gnumake
	];
}