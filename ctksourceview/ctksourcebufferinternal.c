/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8; coding: utf-8 -*- */
/*
 * This file is part of CtkSourceView
 *
 * Copyright (C) 2016 - Sébastien Wilmet <swilmet@gnome.org>
 *
 * CtkSourceView is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * CtkSourceView is distributed in the hope that it will be useful,
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

#include "ctksourcebufferinternal.h"
#include "ctksourcebuffer.h"
#include "ctksource-marshal.h"
#include "ctksourcesearchcontext.h"

/* A private extension of CtkSourceBuffer, to add private signals and
 * properties.
 */

struct _CtkSourceBufferInternal
{
	GObject parent_instance;
};

enum
{
	SIGNAL_SEARCH_START,
	N_SIGNALS
};

#define CTK_SOURCE_BUFFER_INTERNAL_KEY "ctk-source-buffer-internal-key"

static guint signals[N_SIGNALS];

G_DEFINE_TYPE (CtkSourceBufferInternal, _ctk_source_buffer_internal, G_TYPE_OBJECT)

static void
_ctk_source_buffer_internal_class_init (CtkSourceBufferInternalClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	/*
	 * CtkSourceBufferInternal::search-start:
	 * @buffer_internal: the object that received the signal.
	 * @search_context: the #CtkSourceSearchContext.
	 *
	 * The ::search-start signal is emitted when a search is starting.
	 */
	signals[SIGNAL_SEARCH_START] =
		g_signal_new ("search-start",
			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_LAST,
			      0,
			      NULL, NULL,
		              g_cclosure_marshal_VOID__OBJECT,
			      G_TYPE_NONE,
			      1, CTK_SOURCE_TYPE_SEARCH_CONTEXT);
	g_signal_set_va_marshaller (signals[SIGNAL_SEARCH_START],
	                            G_TYPE_FROM_CLASS (klass),
	                            g_cclosure_marshal_VOID__OBJECTv);
}

static void
_ctk_source_buffer_internal_init (CtkSourceBufferInternal *buffer_internal)
{
}

/*
 * _ctk_source_buffer_internal_get_from_buffer:
 * @buffer: a #CtkSourceBuffer.
 *
 * Returns the #CtkSourceBufferInternal object of @buffer. The returned object
 * is guaranteed to be the same for the lifetime of @buffer.
 *
 * Returns: (transfer none): the #CtkSourceBufferInternal object of @buffer.
 */
CtkSourceBufferInternal *
_ctk_source_buffer_internal_get_from_buffer (CtkSourceBuffer *buffer)
{
	CtkSourceBufferInternal *buffer_internal;

	g_return_val_if_fail (CTK_SOURCE_IS_BUFFER (buffer), NULL);

	buffer_internal = g_object_get_data (G_OBJECT (buffer), CTK_SOURCE_BUFFER_INTERNAL_KEY);

	if (buffer_internal == NULL)
	{
		buffer_internal = g_object_new (CTK_SOURCE_TYPE_BUFFER_INTERNAL, NULL);

		g_object_set_data_full (G_OBJECT (buffer),
					CTK_SOURCE_BUFFER_INTERNAL_KEY,
					buffer_internal,
					g_object_unref);
	}

	g_return_val_if_fail (CTK_SOURCE_IS_BUFFER_INTERNAL (buffer_internal), NULL);
	return buffer_internal;
}

void
_ctk_source_buffer_internal_emit_search_start (CtkSourceBufferInternal *buffer_internal,
					       CtkSourceSearchContext  *search_context)
{
	g_return_if_fail (CTK_SOURCE_IS_BUFFER_INTERNAL (buffer_internal));
	g_return_if_fail (CTK_SOURCE_IS_SEARCH_CONTEXT (search_context));

	g_signal_emit (buffer_internal,
		       signals[SIGNAL_SEARCH_START],
		       0,
		       search_context);
}
