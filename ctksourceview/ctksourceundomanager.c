/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * This file is part of CtkSourceView
 *
 * Copyright (C) 1998, 1999 Alex Roberts, Evan Lawrence
 * Copyright (C) 2000, 2001 Chema Celorio, Paolo Maggi
 * Copyright (C) 2002-2005  Paolo Maggi
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <glib.h>
#include <stdlib.h>
#include <string.h>

#include "ctksourceundomanager.h"

/**
 * SECTION:undomanager
 * @short_description: Undo manager interface for CtkSourceView
 * @title: CtkSourceUndoManager
 * @see_also: #CtkTextBuffer, #CtkSourceView
 *
 * For most uses it isn't needed to use #CtkSourceUndoManager. #CtkSourceBuffer
 * already provides an API and a default implementation for the undo/redo.
 *
 * For specific needs, the #CtkSourceUndoManager interface can be implemented to
 * provide custom undo management. Use ctk_source_buffer_set_undo_manager() to
 * install a custom undo manager for a particular #CtkSourceBuffer.
 *
 * Use ctk_source_undo_manager_can_undo_changed() and
 * ctk_source_undo_manager_can_redo_changed() when respectively the undo state
 * or redo state of the undo stack has changed.
 *
 * Since: 2.10
 */

enum
{
	CAN_UNDO_CHANGED,
	CAN_REDO_CHANGED,
	N_SIGNALS
};

static guint signals[N_SIGNALS];

typedef CtkSourceUndoManagerIface CtkSourceUndoManagerInterface;

G_DEFINE_INTERFACE (CtkSourceUndoManager, ctk_source_undo_manager, G_TYPE_OBJECT)

static gboolean
ctk_source_undo_manager_can_undo_default (CtkSourceUndoManager *manager)
{
	return FALSE;
}

static gboolean
ctk_source_undo_manager_can_redo_default (CtkSourceUndoManager *manager)
{
	return FALSE;
}

static void
ctk_source_undo_manager_undo_default (CtkSourceUndoManager *manager)
{
}

static void
ctk_source_undo_manager_redo_default (CtkSourceUndoManager *manager)
{
}

static void
ctk_source_undo_manager_begin_not_undoable_action_default (CtkSourceUndoManager *manager)
{
}

static void
ctk_source_undo_manager_end_not_undoable_action_default (CtkSourceUndoManager *manager)
{
}

static void
ctk_source_undo_manager_default_init (CtkSourceUndoManagerIface *iface)
{
	iface->can_undo = ctk_source_undo_manager_can_undo_default;
	iface->can_redo = ctk_source_undo_manager_can_redo_default;

	iface->undo = ctk_source_undo_manager_undo_default;
	iface->redo = ctk_source_undo_manager_redo_default;

	iface->begin_not_undoable_action = ctk_source_undo_manager_begin_not_undoable_action_default;
	iface->end_not_undoable_action = ctk_source_undo_manager_end_not_undoable_action_default;

	/**
	 * CtkSourceUndoManager::can-undo-changed:
	 * @manager: The #CtkSourceUndoManager
	 *
	 * Emitted when the ability to undo has changed.
	 *
	 * Since: 2.10
	 *
	 */
	signals[CAN_UNDO_CHANGED] =
		g_signal_new ("can-undo-changed",
			      G_TYPE_FROM_INTERFACE (iface),
			      G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
			      G_STRUCT_OFFSET (CtkSourceUndoManagerIface, can_undo_changed),
			      NULL, NULL,
		              g_cclosure_marshal_VOID__VOID,
			      G_TYPE_NONE,
			      0);
	g_signal_set_va_marshaller (signals[CAN_UNDO_CHANGED],
	                            G_TYPE_FROM_INTERFACE (iface),
	                            g_cclosure_marshal_VOID__VOIDv);

	/**
	 * CtkSourceUndoManager::can-redo-changed:
	 * @manager: The #CtkSourceUndoManager
	 *
	 * Emitted when the ability to redo has changed.
	 *
	 * Since: 2.10
	 *
	 */
	signals[CAN_REDO_CHANGED] =
		g_signal_new ("can-redo-changed",
			      G_TYPE_FROM_INTERFACE (iface),
			      G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
			      G_STRUCT_OFFSET (CtkSourceUndoManagerIface, can_redo_changed),
			      NULL, NULL,
			      g_cclosure_marshal_VOID__VOID,
			      G_TYPE_NONE,
			      0);
	g_signal_set_va_marshaller (signals[CAN_REDO_CHANGED],
	                            G_TYPE_FROM_INTERFACE (iface),
	                            g_cclosure_marshal_VOID__VOIDv);
}

