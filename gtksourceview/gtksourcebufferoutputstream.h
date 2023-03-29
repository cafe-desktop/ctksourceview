/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8; coding: utf-8 -*- */
/*
 * This file is part of GtkSourceView
 *
 * Copyright (C) 2010 - Ignacio Casal Quinteiro
 * Copyright (C) 2014 - Sébastien Wilmet <swilmet@gnome.org>
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

#ifndef CTK_SOURCE_BUFFER_OUTPUT_STREAM_H
#define CTK_SOURCE_BUFFER_OUTPUT_STREAM_H

#include <ctk/ctk.h>
#include "ctksourcetypes.h"
#include "ctksourcetypes-private.h"
#include "ctksourcebuffer.h"
#include "ctksourcefile.h"

G_BEGIN_DECLS

#define CTK_SOURCE_TYPE_BUFFER_OUTPUT_STREAM		(ctk_source_buffer_output_stream_get_type ())
#define CTK_SOURCE_BUFFER_OUTPUT_STREAM(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), CTK_SOURCE_TYPE_BUFFER_OUTPUT_STREAM, GtkSourceBufferOutputStream))
#define CTK_SOURCE_BUFFER_OUTPUT_STREAM_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), CTK_SOURCE_TYPE_BUFFER_OUTPUT_STREAM, GtkSourceBufferOutputStreamClass))
#define CTK_SOURCE_IS_BUFFER_OUTPUT_STREAM(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), CTK_SOURCE_TYPE_BUFFER_OUTPUT_STREAM))
#define CTK_SOURCE_IS_BUFFER_OUTPUT_STREAM_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), CTK_SOURCE_TYPE_BUFFER_OUTPUT_STREAM))
#define CTK_SOURCE_BUFFER_OUTPUT_STREAM_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), CTK_SOURCE_TYPE_BUFFER_OUTPUT_STREAM, GtkSourceBufferOutputStreamClass))

typedef struct _GtkSourceBufferOutputStreamClass	GtkSourceBufferOutputStreamClass;
typedef struct _GtkSourceBufferOutputStreamPrivate	GtkSourceBufferOutputStreamPrivate;

struct _GtkSourceBufferOutputStream
{
	GOutputStream parent;

	GtkSourceBufferOutputStreamPrivate *priv;
};

struct _GtkSourceBufferOutputStreamClass
{
	GOutputStreamClass parent_class;
};

CTK_SOURCE_INTERNAL
GType			 ctk_source_buffer_output_stream_get_type	(void) G_GNUC_CONST;

CTK_SOURCE_INTERNAL
GtkSourceBufferOutputStream
			*ctk_source_buffer_output_stream_new		(GtkSourceBuffer             *buffer,
									 GSList                      *candidate_encodings,
									 gboolean                     remove_trailing_newline);

CTK_SOURCE_INTERNAL
GtkSourceNewlineType	 ctk_source_buffer_output_stream_detect_newline_type
									(GtkSourceBufferOutputStream *stream);

CTK_SOURCE_INTERNAL
const GtkSourceEncoding	*ctk_source_buffer_output_stream_get_guessed	(GtkSourceBufferOutputStream *stream);

CTK_SOURCE_INTERNAL
guint			 ctk_source_buffer_output_stream_get_num_fallbacks
									(GtkSourceBufferOutputStream *stream);

G_END_DECLS

#endif /* CTK_SOURCE_BUFFER_OUTPUT_STREAM_H */
