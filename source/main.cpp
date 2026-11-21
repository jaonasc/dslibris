/* 

dslibris - an ebook reader for the Nintendo DS.

 Copyright (C) 2007-2020 Ray Haleblian (ray@haleblian.com)

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA

*/

#include <sys/param.h>
#include "dirent.h"
#include "fat.h"

#include "app.h"
#include "book.h"
#include "button.h"
#include "text.h"
#include "expat.h"
#include "main.h"
#include "parse.h"
#include "types.h"
#include "reader_visual.h"

App *app;
char msg[256];
int ft_main(int argc, char **argv);

/*---------------------------------------------------------------------------*/


int kungheyfatcheck(void) {
	iprintf("** kungheyfatcheck **\n");
	iprintf("root directory:\n");
	swiWaitForVBlank();

	DIR *dp = opendir("/");
	if (!dp) {
		printf("[PANIC] root dir inaccessible!\n");
		return false;
	}
	struct dirent *ent;
	while ((ent = readdir(dp)))
	{
		iprintf("%s %d\n", ent->d_name, ent->d_type);
	}
	closedir(dp);

	return true;
}

PrintConsole* boot_console(void) {
	// Get a console going.
	auto console = consoleDemoInit();
	if (!console) iprintf("[FAIL] console!\n");		// This, of course, won't print :D
	else iprintf("[OK] console\n");
	return console;
}

int boot_filesystem(void) {
	// Start up the filesystem.
	bool success = fatInitDefault();
	if (!success) iprintf("[FAIL] filesystem!\n");
	else iprintf("[OK] filesystem\n");
	return success;
}

void spin(void) {
	while(true) swiWaitForVBlank();
}

void fatal(const char *msg) {
	iprintf(msg);
	spin();
}

int main(void)
{
	if(!boot_console()) spin();
	if(!boot_filesystem()) spin();

	//kungheyfatcheck();
	// asciiart();
	// while(true) swiWaitForVBlank();

	app = new App();
	return app->Run();
}

int getSize(uint8 *source, uint16 *dest, uint32 arg) {
       return *(uint32*)source;
}

uint8 readByte(uint8 *source) { return *source; }

void drawstack(u16 *screen) {
       TDecompressionStream decomp = {getSize, NULL, readByte};
       swiDecompressLZSSVram((void*)splashBitmap, screen, 0, &decomp);
}
