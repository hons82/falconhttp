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
#ifndef STRINGUTILS_H_
#define STRINGUTILS_H_

#include <string.h>
#include <stdio.h>
#include <ctype.h>
/*
 * Replace in the string "str" the first "orig" with "repl"
 */
int strrepl(char *str, char *orig, char *rep);
/*
 * Replace in the string "str" ALL "orig" with "repl"
 */
int strreplall(char *str, char *orig, char *rep);
/*
 * Remove all trailing blanks
 */
int strtrimr(char *toTrim);
/*
 * Remove all leading blanks
 */
int strtriml(char *toTrim);
/*
 * Remove all leading & trailing blanks
 */
int strtrim(char *toTrim);

#endif /* STRINGUTILS_H_ */
