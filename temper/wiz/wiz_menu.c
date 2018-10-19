#define CONFIG_EXIT_BUTTON          "x"
#define CONTROL_CONFIG_START_COLUMN 40
#define CONTROL_CONFIG_STRING       "Wiz                  PC-Engine"

menu_struct options_menu =
{
  17, 0, menu_options_done,
  {
    menu_option_string("Show fps                %s", 7, NULL,
     config.show_fps, 2, yes_no_strings),
    menu_option_string("Enable sound            %s", 8, NULL,
     config.enable_sound, 2, yes_no_strings),
    menu_option_string("Fast forward            %s", 9, NULL,
     config.fast_forward, 2, yes_no_strings),
    menu_option_numeric("Clock speed             %03d", 10, NULL,
     config.clock_speed, 900),
    menu_option_string("Patch idle loops        %s", 11, NULL,
     config.patch_idle_loops, 2, yes_no_strings),
    menu_option_string("Snapshot in saves       %s", 12, NULL,
     config.snapshot_format, 2, yes_no_strings),
    menu_option_string("BZ2 compressed saves    %s", 13, NULL,
     config.bz2_savestates, 2, yes_no_strings),
    menu_option_string("Use six button pad      %s", 14, NULL,
     config.six_button_pad, 2, yes_no_strings),
    menu_option_string("CD-ROM System          %s", 15, NULL,
     config.cd_system_type, 5, cd_system_version_strings),
    menu_option_string("Per-game BRAM saves     %s", 16, NULL,
     config.per_game_bram, 2, yes_no_strings),
    menu_option_string("Double volume           %s", 17, NULL,
     config.sound_double, 2, yes_no_strings),
    menu_option_string("Scale screen width      %s", 18, NULL,
     config.scale_width, 2, yes_no_strings),
    menu_option_string("Don't limit sprites     %s", 19, NULL,
     config.unlimit_sprites, 2, yes_no_strings),
    menu_option_string("Use compatibility mode  %s", 20, NULL,
     config.compatibility_mode, 2, yes_no_strings),
    menu_option_fixed("Exit: save for all games", 22,
     menu_options_save_config_global),
    menu_option_fixed("Exit: save for this game", 23,
     menu_options_save_config_local),
    menu_option_fixed("Exit without saving", 24, menu_options_done)
  }
};

menu_struct pad_menu =
{
  18, 0, menu_pad_done,
  {
    menu_option_string("Up                %s", 6, NULL, config.pad[0],
     button_count, button_strings),
    menu_option_string("Down              %s", 7, NULL, config.pad[1],
     button_count, button_strings),
    menu_option_string("Left              %s", 8, NULL, config.pad[2],
     button_count, button_strings),
    menu_option_string("Right             %s", 9, NULL, config.pad[3],
     button_count, button_strings),
    menu_option_string("A                 %s", 10, NULL, config.pad[4],
     button_count, button_strings),
    menu_option_string("B                 %s", 11, NULL, config.pad[5],
     button_count, button_strings),
    menu_option_string("X                 %s", 12, NULL, config.pad[6],
     button_count, button_strings),
    menu_option_string("Y                 %s", 13, NULL, config.pad[7],
     button_count, button_strings),
    menu_option_string("L Trigger         %s", 14, NULL, config.pad[8],
     button_count, button_strings),
    menu_option_string("R Trigger         %s", 15, NULL, config.pad[9],
     button_count, button_strings),
    menu_option_string("Menu              %s", 16, NULL, config.pad[10],
     button_count, button_strings),
    menu_option_string("Select            %s", 17, NULL, config.pad[11],
     button_count, button_strings),
    menu_option_string("Volume Down       %s", 18, NULL, config.pad[12],
     button_count, button_strings),
    menu_option_string("Volume Up         %s", 19, NULL, config.pad[13],
     button_count, button_strings),
    menu_option_string("Volume Up + Down  %s", 20, NULL, config.pad[14],
     button_count, button_strings),

    menu_option_fixed("Exit: save for all games", 22,
     menu_pad_save_config_global),
    menu_option_fixed("Exit: save for this game", 23,
     menu_pad_save_config_local),
    menu_option_fixed("Exit without saving", 24, menu_pad_done)
  }
};

