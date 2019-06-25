#ifndef BOOKMARK_H_INCLUDED
#define BOOKMARK_H_INCLUDED

#include "defs.h"

#define BOOKMARK_FILE ".seashell_bookmarks"

extern bool save_bookmark(const char* path, const char* name);
extern bool list_bookmarks();
extern bool goto_bookmark(const char* name);

#endif // BOOKMARK_H_INCLUDED
