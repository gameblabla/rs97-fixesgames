/*	Functions added by Dan Silsby (senquack aka DKS) for handheld port -
	Load and save certain settings we wish to remain
	persistent between launches.
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
//#include <stropts.h>
#include <errno.h>

#include "port_cfg.h"

char *portcfgfilename;	// ideally this will be $HOME/.tworld/port_cfg (set in tworld.c, initdirs(...))

port_cfg_settings_struct port_cfg_settings;

static int clamp(int x, int min, int max)
{
	if (x < min) { return min; }
	else if (x > max) { return max; }
	else return x;

}

static char *trim_string(char *buf)
{
	int len;

	while (*buf == ' ') buf++;

	len = strlen(buf);

	while (len != 0 && buf[len - 1] == ' ')
	{
		len--;
		buf[len] = 0;
	}

	return buf;
}

//  Return 1 on success, 0 on error.  Write settings from structure.
int write_port_cfg_file()
{
	if (!portcfgfilename)
	{
		printf("Error: no port config filename set (oshw-sdl/cfg.c)\n");
		printf("\t...skipped writing new port_cfg settings file.\n");
		return 0;
	}

	FILE *f;
	f = fopen(portcfgfilename, "w");
	if (!f)
	{
		printf("Error opening file for writing: %s\n", portcfgfilename);
		return 0;
	}
	else
	{
		printf("Writing settings to file: %s\n", portcfgfilename);
	}
	fprintf(f, "music_enabled=%d\n", port_cfg_settings.music_enabled);
	fprintf(f, "analog_enabled=%d\n", port_cfg_settings.analog_enabled);
	fprintf(f, "last_levelset_played_filename=%s\n", port_cfg_settings.last_levelset_played_filename);
	fprintf(f, "last_level_in_levelset_played=%d\n", port_cfg_settings.last_level_in_levelset_played);
	int returntmp = (fclose(f) == 0);
	sync();
	//	return (fclose(f) == 0);
	return (returntmp);
}

void set_port_cfg_defaults()
{
	port_cfg_settings_struct *p = &port_cfg_settings;
	p->music_enabled = 1;
	p->analog_enabled = 0;
	p->last_levelset_played_filename[0] = '\0';
	p->last_level_in_levelset_played = 0;
}


//  Return 1 on success, 0 on error.  Read settings into new_settings structure
int read_port_cfg_file()
{
	set_port_cfg_defaults();

	if (!portcfgfilename)
	{
		printf("Error: no port config filename set (oshw-sdl/cfg.c)\n");
		printf("\t...skipped reading port_cfg settings file.\n");
		return 0;
	}

	FILE *f;
	char buf[8192];
	char *str, *param;

	f = fopen(portcfgfilename, "r");
	if (f == NULL)
	{
		printf("Error opening file: %s\n", portcfgfilename);
		return 0;
	}
	else
	{
		printf("Loading settings from file: %s\n", portcfgfilename);
	}

	while (!feof(f))
	{
		// skip empty lines
		fscanf(f, "%8192[\n\r]", buf);

		// read line
		buf[0] = 0;
		fscanf(f, "%8192[^\n^\r]", buf);

		// trim line
		str = trim_string(buf);

		if (str[0] == 0) continue;
		if (str[0] == '#') continue;

		// find parameter (after '=')
		param = strchr(str, '=');

		if (param == NULL) continue;

		// split string into two strings
		*param = 0;
		param++;

		// trim them
		str = trim_string(str);
		param = trim_string(param);

		if ( strcasecmp(str, "music_enabled") == 0 )
			port_cfg_settings.music_enabled = clamp(atoi(param), 0, 1);
		else if ( strcasecmp(str, "analog_enabled") == 0 )
			port_cfg_settings.analog_enabled = clamp(atoi(param), 0, 1);
		else if ( strcasecmp(str, "last_levelset_played_filename") == 0 )
		{
			if ((strlen(param) > 0)	&& (strlen(param) < 1024))
			{
				strcpy(port_cfg_settings.last_levelset_played_filename, param);
			} 
		} else if ( strcasecmp(str, "last_level_in_levelset_played") == 0 )
			port_cfg_settings.last_level_in_levelset_played = clamp(atoi(param), 1, 999);
		else
		{
			printf("Ignoring unknown setting: %s\n", str);
		}
	}

	fclose(f);
	return 1;
}

