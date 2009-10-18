/* This methods are intended to simplify the handling with strings
 *
 * Copyright (C) 2009 	Irene Moriggl (irenemoriggl at gmail dot com)
 * 						Hannes Tribus (hons82 at gmail dot com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#include "stringUtils.h"

int strrepl(char *str, char *orig, char *rep) {
	char buffer[4096];
	char *p;

	// Is 'orig' even in 'str'? or
	// are they the same? --> would result in an infinite loop
	if (!(p = strstr(str, orig)) || strcmp(orig, rep) == 0) {
		return 0;
	}
	strncpy(buffer, str, p - str); // Copy characters from 'str' start to 'orig' st$
	buffer[p - str] = '\0';

	sprintf(buffer + (p - str), "%s%s", rep, p + strlen(orig));
	// Use with care!
	strcpy(str, buffer);
	return 1;
}

int strreplall(char *str, char *orig, char *rep) {
	int retval = 1;
	// As long as there are originals in the string...
	while (retval == 1) {
		// ...replace them with rep
		retval = strrepl(str, orig, rep);
	}
	return retval;
}

int strtrimr(char *toTrim) {
	int i, j;
	/* Calculate the length of the string */
	j = i = strlen(toTrim) - 1;
	/* WHILE string ends with a blank */
	while (isspace(toTrim[i]) && (i >= 0)) {
		/*- Replace blank with '\0' */
		toTrim[i--] = '\0';
	}
	/* Return no of replacements */
	return (j - i);
}

int strtriml(char *toTrim) {
	int i = 0, j;
	/* Calculate the length of the string */
	j = strlen(toTrim) - 1;
	/* WHILE string starts with a blank */
	while (isspace(toTrim[i]) && (i <= j)) {
		/*- Count no of leading blanks */
		i++;
	}
	/* IF leading blanks are found */
	if (0 < i) {
		/*- Shift string to the left */
		strcpy(toTrim, &toTrim[i]);
	}
	/* Return no of replacements */
	return (i);
}

int strtrim(char *toTrim) {
	int iBlank;
	/* Remove trailing blanks */
	iBlank = strtrimr(toTrim);
	/* Remove leading blanks */
	iBlank += strtriml(toTrim);

	return (iBlank);
}
