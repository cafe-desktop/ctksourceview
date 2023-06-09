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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <glib.h>
#include <gio/gio.h>
#include <string.h>
#include "ctksourcebufferinputstream.h"
#include "ctksource-enumtypes.h"

/* NOTE: never use async methods on this stream, the stream is just
 * a wrapper around CtkTextBuffer api so that we can use GIO Stream
 * methods, but the underlying code operates on a CtkTextBuffer, so
 * there is no I/O involved and should be accessed only by the main
 * thread.
 */

struct _CtkSourceBufferInputStreamPrivate
{
	CtkTextBuffer *buffer;
	CtkTextMark *pos;
	gint bytes_partial;

	CtkSourceNewlineType newline_type;

	guint newline_added : 1;
	guint is_initialized : 1;
	guint add_trailing_newline : 1;
};

enum
{
	PROP_0,
	PROP_BUFFER,
	PROP_NEWLINE_TYPE,
	PROP_ADD_TRAILING_NEWLINE
};

G_DEFINE_TYPE_WITH_PRIVATE (CtkSourceBufferInputStream, _ctk_source_buffer_input_stream, G_TYPE_INPUT_STREAM);

static gsize
get_new_line_size (CtkSourceBufferInputStream *stream)
{
	switch (stream->priv->newline_type)
	{
		case CTK_SOURCE_NEWLINE_TYPE_CR:
		case CTK_SOURCE_NEWLINE_TYPE_LF:
			return 1;

		case CTK_SOURCE_NEWLINE_TYPE_CR_LF:
			return 2;

		default:
			g_warn_if_reached ();
			break;
	}

	return 1;
}

static const gchar *
get_new_line (CtkSourceBufferInputStream *stream)
{
	switch (stream->priv->newline_type)
	{
		case CTK_SOURCE_NEWLINE_TYPE_LF:
			return "\n";

		case CTK_SOURCE_NEWLINE_TYPE_CR:
			return "\r";

		case CTK_SOURCE_NEWLINE_TYPE_CR_LF:
			return "\r\n";

		default:
			g_warn_if_reached ();
			break;
	}

	return "\n";
}

static gsize
read_line (CtkSourceBufferInputStream *stream,
	   gchar                      *outbuf,
	   gsize                       space_left)
{
	CtkTextIter start, next, end;
	gchar *buf;
	gint bytes; /* int since it's what iter_get_offset returns */
	gsize bytes_to_write, newline_size, read;
	const gchar *newline;
	gboolean is_last;

	if (stream->priv->buffer == NULL)
	{
		return 0;
	}

	ctk_text_buffer_get_iter_at_mark (stream->priv->buffer,
					  &start,
					  stream->priv->pos);

	if (ctk_text_iter_is_end (&start))
	{
		return 0;
	}

	end = next = start;
	newline = get_new_line (stream);

	/* Check needed for empty lines */
	if (!ctk_text_iter_ends_line (&end))
	{
		ctk_text_iter_forward_to_line_end (&end);
	}

	ctk_text_iter_forward_line (&next);

	buf = ctk_text_iter_get_slice (&start, &end);

	/* the bytes of a line includes also the newline, so with the
	   offsets we remove the newline and we add the new newline size */
	bytes = ctk_text_iter_get_bytes_in_line (&start) - stream->priv->bytes_partial;

	/* bytes_in_line includes the newlines, so we remove that assuming that
	   they are single byte characters */
	bytes -= ctk_text_iter_get_offset (&next) - ctk_text_iter_get_offset (&end);
	is_last = ctk_text_iter_is_end (&end);

	/* bytes_to_write contains the amount of bytes we would like to write.
	   This means its the amount of bytes in the line (without the newline
	   in the buffer) + the amount of bytes for the newline we want to
	   write (newline_size) */
	bytes_to_write = bytes;

	/* do not add the new newline_size for the last line */
	newline_size = get_new_line_size (stream);
	if (!is_last)
	{
		bytes_to_write += newline_size;
	}

	if (bytes_to_write > space_left)
	{
		gchar *ptr;
		gint char_offset;
		gint written;
		glong to_write;

		/* Here the line does not fit in the buffer, we thus write
		   the amount of bytes we can still fit, storing the position
		   for the next read with the mark. Do not try to write the
		   new newline in this case, it will be handled in the next
		   iteration */
		to_write = MIN ((glong)space_left, bytes);
		ptr = buf;
		written = 0;
		char_offset = 0;

		while (written < to_write)
		{
			gint w;

			ptr = g_utf8_next_char (ptr);
			w = (ptr - buf);
			if (w > to_write)
			{
				break;
			}
			else
			{
				written = w;
				++char_offset;
			}
		}

		memcpy (outbuf, buf, written);

		/* Note: offset is one past what we wrote */
		ctk_text_iter_forward_chars (&start, char_offset);
		stream->priv->bytes_partial += written;
		read = written;
	}
	else
	{
		/* First just copy the bytes without the newline */
		memcpy (outbuf, buf, bytes);

		/* Then add the newline, but not for the last line */
		if (!is_last)
		{
			memcpy (outbuf + bytes, newline, newline_size);
		}

		start = next;
		stream->priv->bytes_partial = 0;
		read = bytes_to_write;
	}

	ctk_text_buffer_move_mark (stream->priv->buffer,
				   stream->priv->pos,
				   &start);

	g_free (buf);
	return read;
}

