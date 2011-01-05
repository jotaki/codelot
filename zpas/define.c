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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include "zpas.h"
#include "define.h"

static deftbl_t *head = NULL;

int define_init(void)
{
	head = malloc(sizeof(deftbl_t));
	if(head == NULL)
		return E_OUTOFMEM;

	memset(head, 0, sizeof(deftbl_t));
	return 0;
}

int define_add(char *key, char *val)
{
	deftbl_t *p, *n = malloc(sizeof(deftbl_t));
	if(n == NULL)
		return E_OUTOFMEM;

	n->key = strdup(key);
	n->val = strdup(val);

	for(p = head; p->next != NULL; p = p->next);
	p->next = n;

	return 0;	
}

int define_remove(char *key)
{
	deftbl_t *p, *s;

	for(p = head; p != NULL; p = p->next) {
		if(strcmp(key, p->next->key) == 0) {
			s = p->next->next;
			free(p->next->key);
			free(p->next->val);
			free(p->next);
			p->next = s;

			return 0;
		}
	}
	return E_UNKNOWNDEF;
}

char *define_lookup(char *key)
{
	deftbl_t *p;

	for(p = head->next; p != NULL; p = p->next) {
		if(strcmp(key, p->key) == 0)
			return p->val;
	}
	return NULL;
}

void define_shutdown(void)
{
	deftbl_t *p, *s;

	for(p = head; p->next != NULL;) {
		s = p->next;
		free(p->key);
		free(p->val);
		free(p);
		p = s;
	}
}
