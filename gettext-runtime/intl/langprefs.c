/* Determine the user's language preferences.
   Copyright (C) 2004 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify it
   under the terms of the GNU Library General Public License as published
   by the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307,
   USA.  */

/* Written by Bruno Haible <bruno@clisp.org>.  */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdlib.h>

#if HAVE_CFPREFERENCESCOPYAPPVALUE
# include <string.h>
# include <CFPreferences.h>
# include <CFPropertyList.h>
# include <CFArray.h>
# include <CFString.h>
#endif

/* Determine the user's language preferences, as a colon separated list of
   locale names in XPG syntax
     language[_territory[.codeset]][@modifier]
   The result must not be freed; it is statically allocated.  */

const char *
_nl_language_preferences (void)
{
  const char *language;

  language = getenv ("LANGUAGE");
  if (language != NULL && language[0] != '\0')
    return language;

#if HAVE_CFPREFERENCESCOPYAPPVALUE /* MacOS X 10.2 or newer */
  {
    /* Cache the preferences list, since CoreFoundation calls are expensive.  */
    static const char *cached_languages;
    static int cache_initialized;

    if (!cache_initialized)
      {
	CFPropertyListRef preferences =
	  CFPreferencesCopyAppValue (CFSTR ("AppleLanguages"),
				     kCFPreferencesCurrentApplication);
	if (CFGetTypeID (preferences) == CFArrayGetTypeID ())
	  {
	    CFArrayRef prefArray = (CFArrayRef)preferences;
	    int n = CFArrayGetCount (prefArray);
	    char buf[256];
	    size_t size = 0;
	    int i;

	    for (i = 0; i < n; i++)
	      {
		CFTypeRef element = CFArrayGetValueAtIndex (prefArray, i);
		if (CFGetTypeID (element) == CFStringGetTypeID ()
		    && CFStringGetCString ((CFStringRef)element,
					   buf, sizeof (buf),
					   kCFStringEncodingASCII))
		  {
		    size += strlen (buf) + 1;
		    /* Most GNU programs use msgids in English and don't ship
		       an en.mo message catalog.  Therefore when we see "en"
		       in the preferences list, arrange for gettext() to
		       return the msgid, and ignore all further elements of
		       the preferences list.  */
		    if (strcmp (buf, "en") == 0)
		      break;
		  }
		else
		  break;
	      }
	    if (size > 0)
	      {
		char *languages = (char *) malloc (size);

		if (languages != NULL)
		  {
		    char *p = languages;

		    for (i = 0; i < n; i++)
		      {
			CFTypeRef element =
			  CFArrayGetValueAtIndex (prefArray, i);
			if (CFGetTypeID (element) == CFStringGetTypeID ()
			    && CFStringGetCString ((CFStringRef)element,
						   buf, sizeof (buf),
						   kCFStringEncodingASCII))
			  {
			    strcpy (p, buf);
			    p += strlen (buf);
			    *p++ = ':';
			    if (strcmp (buf, "en") == 0)
			      break;
			  }
			else
			  break;
		      }
		    *--p = '\0';

		    cached_languages = languages;
		  }
	      }
	  }
	cache_initialized = 1;
      }
    if (cached_languages != NULL)
      return cached_languages;
  }
#endif

  return NULL;
}