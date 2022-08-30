/*
 * Media controller test application
 *
 * Copyright (C) 2010-2014 Ideas on board SPRL
 *
 * Contact: Laurent Pinchart <laurent.pinchart@ideasonboard.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __OPTIONS_H
#define __OPTIONS_H

struct media_options
{
	const char *devname;
	unsigned int interactive:1,
		     print:1,
		     print_dot:1,
		     reset:1,
		     verbose:1;
	const char *entity;
	const char *formats;
	const char *links;
	const char *fmt_pad;
	const char *get_dv_pad;
	const char *dv_pad;
};

extern struct media_options media_opts;

extern int parse_cmdline(int argc, char **argv);

#endif /* __OPTIONS_H */

