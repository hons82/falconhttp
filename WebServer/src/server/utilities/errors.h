/* String representation of the errorpages
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
#ifndef ERRORS_H_
#define ERRORS_H_

/* missing = requested page tot. length = 359 incl. string termination */
#define E_404_1 "\n\n<html><head><meta http-equiv=content-type content=\"text/html; charset=UTF-8\"><title>404 - Page Not Found</title></head><body bgcolor=#ccffff topmargin=80 marginheight=40><h2>Page Not Found</h2><hr><p>The page - <b>"
#define E_404_2 "</b> - does not exist.</p><p>Suggestions:</p><p style=\"margin-left : 1cm;\"> Check the spelling of the address you typed.</p></body></html>\n"

/* missing = requested page tot. length = 273 incl. string termination */
#define E_403_1 "\n\n<html><head><meta http-equiv=content-type content=\"text/html; charset=UTF-8\"><title>403 Forbidden</title></head><body bgcolor=#ccffff topmargin=80 marginheight=40><h2>Forbidden</h2><hr><p>You don't have permission to access - <b>"
#define E_403_2 "</b> - on this server.</p></body></html>\n"

/* missing = requested page, tot. length = 382 incl. string termination */
#define E_400_1 "\n\n<html><head><meta http-equiv=content-type content=\"text/html; charset=UTF-8\"><title>400 Bad Request</title></head><body bgcolor=#ccffff topmargin=80 marginheight=40><h2>Bad Request</h2><hr><p>You sent a request that this server could not understand: <b>"
#define E_400_2 "</b>.</p><p>Suggestions:</p><p style=\"margin-left : 1cm;\"> Check the spelling of the address you typed.</p></body></html>\n"

/* missing = requested page, tot. length = 363 incl. string termination */
#define E_500_1 "\n\n<html><head><meta http-equiv=content-type content=\"text/html; charset=UTF-8\"><title>500 Internal Server Error</title></head><body bgcolor=#ccffff topmargin=80 marginheight=40><h2>Internal Server Error</h2><hr><p>There has been an internal server error with the page you've requested: <b>"
#define E_500_2 "</b>. The server was unable to complete your request.</p></body></html>\n"

/* missing = requested operation, tot. length = 304 incl. string termination */
#define E_501_1 "\n\n<html><head><meta http-equiv=content-type content=\"text/html; charset=UTF-8\"><title>501 Method Not Implemented</title></head><body bgcolor=#ccffff topmargin=80 marginheight=40><h2>Method Not Implemented</h2><hr><p>The requested operation - <b>"
#define E_501_2 "</b> - is not supported by this server.</p></body></html>\n"

/* tot. length = 384 incl. string termination */
#define E_503 "\n\n<html><head><meta http-equiv=content-type content=\"text/html; charset=UTF-8\"><title>503 Service Temporarily Unavailable</title></head><body bgcolor=#ccffff topmargin=80 marginheight=40><h2>Service Temporarily Unavailable</h2><hr><p>The server is temporarily unable to service your request due to maintenance downtime or capacity problems. Please try again later.</p></body></html>\n"

#endif /* ERRORS_H_ */
