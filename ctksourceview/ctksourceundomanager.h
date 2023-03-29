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

#ifndef CTK_SOURCE_UNDO_MANAGER_H
#define CTK_SOURCE_UNDO_MANAGER_H

#if !defined (CTK_SOURCE_H_INSIDE) && !defined (CTK_SOURCE_COMPILATION)
#error "Only <ctksourceview/ctksource.h> can be included directly."
#endif

#include <ctk/ctk.h>
#include <ctksourceview/ctksourcetypes.h>

G_BEGIN_DECLS

#define CTK_SOURCE_TYPE_UNDO_MANAGER                (ctk_source_undo_manager_get_type ())
#define CTK_SOURCE_UNDO_MANAGER(obj)                (G_TYPE_CHECK_INSTANCE_CAST ((obj), CTK_SOURCE_TYPE_UNDO_MANAGER, CtkSourceUndoManager))
#define CTK_SOURCE_IS_UNDO_MANAGER(obj)             (G_TYPE_CHECK_INSTANCE_TYPE ((obj), CTK_SOURCE_TYPE_UNDO_MANAGER))
#define CTK_SOURCE_UNDO_MANAGER_GET_INTERFACE(obj)  (G_TYPE_INSTANCE_GET_INTERFACE ((obj), CTK_SOURCE_TYPE_UNDO_MANAGER, CtkSourceUndoManagerIface))

typedef struct _CtkSourceUndoManagerIface      	CtkSourceUndoManagerIface;

struct _CtkSourceUndoManagerIface
{
	GTypeInterface parent;

	/* Interface functions */
	gboolean (*can_undo)                  (CtkSourceUndoManager *manager);
	gboolean (*can_redo)                  (CtkSourceUndoManager *manager);

	void     (*undo)                      (CtkSourceUndoManager *manager);
	void     (*redo)                      (CtkSourceUndoManager *manager);

	void     (*begin_not_undoable_action) (CtkSourceUndoManager *manager);
	void     (*end_not_undoable_action)   (CtkSourceUndoManager *manager);

	/* Signals */
	void     (*can_undo_changed)          (CtkSourceUndoManager *manager);
	void     (*can_redo_changed)          (CtkSourceUndoManager *manager);
};

CTK_SOURCE_AVAILABLE_IN_ALL
GType     ctk_source_undo_manager_get_type                  (void) G_GNUC_CONST;

CTK_SOURCE_AVAILABLE_IN_ALL
gboolean  ctk_source_undo_manager_can_undo                  (CtkSourceUndoManager *manager);

CTK_SOURCE_AVAILABLE_IN_ALL
gboolean  ctk_source_undo_manager_can_redo                  (CtkSourceUndoManager *manager);

CTK_SOURCE_AVAILABLE_IN_ALL
void      ctk_source_undo_manager_undo                      (CtkSourceUndoManager *manager);

CTK_SOURCE_AVAILABLE_IN_ALL
void      ctk_source_undo_manager_redo                      (CtkSourceUndoManager *manager);

CTK_SOURCE_AVAILABLE_IN_ALL
void      ctk_source_undo_manager_begin_not_undoable_action (CtkSourceUndoManager *manager);

CTK_SOURCE_AVAILABLE_IN_ALL
void      ctk_source_undo_manager_end_not_undoable_action   (CtkSourceUndoManager *manager);

CTK_SOURCE_AVAILABLE_IN_ALL
void      ctk_source_undo_manager_can_undo_changed          (CtkSourceUndoManager *manager);

CTK_SOURCE_AVAILABLE_IN_ALL
void      ctk_source_undo_manager_can_redo_changed          (CtkSourceUndoManager *manager);

G_END_DECLS

#endif /* CTK_SOURCE_UNDO_MANAGER_H */
