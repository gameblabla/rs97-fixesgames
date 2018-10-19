#ifndef	_port_cfg_h_
#define	_port_cfg_h_

typedef struct port_cfg_settings_struct {
	int	music_enabled;
	int	analog_enabled;
	char	last_levelset_played_filename[1024];
	int	last_level_in_levelset_played;
} port_cfg_settings_struct;

extern char *portcfgfilename;
extern port_cfg_settings_struct port_cfg_settings;

extern int write_port_cfg_file();
extern void set_port_cfg_defaults();
extern int read_port_cfg_file();

#endif

