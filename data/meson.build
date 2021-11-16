install_data (join_paths ('icons', 'scalable', project_id + '.svg'),
              install_dir: join_paths (datadir, 'icons', 'hicolor', 'scalable', 'apps'))
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

desktop_file = i18n.merge_file (
                 input: project_id + '.desktop.in',
                 output: project_id + '.desktop',
                 install: true,
                 install_dir: join_paths (datadir, 'applications'),
                 po_dir: '../po',
                 type: 'desktop')

desktop_file_validate = find_program('desktop-file-validate', required: false)
if desktop_file_validate.found()
  test(
    'validate-desktop',
    desktop_file_validate,
    args: [
      desktop_file.full_path()
    ]
  )
endif

appdata_file = i18n.merge_file (
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