static gssize
_ctk_source_buffer_input_stream_read (GInputStream  *input_stream,
				      void          *buffer,
				      gsize          count,
				      GCancellable  *cancellable,
				      GError       **error)
{
	CtkSourceBufferInputStream *stream;
	CtkTextIter iter;
	gssize space_left, read, n;

	stream = CTK_SOURCE_BUFFER_INPUT_STREAM (input_stream);

	if (count < 6)
	{
		g_set_error_literal (error, G_IO_ERROR, G_IO_ERROR_NO_SPACE,
				     "Not enougth space in destination");
		return -1;
	}

	if (g_cancellable_set_error_if_cancelled (cancellable, error))
	{
		return -1;
	}

	if (stream->priv->buffer == NULL)
	{
		return 0;
	}

	/* Initialize the mark to the first char in the text buffer */
	if (!stream->priv->is_initialized)
	{
		ctk_text_buffer_get_start_iter (stream->priv->buffer, &iter);
		stream->priv->pos = ctk_text_buffer_create_mark (stream->priv->buffer,
								 NULL,
								 &iter,
								 FALSE);

		stream->priv->is_initialized = TRUE;
	}

	space_left = count;
	read = 0;

	do
	{
		n = read_line (stream, (gchar *)buffer + read, space_left);
		read += n;
		space_left -= n;
	} while (space_left > 0 && n != 0 && stream->priv->bytes_partial == 0);

	/* Make sure that non-empty files are always terminated with \n (see bug #95676).
	 * Note that we strip the trailing \n when loading the file */
	ctk_text_buffer_get_iter_at_mark (stream->priv->buffer,
					  &iter,
					  stream->priv->pos);

	if (ctk_text_iter_is_end (&iter) &&
	    !ctk_text_iter_is_start (&iter) &&
	    stream->priv->add_trailing_newline)
	{
		gssize newline_size;

		newline_size = get_new_line_size (stream);

		if (space_left >= newline_size &&
		    !stream->priv->newline_added)
		{
			const gchar *newline;

			newline = get_new_line (stream);

			memcpy ((gchar *)buffer + read, newline, newline_size);

			read += newline_size;
			stream->priv->newline_added = TRUE;
		}
	}

	return read;
}

static gboolean
_ctk_source_buffer_input_stream_close (GInputStream  *input_stream,
				       GCancellable  *cancellable,
				       GError       **error)
{
	CtkSourceBufferInputStream *stream = CTK_SOURCE_BUFFER_INPUT_STREAM (input_stream);

	stream->priv->newline_added = FALSE;

	if (stream->priv->is_initialized &&
	    stream->priv->buffer != NULL)
	{
		ctk_text_buffer_delete_mark (stream->priv->buffer, stream->priv->pos);
	}

	return TRUE;
}