/**
 * ctk_source_undo_manager_can_undo:
 * @manager: a #CtkSourceUndoManager.
 *
 * Get whether there are undo operations available.
 *
 * Returns: %TRUE if there are undo operations available, %FALSE otherwise
 *
 * Since: 2.10
 */
gboolean
ctk_source_undo_manager_can_undo (CtkSourceUndoManager *manager)
{
	g_return_val_if_fail (CTK_SOURCE_IS_UNDO_MANAGER (manager), FALSE);

	return CTK_SOURCE_UNDO_MANAGER_GET_INTERFACE (manager)->can_undo (manager);
}

/**
 * ctk_source_undo_manager_can_redo:
 * @manager: a #CtkSourceUndoManager.
 *
 * Get whether there are redo operations available.
 *
 * Returns: %TRUE if there are redo operations available, %FALSE otherwise
 *
 * Since: 2.10
 */
gboolean
ctk_source_undo_manager_can_redo (CtkSourceUndoManager *manager)
{
	g_return_val_if_fail (CTK_SOURCE_IS_UNDO_MANAGER (manager), FALSE);

	return CTK_SOURCE_UNDO_MANAGER_GET_INTERFACE (manager)->can_redo (manager);
}

/**
 * ctk_source_undo_manager_undo:
 * @manager: a #CtkSourceUndoManager.
 *
 * Perform a single undo. Calling this function when there are no undo operations
 * available is an error. Use ctk_source_undo_manager_can_undo() to find out
 * if there are undo operations available.
 *
 * Since: 2.10
 */
void
ctk_source_undo_manager_undo (CtkSourceUndoManager *manager)
{
	g_return_if_fail (CTK_SOURCE_IS_UNDO_MANAGER (manager));

	CTK_SOURCE_UNDO_MANAGER_GET_INTERFACE (manager)->undo (manager);
}

/**
 * ctk_source_undo_manager_redo:
 * @manager: a #CtkSourceUndoManager.
 *
 * Perform a single redo. Calling this function when there are no redo operations
 * available is an error. Use ctk_source_undo_manager_can_redo() to find out
 * if there are redo operations available.
 *
 * Since: 2.10
 */
void
ctk_source_undo_manager_redo (CtkSourceUndoManager *manager)
{
	g_return_if_fail (CTK_SOURCE_IS_UNDO_MANAGER (manager));

	CTK_SOURCE_UNDO_MANAGER_GET_INTERFACE (manager)->redo (manager);
}

/**
 * ctk_source_undo_manager_begin_not_undoable_action:
 * @manager: a #CtkSourceUndoManager.
 *
 * Begin a not undoable action on the buffer. All changes between this call
 * and the call to ctk_source_undo_manager_end_not_undoable_action() cannot
 * be undone. This function should be re-entrant.
 *
 * Since: 2.10
 */
void
ctk_source_undo_manager_begin_not_undoable_action (CtkSourceUndoManager *manager)
{
	g_return_if_fail (CTK_SOURCE_IS_UNDO_MANAGER (manager));

	CTK_SOURCE_UNDO_MANAGER_GET_INTERFACE (manager)->begin_not_undoable_action (manager);
}

/**
 * ctk_source_undo_manager_end_not_undoable_action:
 * @manager: a #CtkSourceUndoManager.
 *
 * Ends a not undoable action on the buffer.
 *
 * Since: 2.10
 */
void
ctk_source_undo_manager_end_not_undoable_action (CtkSourceUndoManager *manager)
{
	g_return_if_fail (CTK_SOURCE_IS_UNDO_MANAGER (manager));

	CTK_SOURCE_UNDO_MANAGER_GET_INTERFACE (manager)->end_not_undoable_action (manager);
}

/**
 * ctk_source_undo_manager_can_undo_changed:
 * @manager: a #CtkSourceUndoManager.
 *
 * Emits the #CtkSourceUndoManager::can-undo-changed signal.
 *
 * Since: 2.10
 **/
void
ctk_source_undo_manager_can_undo_changed (CtkSourceUndoManager *manager)
{
	g_return_if_fail (CTK_SOURCE_IS_UNDO_MANAGER (manager));

	g_signal_emit (manager, signals[CAN_UNDO_CHANGED], 0);
}

/**
 * ctk_source_undo_manager_can_redo_changed:
 * @manager: a #CtkSourceUndoManager.
 *
 * Emits the #CtkSourceUndoManager::can-redo-changed signal.
 *
 * Since: 2.10
 **/
void
ctk_source_undo_manager_can_redo_changed (CtkSourceUndoManager *manager)
{
	g_return_if_fail (CTK_SOURCE_IS_UNDO_MANAGER (manager));

	g_signal_emit (manager, signals[CAN_REDO_CHANGED], 0);
}
