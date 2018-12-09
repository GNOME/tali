foreach size: [ '16x16', '22x22', '24x24', '32x32', '48x48', '256x256', '512x512']
    install_data (join_paths ('icons', size, 'tali.png'),
                  install_dir: join_paths (datadir, 'icons', 'hicolor', size, 'apps'))
endforeach
install_data (join_paths ('icons', 'symbolic', 'tali-symbolic.svg'),
              install_dir: join_paths (datadir, 'icons', 'hicolor', 'scalable', 'apps'))

install_data ([ 'gnome-dice-1.svg',
                'gnome-dice-2.svg',
                'gnome-dice-3.svg',
                'gnome-dice-4.svg',
                'gnome-dice-5.svg',
                'gnome-dice-6.svg',
                'gnome-dice-none.svg',
                'kismet1.svg',
                'kismet2.svg',
                'kismet3.svg',
                'kismet4.svg',
                'kismet5.svg',
                'kismet6.svg',
                'kismet-none.svg'],
                install_dir: pkgdatadir)

install_data ('org.gnome.tali.gschema.xml',
              install_dir: join_paths (datadir, 'glib-2.0', 'schemas'))
meson.add_install_script ('meson_compile_gschema.py')

i18n.merge_file ('desktop-file',
                 input: 'tali.desktop.in',
                 output: 'tali.desktop',
                 install: true,
                 install_dir: join_paths (datadir, 'applications'),
                 po_dir: '../po',
                 type: 'desktop')

i18n.merge_file ('appdata-file',
                 input: 'tali.appdata.xml.in',
                 output: 'tali.appdata.xml',
                 install: true,
                 install_dir: join_paths (datadir, 'metainfo'),
                 po_dir: '../po')

install_man ('tali.6')
