/*
 * This file is part of GtkSourceView
 *
 * Copyright (C) 2017 - Sébastien Wilmet <swilmet@gnome.org>
 *
 * GtkSourceView is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * GtkSourceView is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, see <http://www.gnu.org/licenses/>.
 */

#ifndef CTK_SOURCE_INIT_H
#define CTK_SOURCE_INIT_H

#if !defined (CTK_SOURCE_H_INSIDE) && !defined (CTK_SOURCE_COMPILATION)
#error "Only <ctksourceview/ctksource.h> can be included directly."
#endif

#include <glib.h>
#include <ctksourceview/ctksourceversion.h>

G_BEGIN_DECLS

CTK_SOURCE_AVAILABLE_IN_4_0
void	ctk_source_init		(void);

CTK_SOURCE_AVAILABLE_IN_4_0
void	ctk_source_finalize	(void);

G_END_DECLS

#endif /* CTK_SOURCE_INIT_H */
