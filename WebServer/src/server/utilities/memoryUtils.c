/* This methods help with the memory management
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
#include "memoryUtils.h"

void *safeCalloc(unsigned int uiNoItems, unsigned int uiSize) {
	void *vpTemp;

	vpTemp = calloc((unsigned) uiNoItems, (unsigned) uiSize);
	if (vpTemp == (void *) NULL) {
		fprintf(stderr, "ERROR : calloc() failed in safeCalloc()\n");
		exit(1);
	} else
		return (vpTemp);
}

void *safeMalloc(unsigned int uiSize) {
	void * vpTemp;

	vpTemp = malloc((unsigned) uiSize);
	if (vpTemp == (void *) NULL) {
		fprintf(stderr, "ERROR : malloc() failed in safeMalloc()\n");
		exit(1);
	} else
		return (vpTemp);
}

void safeFree(void *vpTemp) {
	// free only if memory was previously allocated
	if (vpTemp != NULL) {
		free(vpTemp);
	}
}
