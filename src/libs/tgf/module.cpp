/***************************************************************************
                      module.cpp -- Dynamic module management                                
                             -------------------                                         
    created              : Fri Aug 13 22:25:53 CEST 1999
    copyright            : (C) 1999-2014 by Eric Espie, Bernhard Wymann
    email                : torcs@free.fr   
    version              : $Id: module.cpp,v 1.5.2.1 2014/05/20 14:07:09 berniw Exp $                                  
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
    Dynamic module API.
        
    This is the interface to load/unload the shared libraries (or DLLs).
    Two modes are allowed, the access by filename, of the access by the entire directory.
    When the directory mode is used, the filenames are not known in advance, this allows
    more flexibility at runtime.

    The generic information can be retrieved, without keeping the DLL loaded.
    The gfid parameter is used to differentiate the modules using different includes. This
    functionality is not used yet. This API is not used for shared libraries linked
    staticaly at compilation time.
    @author Bernhard Wymann, Eric Espie
    @version $Id: module.cpp,v 1.5.2.1 2014/05/20 14:07:09 berniw Exp $
*/

#ifdef WIN32
#include <windows.h>
#endif
#include <tgf.h>
#include "os.h"

void
gfModInit(void)
{
}

/** Load the specified DLLs.
    @ingroup	module
    @param	gfid	Mask for version checking
    @param	dllname	File name of the DLL
    @param	modlist	List of module description structure
    @return	>=0 Number of modules loaded
		<br>-1 Error
    @warning	The modlist contains only one element
    @see	tModList
 */
int
GfModLoad(unsigned int gfid, char *dllname, tModList **modlist)
{
    if (GfOs.modLoad) {
	return GfOs.modLoad(gfid, dllname, modlist);
    } else {
	return -1;
    }
}

/** Load the DLLs in the specified directory.
    @ingroup	module
    @param	gfid	Mask for version checking
    @param	dir	Directory name where to find the DLLs
    @param	modlist	List of module description structure
    @return	>=0 Number of modules loaded
		<br>-1 Error
 */
int
GfModLoadDir(unsigned int gfid, char *dir, tModList **modlist)
{
    if (GfOs.modLoadDir) {
	return GfOs.modLoadDir(gfid, dir, modlist);
    } else {
	return -1;
    }
}

/** Unload the DLLs of a list.
    @ingroup	module
    @param	modlist	List of DLLs to unload
    @return	0 Ok
		<br>-1 Error
 */
int
GfModUnloadList(tModList **modlist)
{
    if (GfOs.modUnloadList) {
	return GfOs.modUnloadList(modlist);
    } else {
	return -1;
    }
}

/** Get the generic information of the specified DLL.
    @ingroup	module
    @param	gfid	Mask for version control
    @param	dllname	File name of the DLL
    @param	modlist	The information are stored here.
    @return	>=0	Number of modules infoed
		<br>-1 Error
    @warning	The modlist contains only one element
 */
int
GfModInfo(unsigned int gfid, char *dllname, tModList **modlist)
{
    if (GfOs.modInfo) {
	return GfOs.modInfo(gfid, dllname, modlist);
    } else {
	return -1;
    }
}

/** Get the generic module information of the DLLs of the specified directory.
    @ingroup	module
    @param	gfid	Mask for version checking
    @param	dir	Directory name where to find the DLLs
    @param	level	Indicates the sub dir level (0 or 1) 
    @param	modlist	List of module description structure
    @return	>=0	Number of modules infoed
		<br>-1 Error
 */
int
GfModInfoDir(unsigned int gfid, char *dir, int level, tModList **modlist)
{
    if (GfOs.modInfoDir) {
	return GfOs.modInfoDir(gfid, dir, level, modlist);
    } else {
	return -1;
    }
}

/** Free the info contained in the specified list.
    @ingroup	module
    @param	modlist	List of info to free
    @return	0 Ok
		<br>-1 Error
 */
int
GfModFreeInfoList(tModList **modlist)
{
    if (GfOs.modFreeInfoList) {
	return GfOs.modFreeInfoList(modlist);
    } else {
	return -1;
    }
}
