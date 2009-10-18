/* provides functionality to deal with dates
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
#include "dateUtils.h"

// date lenght = 29 incl. string term.
void formatLogTime(char* date) {
	if (date != NULL) {
		time_t t = time(NULL);
		struct tm *loc_t = localtime(&t);
		strftime(date, 29, "[%d/%b/%Y:%H:%M:%S %z]", loc_t);
	}
}

// date lenght = 31 incl. string term.
void formatHeaderTime(char* date) {
	if (date != NULL) {
		time_t t = time(NULL);
		struct tm *loc_t = localtime(&t);
		strftime(date, 31, "%a, %d %b %Y %H:%M:%S %Z", loc_t);
	}
}
