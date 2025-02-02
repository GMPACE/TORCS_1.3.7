/***************************************************************************

    file                 : loadingscreen.cpp
    created              : Sun Feb 25 00:34:46 /etc/localtime 2001
    copyright            : (C) 2000-2014 by Eric Espie, Bernhard Wymann
    email                : eric.espie@torcs.org
    version              : $Id: loadingscreen.cpp,v 1.2.2.5 2014/05/20 12:20:05 berniw Exp $

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

/** @file
    Loading screen.
    @author	Eric Espie, Bernhard Wymann
    @version	$Id: loadingscreen.cpp,v 1.2.2.5 2014/05/20 12:20:05 berniw Exp $
*/

#include <stdlib.h>
#ifdef WIN32
#include <windows.h>
#endif
#include <tgfclient.h>
#include <car.h>

static void *menuHandle = NULL;
#define TEXTLINES 23
static int rmTextId[TEXTLINES];
static char *rmTextLines[TEXTLINES] = {0};
static int rmCurText;

float black[4] = { 0.0, 0.0, 0.0, 0.0 };
float white[TEXTLINES][4];



static void rmDeativate(void * /* dummy */)
{
}


/** @brief Set up loading screen
 *  @ingroup racemantools
 *  @param title Screen title.
 *  @param bgimg Optionnal backgrounf image (NULL for no img).
*/
void RmLoadingScreenStart(const char *title, const char *bgimg)
{
	int i;
	int y;
	
	if (GfuiScreenIsActive(menuHandle)) {
		/* Already active */
		return;
	}
	
	if (menuHandle) {
		GfuiScreenRelease(menuHandle);
	}
	menuHandle = GfuiScreenCreateEx(black, NULL, NULL, NULL, rmDeativate, 0);
	
	GfuiTitleCreate(menuHandle, title, strlen(title));
	
	/* create TEXTLINES lines of text */
	for (i = 0, y = 400; i < TEXTLINES; i++, y -= 16) {
		white[i][0] = white[i][1] = white[i][2] = 1.0;
		white[i][3] = (float)i * 0.0421 + 0.2;
		rmTextId[i] = GfuiLabelCreateEx(menuHandle, "", white[i], GFUI_FONT_MEDIUM_C, 60, y, 
						GFUI_ALIGN_HL_VB, 100);
		if (rmTextLines[i]) {
			/* free old text */
			free(rmTextLines[i]);
			rmTextLines[i] = NULL;
		}
	}
	
	rmCurText = 0;
	
	if (bgimg) {
		GfuiScreenAddBgImg(menuHandle, bgimg);
	}
	
	GfuiScreenActivate(menuHandle);
	GfuiDisplay();
}


/** @brief Shut down loading screen
 *  @ingroup racemantools
 */
void RmShutdownLoadingScreen(void)
{
	if (menuHandle) {
		GfuiScreenRelease(menuHandle);
		menuHandle = 0;
		// TODO: release rmTextLines here instead of in RmLoadingScreenStart, or both?
	}
}


/** @brief Set a new line of text on the loading screen
 *  @ingroup racemantools
 *  @param[in] text Text to display
 */
void RmLoadingScreenSetText(const char *text)
{
	int i, j;
	
	GfOut("%s\n", text);
	
	if (menuHandle) {
		if (text) {
			if (rmTextLines[rmCurText]) {
				free(rmTextLines[rmCurText]);
			}
			rmTextLines[rmCurText] = strdup(text);
			rmCurText = (rmCurText + 1) % TEXTLINES;
		}
		
		i = rmCurText;
		j = 0;
		do {
			if (rmTextLines[i]) {
				GfuiLabelSetText(menuHandle, rmTextId[j], rmTextLines[i]);
			}
			j++;
			i = (i + 1) % TEXTLINES;
		} while (i != rmCurText);
		
		GfuiDisplay();
	}
}