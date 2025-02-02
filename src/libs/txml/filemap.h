/*
The contents of this file are subject to the Mozilla Public License
Version 1.0 (the "License"); you may not use this file except in
compliance with the License. You may obtain a copy of the License at
http://www.mozilla.org/MPL/

Software distributed under the License is distributed on an "AS IS"
basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
License for the specific language governing rights and limitations
under the License.

The Original Code is expat.

The Initial Developer of the Original Code is James Clark.
Portions created by James Clark are Copyright (C) 1998
James Clark. All Rights Reserved.

Contributor(s):
$Id: filemap.h,v 1.1.1.1 2001/06/24 18:29:45 torcs Exp $
*/


#include <stddef.h>

#ifdef XML_UNICODE
int filemap(const wchar_t *name,
	    void (*processor)(const void *, size_t, const wchar_t *, void *arg),
	    void *arg);
#else
int filemap(const char *name,
	    void (*processor)(const void *, size_t, const char *, void *arg),
	    void *arg);
#endif
