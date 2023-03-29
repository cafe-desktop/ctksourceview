/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * This file is part of CtkSourceView
 *
 * Copyright (C) 1998, 1999 Alex Roberts, Evan Lawrence
 * Copyright (C) 2000, 2001 Chema Celorio, Paolo Maggi
 * Copyright (C) 2002, 2003 Paolo Maggi
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, see <http://www.gnu.org/licenses/>.
 */

#ifndef CTK_SOURCE_UNDO_MANAGER_DEFAULT_H
#define CTK_SOURCE_UNDO_MANAGER_DEFAULT_H

#include <glib-object.h>
#include "ctksourcetypes-private.h"

G_BEGIN_DECLS

#define CTK_SOURCE_TYPE_UNDO_MANAGER_DEFAULT		(ctk_source_undo_manager_default_get_type ())
#define CTK_SOURCE_UNDO_MANAGER_DEFAULT(obj)		(G_TYPE_CHECK_INSTANCE_CAST((obj), CTK_SOURCE_TYPE_UNDO_MANAGER_DEFAULT, CtkSourceUndoManagerDefault))
#define CTK_SOURCE_UNDO_MANAGER_DEFAULT_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST((klass), CTK_SOURCE_TYPE_UNDO_MANAGER_DEFAULT, CtkSourceUndoManagerDefaultClass))
#define CTK_SOURCE_IS_UNDO_MANAGER_DEFAULT(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), CTK_SOURCE_TYPE_UNDO_MANAGER_DEFAULT))
#define CTK_SOURCE_IS_UNDO_MANAGER_DEFAULT_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), CTK_SOURCE_TYPE_UNDO_MANAGER_DEFAULT))
#define CTK_SOURCE_UNDO_MANAGER_DEFAULT_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), CTK_SOURCE_TYPE_UNDO_MANAGER_DEFAULT, CtkSourceUndoManagerDefaultClass))

typedef struct _CtkSourceUndoManagerDefaultClass 	CtkSourceUndoManagerDefaultClass;
typedef struct _CtkSourceUndoManagerDefaultPrivate 	CtkSourceUndoManagerDefaultPrivate;

struct _CtkSourceUndoManagerDefault
{
	GObject parent;

	CtkSourceUndoManagerDefaultPrivate *priv;
};

struct _CtkSourceUndoManagerDefaultClass
{
	GObjectClass parent_class;
};

G_GNUC_INTERNAL
GType ctk_source_undo_manager_default_get_type (void) G_GNUC_CONST;

G_GNUC_INTERNAL
void ctk_source_undo_manager_default_set_max_undo_levels (CtkSourceUndoManagerDefault *manager,
                                                          gint                         max_undo_levels);

G_END_DECLS

#endif /* CTK_SOURCE_UNDO_MANAGER_DEFAULT_H */