static void
_ctk_source_buffer_input_stream_set_property (GObject      *object,
					      guint         prop_id,
					      const GValue *value,
					      GParamSpec   *pspec)
{
	CtkSourceBufferInputStream *stream = CTK_SOURCE_BUFFER_INPUT_STREAM (object);

	switch (prop_id)
	{
		case PROP_BUFFER:
			g_assert (stream->priv->buffer == NULL);
			stream->priv->buffer = g_value_dup_object (value);
			break;

		case PROP_NEWLINE_TYPE:
			stream->priv->newline_type = g_value_get_enum (value);
			break;

		case PROP_ADD_TRAILING_NEWLINE:
			stream->priv->add_trailing_newline = g_value_get_boolean (value);
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
_ctk_source_buffer_input_stream_get_property (GObject    *object,
					  guint       prop_id,
					  GValue     *value,
					  GParamSpec *pspec)
{
	CtkSourceBufferInputStream *stream = CTK_SOURCE_BUFFER_INPUT_STREAM (object);

	switch (prop_id)
	{
		case PROP_BUFFER:
			g_value_set_object (value, stream->priv->buffer);
			break;

		case PROP_NEWLINE_TYPE:
			g_value_set_enum (value, stream->priv->newline_type);
			break;

		case PROP_ADD_TRAILING_NEWLINE:
			g_value_set_boolean (value, stream->priv->add_trailing_newline);
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
_ctk_source_buffer_input_stream_dispose (GObject *object)
{
	CtkSourceBufferInputStream *stream = CTK_SOURCE_BUFFER_INPUT_STREAM (object);

	g_clear_object (&stream->priv->buffer);

	G_OBJECT_CLASS (_ctk_source_buffer_input_stream_parent_class)->dispose (object);
}

static void
_ctk_source_buffer_input_stream_class_init (CtkSourceBufferInputStreamClass *klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
	GInputStreamClass *stream_class = G_INPUT_STREAM_CLASS (klass);

	gobject_class->get_property = _ctk_source_buffer_input_stream_get_property;
	gobject_class->set_property = _ctk_source_buffer_input_stream_set_property;
	gobject_class->dispose = _ctk_source_buffer_input_stream_dispose;

	stream_class->read_fn = _ctk_source_buffer_input_stream_read;
	stream_class->close_fn = _ctk_source_buffer_input_stream_close;

	g_object_class_install_property (gobject_class,
					 PROP_BUFFER,
					 g_param_spec_object ("buffer",
							      "CtkTextBuffer",
							      "",
							      CTK_TYPE_TEXT_BUFFER,
							      G_PARAM_READWRITE |
							      G_PARAM_CONSTRUCT_ONLY |
							      G_PARAM_STATIC_STRINGS));

	/**
	 * CtkSourceBufferInputStream:newline-type:
	 *
	 * The :newline-type property determines what is considered
	 * as a line ending when reading complete lines from the stream.
	 */
	g_object_class_install_property (gobject_class,
					 PROP_NEWLINE_TYPE,
					 g_param_spec_enum ("newline-type",
							    "Newline type",
							    "",
							    CTK_SOURCE_TYPE_NEWLINE_TYPE,
							    CTK_SOURCE_NEWLINE_TYPE_LF,
							    G_PARAM_READWRITE |
							    G_PARAM_STATIC_STRINGS |
							    G_PARAM_CONSTRUCT_ONLY));

	/**
	 * CtkSourceBufferInputStream:add-trailing-newline:
	 *
	 * The :add-trailing-newline property specifies whether or not to
	 * add a trailing newline when reading the buffer.
	 */
	g_object_class_install_property (gobject_class,
	                                 PROP_ADD_TRAILING_NEWLINE,
	                                 g_param_spec_boolean ("add-trailing-newline",
	                                                       "Add trailing newline",
	                                                       "",
	                                                       TRUE,
	                                                       G_PARAM_READWRITE |
	                                                       G_PARAM_STATIC_STRINGS |
	                                                       G_PARAM_CONSTRUCT_ONLY));
}

static void
_ctk_source_buffer_input_stream_init (CtkSourceBufferInputStream *stream)
{
	stream->priv = _ctk_source_buffer_input_stream_get_instance_private (stream);
}

/**
 * _ctk_source_buffer_input_stream_new:
 * @buffer: a #CtkTextBuffer
 *
 * Reads the data from @buffer.
 *
 * Returns: a new input stream to read @buffer
 */
CtkSourceBufferInputStream *
_ctk_source_buffer_input_stream_new (CtkTextBuffer        *buffer,
				     CtkSourceNewlineType  type,
				     gboolean              add_trailing_newline)
{
	g_return_val_if_fail (CTK_IS_TEXT_BUFFER (buffer), NULL);

	return g_object_new (CTK_SOURCE_TYPE_BUFFER_INPUT_STREAM,
			     "buffer", buffer,
			     "newline-type", type,
			     "add-trailing-newline", add_trailing_newline,
			     NULL);
}

gsize
_ctk_source_buffer_input_stream_get_total_size (CtkSourceBufferInputStream *stream)
{
	g_return_val_if_fail (CTK_SOURCE_IS_BUFFER_INPUT_STREAM (stream), 0);

	if (stream->priv->buffer == NULL)
	{
		return 0;
	}

	return ctk_text_buffer_get_char_count (stream->priv->buffer);
}

gsize
_ctk_source_buffer_input_stream_tell (CtkSourceBufferInputStream *stream)
{
	g_return_val_if_fail (CTK_SOURCE_IS_BUFFER_INPUT_STREAM (stream), 0);

	/* FIXME: is this potentially inefficient? If yes, we could keep
	   track of the offset internally, assuming the mark doesn't move
	   during the operation */
	if (!stream->priv->is_initialized ||
	    stream->priv->buffer == NULL)
	{
		return 0;
	}
	else
	{
		CtkTextIter iter;

		ctk_text_buffer_get_iter_at_mark (stream->priv->buffer,
						  &iter,
						  stream->priv->pos);
		return ctk_text_iter_get_offset (&iter);
	}
}
