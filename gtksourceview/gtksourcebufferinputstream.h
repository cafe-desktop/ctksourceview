/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8; coding: utf-8 -*- */
/*
 * This file is part of CtkSourceView
 *
 * Copyright (C) 2010 - Ignacio Casal Quinteiro
 * Copyright (C) 2014 - Sébastien Wilmet <swilmet@gnome.org>
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

#ifndef CTK_SOURCE_BUFFER_INPUT_STREAM_H
#define CTK_SOURCE_BUFFER_INPUT_STREAM_H

#include <gio/gio.h>
#include <ctk/ctk.h>
#include "ctksourcetypes-private.h"
#include "ctksourcebuffer.h"
#include "ctksourcefile.h"

G_BEGIN_DECLS

#define CTK_SOURCE_TYPE_BUFFER_INPUT_STREAM		(_ctk_source_buffer_input_stream_get_type ())
#define CTK_SOURCE_BUFFER_INPUT_STREAM(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), CTK_SOURCE_TYPE_BUFFER_INPUT_STREAM, CtkSourceBufferInputStream))
#define CTK_SOURCE_BUFFER_INPUT_STREAM_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), CTK_SOURCE_TYPE_BUFFER_INPUT_STREAM, CtkSourceBufferInputStreamClass))
#define CTK_SOURCE_IS_BUFFER_INPUT_STREAM(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), CTK_SOURCE_TYPE_BUFFER_INPUT_STREAM))
#define CTK_SOURCE_IS_BUFFER_INPUT_STREAM_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), CTK_SOURCE_TYPE_BUFFER_INPUT_STREAM))
#define CTK_SOURCE_BUFFER_INPUT_STREAM_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), CTK_SOURCE_TYPE_BUFFER_INPUT_STREAM, CtkSourceBufferInputStreamClass))

typedef struct _CtkSourceBufferInputStreamClass		CtkSourceBufferInputStreamClass;
typedef struct _CtkSourceBufferInputStreamPrivate	CtkSourceBufferInputStreamPrivate;

struct _CtkSourceBufferInputStream
{
	GInputStream parent;

	CtkSourceBufferInputStreamPrivate *priv;
};

struct _CtkSourceBufferInputStreamClass
{
	GInputStreamClass parent_class;
};

CTK_SOURCE_INTERNAL
GType		 _ctk_source_buffer_input_stream_get_type		(void) G_GNUC_CONST;

CTK_SOURCE_INTERNAL
CtkSourceBufferInputStream
		*_ctk_source_buffer_input_stream_new			(CtkTextBuffer              *buffer,
									 CtkSourceNewlineType        type,
									 gboolean                    add_trailing_newline);

CTK_SOURCE_INTERNAL
gsize		 _ctk_source_buffer_input_stream_get_total_size		(CtkSourceBufferInputStream *stream);

CTK_SOURCE_INTERNAL
gsize		 _ctk_source_buffer_input_stream_tell			(CtkSourceBufferInputStream *stream);

G_END_DECLS

#endif /* CTK_SOURCE_BUFFER_INPUT_STREAM_H */
