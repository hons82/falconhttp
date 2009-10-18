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
#ifndef MEMORYUTILS_H_
#define MEMORYUTILS_H_

#include <stdio.h>
#include <stdlib.h>
/*
 * Allocate and zero memory (With success control)
 */
void *safeCalloc(unsigned int uiNoItems, unsigned int uiSize);
/*
 * Allocate memory (With success control)
 */
void *safeMalloc(unsigned int uiSize);
/*
 * Free the memory (if allocated)
 */
void safeFree(void *vpTemp);

#endif /* MEMORYUTILS_H_ */
