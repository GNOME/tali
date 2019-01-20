foreach size: [ '16x16', '22x22', '24x24', '32x32', '48x48', '256x256', '512x512']
    install_data (join_paths ('icons', size, project_id + '.png'),
                  install_dir: join_paths (datadir, 'icons', 'hicolor', size, 'apps'))
endforeach
install_data (join_paths ('icons', 'symbolic', project_id + '-symbolic.svg'),
              install_dir: join_paths (datadir, 'icons', 'hicolor', 'symbolic', 'apps'))

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

install_data (project_id + '.gschema.xml',
              install_dir: join_paths (datadir, 'glib-2.0', 'schemas'))
meson.add_install_script ('meson_compile_gschema.py')

desktop_file = i18n.merge_file ('desktop-file',
                 input: project_id + '.desktop.in',
                 output: project_id + '.desktop',
                 install: true,
                 install_dir: join_paths (datadir, 'applications'),
                 po_dir: '../po',
                 type: 'desktop')

if desktop_file_validate.found()
  test(
    'validate-desktop',
    desktop_file_validate,
    args: [
      desktop_file.full_path()
    ]
  )
endif

appdata_file = i18n.merge_file ('appdata-file',
                 input: project_id + '.appdata.xml.in',
                 output: project_id + '.appdata.xml',
                 install: true,
                 install_dir: join_paths (datadir, 'metainfo'),
                 po_dir: '../po')

appstream_util = find_program('appstream-util', required: false)
if appstream_util.found()
  test(
    'validate-appdata', appstream_util,
    args: [
      'validate-relax', '--nonet', appdata_file.full_path()
    ]
  )
endif

install_man ('tali.6')
