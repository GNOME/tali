resources = gnome.compile_resources ('resources', 'tali.gresource.xml',
                                     source_dir: '.',
                                     c_name: 'resources')

executable ('tali',
            [ 'clist.c',
              'computer.c',
              'games-frame.c',
              'gyahtzee.c',
              'setup.c',
              'yahtzee.c'] + resources,
             dependencies: [ gtk_dep, gnome_games_dep ],
             c_args: [ '-include', 'config.h',
                       '-DVERSION="@0@"'.format (meson.project_version ()),
                       '-DGETTEXT_PACKAGE="tali"',
                       '-DLOCALEDIR="@0@"'.format (localedir),
                       '-DDATA_DIRECTORY="@0@"'.format (pkgdatadir) ],
             include_directories: config_h_dir,
             install: true )
