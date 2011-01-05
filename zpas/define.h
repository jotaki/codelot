/*

    zpas - zer0python's simple binary assembler
    Copyright (C) 2005 Joseph Kinsella <zer0python@yahoo.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*/

#ifndef H_DEFINE_
# define H_DEFINE_

typedef struct deftbl_t {
	char *key, *val;
	struct deftbl_t *next;
} deftbl_t;

int define_init(void);
int define_add(char *key, char *val);
int define_remove(char *key);

char *define_lookup(char *key);

void define_shutdown(void);

#endif	/* H_DEFINE_ */
