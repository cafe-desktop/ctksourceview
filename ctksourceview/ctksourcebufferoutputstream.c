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

#include "ctksourcebufferoutputstream.h"
#include <string.h>
#include <errno.h>
#include <glib/gi18n-lib.h>
#include "ctksourcebuffer.h"
#include "ctksourcebuffer-private.h"
#include "ctksourceencoding.h"
#include "ctksourcefileloader.h"

/* NOTE: never use async methods on this stream, the stream is just
 * a wrapper around CtkTextBuffer api so that we can use GIO Stream
 * methods, but the underlying code operates on a CtkTextBuffer, so
 * there is no I/O involved and should be accessed only by the main
 * thread.
 */

/* NOTE2: welcome to a really big headache. At the beginning this was
 * split in several classes, one for encoding detection, another
 * for UTF-8 conversion and another for validation. The reason this is
 * all together is because we need specific information from all parts
 * in other to be able to mark characters as invalid if there was some
 * specific problem on the conversion.
 */

/* The code comes from gedit, the class was GeditDocumentOutputStream. */

#if 0
#define DEBUG(x) (x)
#else
#define DEBUG(x)
#endif

#define MAX_UNICHAR_LEN 6

struct _CtkSourceBufferOutputStreamPrivate
{
	CtkSourceBuffer *source_buffer;
	CtkTextIter pos;

	gchar *buffer;
	gsize buflen;

	gchar *iconv_buffer;
	gsize iconv_buflen;

	/* Encoding detection */
	GIConv iconv;
	GCharsetConverter *charset_conv;

	GSList *encodings;
	GSList *current_encoding;

	gint error_offset;
	guint n_fallback_errors;

	guint is_utf8 : 1;
	guint use_first : 1;

	guint is_initialized : 1;
	guint is_closed : 1;

	guint remove_trailing_newline : 1;
};

enum
{
	PROP_0,
	PROP_BUFFER,
	PROP_REMOVE_TRAILING_NEWLINE
};

G_DEFINE_TYPE_WITH_PRIVATE (CtkSourceBufferOutputStream, ctk_source_buffer_output_stream, G_TYPE_OUTPUT_STREAM)

static gssize ctk_source_buffer_output_stream_write   (GOutputStream  *stream,
						       const void     *buffer,
						       gsize           count,
						       GCancellable   *cancellable,
						       GError        **error);

static gboolean ctk_source_buffer_output_stream_close (GOutputStream  *stream,
						       GCancellable   *cancellable,
						       GError        **error);

static gboolean ctk_source_buffer_output_stream_flush (GOutputStream  *stream,
						       GCancellable   *cancellable,
						       GError        **error);

static void
ctk_source_buffer_output_stream_set_property (GObject      *object,
					      guint         prop_id,
					      const GValue *value,
					      GParamSpec   *pspec)
{
	CtkSourceBufferOutputStream *stream = CTK_SOURCE_BUFFER_OUTPUT_STREAM (object);

	switch (prop_id)
	{
		case PROP_BUFFER:
			g_assert (stream->priv->source_buffer == NULL);
			stream->priv->source_buffer = g_value_dup_object (value);
			break;

		case PROP_REMOVE_TRAILING_NEWLINE:
			stream->priv->remove_trailing_newline = g_value_get_boolean (value);
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
ctk_source_buffer_output_stream_get_property (GObject    *object,
					      guint       prop_id,
					      GValue     *value,
					      GParamSpec *pspec)
{
	CtkSourceBufferOutputStream *stream = CTK_SOURCE_BUFFER_OUTPUT_STREAM (object);

	switch (prop_id)
	{
		case PROP_BUFFER:
			g_value_set_object (value, stream->priv->source_buffer);
			break;

		case PROP_REMOVE_TRAILING_NEWLINE:
			g_value_set_boolean (value, stream->priv->remove_trailing_newline);
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
ctk_source_buffer_output_stream_dispose (GObject *object)
{
	CtkSourceBufferOutputStream *stream = CTK_SOURCE_BUFFER_OUTPUT_STREAM (object);

	g_clear_object (&stream->priv->source_buffer);
	g_clear_object (&stream->priv->charset_conv);

	G_OBJECT_CLASS (ctk_source_buffer_output_stream_parent_class)->dispose (object);
}

static void
ctk_source_buffer_output_stream_finalize (GObject *object)
{
	CtkSourceBufferOutputStream *stream = CTK_SOURCE_BUFFER_OUTPUT_STREAM (object);

	g_free (stream->priv->buffer);
	g_free (stream->priv->iconv_buffer);
	g_slist_free (stream->priv->encodings);

	G_OBJECT_CLASS (ctk_source_buffer_output_stream_parent_class)->finalize (object);
}

static void
ctk_source_buffer_output_stream_constructed (GObject *object)
{
	CtkSourceBufferOutputStream *stream = CTK_SOURCE_BUFFER_OUTPUT_STREAM (object);

	if (stream->priv->source_buffer == NULL)
	{
		g_critical ("This should never happen, a problem happened constructing the Buffer Output Stream!");
		return;
	}

	ctk_source_buffer_begin_not_undoable_action (stream->priv->source_buffer);

	ctk_text_buffer_set_text (CTK_TEXT_BUFFER (stream->priv->source_buffer), "", 0);
	ctk_text_buffer_set_modified (CTK_TEXT_BUFFER (stream->priv->source_buffer), FALSE);

	ctk_source_buffer_end_not_undoable_action (stream->priv->source_buffer);

	G_OBJECT_CLASS (ctk_source_buffer_output_stream_parent_class)->constructed (object);
}

static void
ctk_source_buffer_output_stream_class_init (CtkSourceBufferOutputStreamClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	GOutputStreamClass *stream_class = G_OUTPUT_STREAM_CLASS (klass);

	object_class->get_property = ctk_source_buffer_output_stream_get_property;
	object_class->set_property = ctk_source_buffer_output_stream_set_property;
	object_class->dispose = ctk_source_buffer_output_stream_dispose;
	object_class->finalize = ctk_source_buffer_output_stream_finalize;
	object_class->constructed = ctk_source_buffer_output_stream_constructed;

	stream_class->write_fn = ctk_source_buffer_output_stream_write;
	stream_class->close_fn = ctk_source_buffer_output_stream_close;
	stream_class->flush = ctk_source_buffer_output_stream_flush;

	g_object_class_install_property (object_class,
					 PROP_BUFFER,
					 g_param_spec_object ("buffer",
							      "CtkSourceBuffer",
							      "",
							      CTK_SOURCE_TYPE_BUFFER,
							      G_PARAM_READWRITE |
							      G_PARAM_CONSTRUCT_ONLY |
							      G_PARAM_STATIC_STRINGS));

	g_object_class_install_property (object_class,
	                                 PROP_REMOVE_TRAILING_NEWLINE,
	                                 g_param_spec_boolean ("remove-trailing-newline",
	                                                       "Remove trailing newline",
	                                                       "",
	                                                       TRUE,
	                                                       G_PARAM_READWRITE |
	                                                       G_PARAM_CONSTRUCT_ONLY |
	                                                       G_PARAM_STATIC_STRINGS));
}

static void
ctk_source_buffer_output_stream_init (CtkSourceBufferOutputStream *stream)
{
	stream->priv = ctk_source_buffer_output_stream_get_instance_private (stream);

	stream->priv->buffer = NULL;
	stream->priv->buflen = 0;

	stream->priv->charset_conv = NULL;
	stream->priv->encodings = NULL;
	stream->priv->current_encoding = NULL;

	stream->priv->error_offset = -1;

	stream->priv->is_initialized = FALSE;
	stream->priv->is_closed = FALSE;
	stream->priv->is_utf8 = FALSE;
	stream->priv->use_first = FALSE;
}

static const CtkSourceEncoding *
get_encoding (CtkSourceBufferOutputStream *stream)
{
	if (stream->priv->current_encoding == NULL)
	{
		stream->priv->current_encoding = stream->priv->encodings;
	}
	else
	{
		stream->priv->current_encoding = g_slist_next (stream->priv->current_encoding);
	}

	if (stream->priv->current_encoding != NULL)
	{
		return stream->priv->current_encoding->data;
	}

	stream->priv->use_first = TRUE;
	stream->priv->current_encoding = stream->priv->encodings;

	return stream->priv->current_encoding->data;
}

static gboolean
try_convert (GCharsetConverter *converter,
             const void        *inbuf,
             gsize              inbuf_size)
{
	GError *err;
	gsize bytes_read, nread;
	gsize bytes_written, nwritten;
	GConverterResult res;
	gchar *out;
	gboolean ret;
	gsize out_size;

	if (inbuf == NULL || inbuf_size == 0)
	{
		return FALSE;
	}

	err = NULL;
	nread = 0;
	nwritten = 0;
	out_size = inbuf_size * 4;
	out = g_malloc (out_size);

	do
	{
		res = g_converter_convert (G_CONVERTER (converter),
		                           (gchar *)inbuf + nread,
		                           inbuf_size - nread,
		                           (gchar *)out + nwritten,
		                           out_size - nwritten,
		                           G_CONVERTER_INPUT_AT_END,
		                           &bytes_read,
		                           &bytes_written,
		                           &err);

		nread += bytes_read;
		nwritten += bytes_written;
	} while (res != G_CONVERTER_FINISHED && res != G_CONVERTER_ERROR && err == NULL);

	if (err != NULL)
	{
		if (err->code == G_CONVERT_ERROR_PARTIAL_INPUT)
		{
			/* FIXME We can get partial input while guessing the
			   encoding because we just take some amount of text
			   to guess from. */
			ret = TRUE;
		}
		else
		{
			ret = FALSE;
		}

		g_error_free (err);
	}
	else
	{
		ret = TRUE;
	}

	/* FIXME: Check the remainder? */
	if (ret == TRUE && !g_utf8_validate (out, nwritten, NULL))
	{
		ret = FALSE;
	}

	g_free (out);

	return ret;
}

static GCharsetConverter *
guess_encoding (CtkSourceBufferOutputStream *stream,
	       	const void                  *inbuf,
	       	gsize                        inbuf_size)
{
	GCharsetConverter *conv = NULL;

	if (inbuf == NULL || inbuf_size == 0)
	{
		stream->priv->is_utf8 = TRUE;
		return NULL;
	}

	if (stream->priv->encodings != NULL &&
	    stream->priv->encodings->next == NULL)
	{
		stream->priv->use_first = TRUE;
	}

	/* We just check the first block */
	while (TRUE)
	{
		const CtkSourceEncoding *enc;

		g_clear_object (&conv);

		/* We get an encoding from the list */
		enc = get_encoding (stream);

		/* if it is NULL we didn't guess anything */
		if (enc == NULL)
		{
			break;
		}

		DEBUG ({
		       g_print ("trying charset: %s\n",
				ctk_source_encoding_get_charset (stream->priv->current_encoding->data));
		});

		if (enc == ctk_source_encoding_get_utf8 ())
		{
			gsize remainder;
			const gchar *end;

			if (g_utf8_validate (inbuf, inbuf_size, &end) ||
			    stream->priv->use_first)
			{
				stream->priv->is_utf8 = TRUE;
				break;
			}

			/* Check if the end is less than one char */
			remainder = inbuf_size - (end - (gchar *)inbuf);
			if (remainder < 6)
			{
				stream->priv->is_utf8 = TRUE;
				break;
			}

			continue;
		}

		conv = g_charset_converter_new ("UTF-8",
						ctk_source_encoding_get_charset (enc),
						NULL);

		/* If we tried all encodings we use the first one */
		if (stream->priv->use_first)
		{
			break;
		}

		/* Try to convert */
		if (try_convert (conv, inbuf, inbuf_size))
		{
			break;
		}
	}

	if (conv != NULL)
	{
		g_converter_reset (G_CONVERTER (conv));
	}

	return conv;
}

static CtkSourceNewlineType
get_newline_type (CtkTextIter *end)
{
	CtkSourceNewlineType res;
	CtkTextIter copy;
	gunichar c;

	copy = *end;
	c = ctk_text_iter_get_char (&copy);

	if (g_unichar_break_type (c) == G_UNICODE_BREAK_CARRIAGE_RETURN)
	{
		if (ctk_text_iter_forward_char (&copy) &&
		    g_unichar_break_type (ctk_text_iter_get_char (&copy)) == G_UNICODE_BREAK_LINE_FEED)
		{
			res = CTK_SOURCE_NEWLINE_TYPE_CR_LF;
		}
		else
		{
			res = CTK_SOURCE_NEWLINE_TYPE_CR;
		}
	}
	else
	{
		res = CTK_SOURCE_NEWLINE_TYPE_LF;
	}

	return res;
}

CtkSourceBufferOutputStream *
ctk_source_buffer_output_stream_new (CtkSourceBuffer *buffer,
				     GSList          *candidate_encodings,
				     gboolean         remove_trailing_newline)
{
	CtkSourceBufferOutputStream *stream;

	stream = g_object_new (CTK_SOURCE_TYPE_BUFFER_OUTPUT_STREAM,
	                       "buffer", buffer,
	                       "remove-trailing-newline", remove_trailing_newline,
	                       NULL);

	stream->priv->encodings = g_slist_copy (candidate_encodings);

	return stream;
}

CtkSourceNewlineType
ctk_source_buffer_output_stream_detect_newline_type (CtkSourceBufferOutputStream *stream)
{
	CtkSourceNewlineType type;
	CtkTextIter iter;

	g_return_val_if_fail (CTK_SOURCE_IS_BUFFER_OUTPUT_STREAM (stream),
			      CTK_SOURCE_NEWLINE_TYPE_DEFAULT);

	if (stream->priv->source_buffer == NULL)
	{
		return CTK_SOURCE_NEWLINE_TYPE_DEFAULT;
	}

	type = CTK_SOURCE_NEWLINE_TYPE_DEFAULT;

	ctk_text_buffer_get_start_iter (CTK_TEXT_BUFFER (stream->priv->source_buffer),
					&iter);

	if (ctk_text_iter_ends_line (&iter) || ctk_text_iter_forward_to_line_end (&iter))
	{
		type = get_newline_type (&iter);
	}

	return type;
}

const CtkSourceEncoding *
ctk_source_buffer_output_stream_get_guessed (CtkSourceBufferOutputStream *stream)
{
	g_return_val_if_fail (CTK_SOURCE_IS_BUFFER_OUTPUT_STREAM (stream), NULL);

	if (stream->priv->current_encoding != NULL)
	{
		return stream->priv->current_encoding->data;
	}
	else if (stream->priv->is_utf8 || !stream->priv->is_initialized)
	{
		/* If it is not initialized we assume that we are trying to
		 * convert the empty string.
		 */
		return ctk_source_encoding_get_utf8 ();
	}

	return NULL;
}

guint
ctk_source_buffer_output_stream_get_num_fallbacks (CtkSourceBufferOutputStream *stream)
{
	g_return_val_if_fail (CTK_SOURCE_IS_BUFFER_OUTPUT_STREAM (stream), 0);

	return stream->priv->n_fallback_errors;
}

static void
apply_error_tag (CtkSourceBufferOutputStream *stream)
{
	CtkTextIter start;

	if (stream->priv->error_offset == -1 ||
	    stream->priv->source_buffer == NULL)
	{
		return;
	}

	ctk_text_buffer_get_iter_at_offset (CTK_TEXT_BUFFER (stream->priv->source_buffer),
	                                    &start, stream->priv->error_offset);

	_ctk_source_buffer_set_as_invalid_character (stream->priv->source_buffer,
						     &start,
						     &stream->priv->pos);

	stream->priv->error_offset = -1;
}

static void
insert_fallback (CtkSourceBufferOutputStream *stream,
		 const gchar                 *buffer)
{
	guint8 out[4];
	guint8 v;
	const gchar hex[] = "0123456789ABCDEF";

	if (stream->priv->source_buffer == NULL)
	{
		return;
	}

	/* If we are here it is because we are pointing to an invalid char so we
	 * substitute it by an hex value.
	 */
	v = *(guint8 *)buffer;
	out[0] = '\\';
	out[1] = hex[(v & 0xf0) >> 4];
	out[2] = hex[(v & 0x0f) >> 0];
	out[3] = '\0';

	ctk_text_buffer_insert (CTK_TEXT_BUFFER (stream->priv->source_buffer),
	                        &stream->priv->pos, (const gchar *)out, 3);

	++stream->priv->n_fallback_errors;
}

static void
validate_and_insert (CtkSourceBufferOutputStream *stream,
		     gchar                       *buffer,
		     gsize                        count,
		     gboolean                     owned)
{
	CtkTextBuffer *text_buffer;
	CtkTextIter *iter;
	gsize len;
	gchar *free_text = NULL;

	if (stream->priv->source_buffer == NULL)
	{
		return;
	}

	text_buffer = CTK_TEXT_BUFFER (stream->priv->source_buffer);
	iter = &stream->priv->pos;
	len = count;

	while (len != 0)
	{
		const gchar *end;
		gboolean valid;
		gsize nvalid;

		/* validate */
		valid = g_utf8_validate (buffer, len, &end);
		nvalid = end - buffer;

		/* Note: this is a workaround for a 'bug' in CtkTextBuffer where
		   inserting first a \r and then in a second insert, a \n,
		   will result in two lines being added instead of a single
		   one */

		if (valid)
		{
			gchar *ptr;

			ptr = g_utf8_find_prev_char (buffer, buffer + len);

			if (ptr && *ptr == '\r' && ptr - buffer == (glong)len - 1)
			{
				stream->priv->buffer = g_new (gchar, 2);
				stream->priv->buffer[0] = '\r';
				stream->priv->buffer[1] = '\0';
				stream->priv->buflen = 1;

				/* Decrease also the len so in the check
				   nvalid == len we get out of this method */
				--nvalid;
				--len;
			}
		}

		/* if we've got any valid char we must tag the invalid chars */
		if (nvalid > 0)
		{
			gchar orig_non_null = '\0';

			apply_error_tag (stream);

			if ((nvalid != len || !owned) && buffer[nvalid] != '\0')
			{
				/* make sure the buffer is always properly null
				 * terminated. This is needed, at least for now,
				 * to avoid issues with pygobject marshalling of
				 * the insert-text signal of ctktextbuffer
				 *
				 * https://bugzilla.gnome.org/show_bug.cgi?id=726689
				 */
				if (!owned)
				{
					/* forced to make a copy */
					free_text = g_new (gchar, len + 1);
					memcpy (free_text, buffer, len);
					free_text[len] = '\0';

					buffer = free_text;
					owned = TRUE;
				}

				orig_non_null = buffer[nvalid];
				buffer[nvalid] = '\0';
			}

			ctk_text_buffer_insert (text_buffer, iter, buffer, nvalid);

			if (orig_non_null != '\0')
			{
				/* restore null terminated replaced byte */
				buffer[nvalid] = orig_non_null;
			}
		}

		/* If we inserted all return */
		if (nvalid == len)
		{
			break;
		}

		buffer += nvalid;
		len = len - nvalid;

		if ((len < MAX_UNICHAR_LEN) &&
		    (g_utf8_get_char_validated (buffer, len) == (gunichar)-2))
		{
			stream->priv->buffer = g_strndup (end, len);
			stream->priv->buflen = len;

			break;
		}

		/* we need the start of the chunk of invalid chars */
		if (stream->priv->error_offset == -1)
		{
			stream->priv->error_offset = ctk_text_iter_get_offset (&stream->priv->pos);
		}

		insert_fallback (stream, buffer);
		++buffer;
		--len;
	}

	g_free (free_text);
}

static void
remove_trailing_newline (CtkSourceBufferOutputStream *stream)
{
	CtkTextIter end;
	CtkTextIter start;

	if (stream->priv->source_buffer == NULL)
	{
		return;
	}

	ctk_text_buffer_get_end_iter (CTK_TEXT_BUFFER (stream->priv->source_buffer), &end);
	start = end;

	ctk_text_iter_set_line_offset (&start, 0);

	if (ctk_text_iter_ends_line (&start) &&
	    ctk_text_iter_backward_line (&start))
	{
		if (!ctk_text_iter_ends_line (&start))
		{
			ctk_text_iter_forward_to_line_end (&start);
		}

		ctk_text_buffer_delete (CTK_TEXT_BUFFER (stream->priv->source_buffer),
		                        &start,
		                        &end);
	}
}

static void
end_append_text_to_document (CtkSourceBufferOutputStream *stream)
{
	if (stream->priv->source_buffer == NULL)
	{
		return;
	}

	if (stream->priv->remove_trailing_newline)
	{
		remove_trailing_newline (stream);
	}

	ctk_text_buffer_set_modified (CTK_TEXT_BUFFER (stream->priv->source_buffer),
	                              FALSE);

	ctk_text_buffer_end_user_action (CTK_TEXT_BUFFER (stream->priv->source_buffer));
	ctk_source_buffer_end_not_undoable_action (stream->priv->source_buffer);
}

static gboolean
convert_text (CtkSourceBufferOutputStream  *stream,
	      const gchar                  *inbuf,
	      gsize                         inbuf_len,
	      gchar                       **outbuf,
	      gsize                        *outbuf_len,
	      GError                      **error)
{
	gchar *out, *dest;
	gsize in_left, out_left, outbuf_size, res;
	gint errsv;
	gboolean done, have_error;

	in_left = inbuf_len;
	/* set an arbitrary length if inbuf_len is 0, this is needed to flush
	   the iconv data */
	outbuf_size = (inbuf_len > 0) ? inbuf_len : 100;

	out_left = outbuf_size;

	/* keep room for null termination */
	out = dest = g_malloc (sizeof (gchar) * (outbuf_size + 1));

	done = FALSE;
	have_error = FALSE;

	while (!done && !have_error)
	{
		/* If we reached here is because we need to convert the text,
		   so we convert it using iconv.
		   See that if inbuf is NULL the data will be flushed */
		res = g_iconv (stream->priv->iconv,
		               (gchar **)&inbuf, &in_left,
		               &out, &out_left);

		/* something went wrong */
		if (res == (gsize)-1)
		{
			errsv = errno;

			switch (errsv)
			{
				case EINVAL:
					/* Incomplete text, do not report an error */
					stream->priv->iconv_buffer = g_strndup (inbuf, in_left);
					stream->priv->iconv_buflen = in_left;
					done = TRUE;
					break;

				case E2BIG:
					{
						/* allocate more space */
						gsize used = out - dest;

						outbuf_size *= 2;

						/* make sure to allocate room for
						   terminating null byte */
						dest = g_realloc (dest, outbuf_size + 1);

						out = dest + used;
						out_left = outbuf_size - used;
					}
					break;

				case EILSEQ:
					g_set_error_literal (error, G_CONVERT_ERROR,
					                     G_CONVERT_ERROR_ILLEGAL_SEQUENCE,
					                     _("Invalid byte sequence in conversion input"));
					have_error = TRUE;
					break;

				default:
					g_set_error (error, G_CONVERT_ERROR, G_CONVERT_ERROR_FAILED,
					             _("Error during conversion: %s"),
					             g_strerror (errsv));
					have_error = TRUE;
					break;
			}
		}
		else
		{
			done = TRUE;
		}
	}

	if (have_error)
	{
		g_free (dest);
		*outbuf = NULL;
		*outbuf_len = 0;

		return FALSE;
	}

	*outbuf_len = out - dest;
	dest[*outbuf_len] = '\0';

	*outbuf = dest;
	return TRUE;
}

static gssize
ctk_source_buffer_output_stream_write (GOutputStream  *stream,
				       const void     *buffer,
				       gsize           count,
				       GCancellable   *cancellable,
				       GError        **error)
{
	CtkSourceBufferOutputStream *ostream;
	gchar *text;
	gsize len;
	gboolean freetext = FALSE;

	ostream = CTK_SOURCE_BUFFER_OUTPUT_STREAM (stream);

	if (g_cancellable_set_error_if_cancelled (cancellable, error) ||
	    ostream->priv->source_buffer == NULL)
	{
		return -1;
	}

	if (!ostream->priv->is_initialized)
	{
		ostream->priv->charset_conv = guess_encoding (ostream, buffer, count);

		/* If we still have the previous case is that we didn't guess
		   anything */
		if (ostream->priv->charset_conv == NULL &&
		    !ostream->priv->is_utf8)
		{
			g_set_error_literal (error, CTK_SOURCE_FILE_LOADER_ERROR,
			                     CTK_SOURCE_FILE_LOADER_ERROR_ENCODING_AUTO_DETECTION_FAILED,
			                     "It is not possible to detect the encoding automatically");

			return -1;
		}

		/* Do not initialize iconv if we are not going to convert anything */
		if (!ostream->priv->is_utf8)
		{
			gchar *from_charset;

			/* Initialize iconv */
			g_object_get (G_OBJECT (ostream->priv->charset_conv),
				      "from-charset", &from_charset,
				      NULL);

			ostream->priv->iconv = g_iconv_open ("UTF-8", from_charset);

			if (ostream->priv->iconv == (GIConv)-1)
			{
				if (errno == EINVAL)
				{
					g_set_error (error, G_IO_ERROR, G_IO_ERROR_NOT_SUPPORTED,
						     _("Conversion from character set “%s” to “UTF-8” is not supported"),
						     from_charset);
				}
				else
				{
					g_set_error (error, G_IO_ERROR, G_IO_ERROR_FAILED,
						     _("Could not open converter from “%s” to “UTF-8”"),
						     from_charset);
				}

				g_free (from_charset);
				g_clear_object (&ostream->priv->charset_conv);

				return -1;
			}

			g_free (from_charset);
		}

		/* Begin not undoable action. Begin also a normal user action,
		 * since we load the file chunk by chunk and it should be seen
		 * as only one action, for the features that rely on the user
		 * action.
		 */
		ctk_source_buffer_begin_not_undoable_action (ostream->priv->source_buffer);
		ctk_text_buffer_begin_user_action (CTK_TEXT_BUFFER (ostream->priv->source_buffer));

		ctk_text_buffer_get_start_iter (CTK_TEXT_BUFFER (ostream->priv->source_buffer),
		                                &ostream->priv->pos);

		ostream->priv->is_initialized = TRUE;
	}

	if (ostream->priv->buflen > 0)
	{
		len = ostream->priv->buflen + count;
		text = g_malloc (len + 1);

		memcpy (text, ostream->priv->buffer, ostream->priv->buflen);
		memcpy (text + ostream->priv->buflen, buffer, count);

		text[len] = '\0';

		g_free (ostream->priv->buffer);

		ostream->priv->buffer = NULL;
		ostream->priv->buflen = 0;

		freetext = TRUE;
	}
	else
	{
		text = (gchar *) buffer;
		len = count;
	}

	if (!ostream->priv->is_utf8)
	{
		gchar *outbuf;
		gsize outbuf_len;

		/* check if iconv was correctly initializated, this shouldn't
		   happen but better be safe */
		if (ostream->priv->iconv == NULL)
		{
			g_set_error_literal (error, G_IO_ERROR, G_IO_ERROR_NOT_INITIALIZED,
			                     _("Invalid object, not initialized"));

			if (freetext)
			{
				g_free (text);
			}

			return -1;
		}

		/* manage the previous conversion buffer */
		if (ostream->priv->iconv_buflen > 0)
		{
			gchar *text2;
			gsize len2;

			len2 = len + ostream->priv->iconv_buflen;
			text2 = g_malloc (len2 + 1);

			memcpy (text2, ostream->priv->iconv_buffer, ostream->priv->iconv_buflen);
			memcpy (text2 + ostream->priv->iconv_buflen, text, len);

			text2[len2] = '\0';

			if (freetext)
			{
				g_free (text);
			}

			text = text2;
			len = len2;

			g_free (ostream->priv->iconv_buffer);

			ostream->priv->iconv_buffer = NULL;
			ostream->priv->iconv_buflen = 0;

			freetext = TRUE;
		}

		if (!convert_text (ostream, text, len, &outbuf, &outbuf_len, error))
		{
			if (freetext)
			{
				g_free (text);
			}

			return -1;
		}

		if (freetext)
		{
			g_free (text);
		}

		/* set the converted text as the text to validate */
		text = outbuf;
		len = outbuf_len;
	}

	validate_and_insert (ostream, text, len, freetext);

	if (freetext)
	{
		g_free (text);
	}

	return count;
}

static gboolean
ctk_source_buffer_output_stream_flush (GOutputStream  *stream,
				       GCancellable   *cancellable,
				       GError        **error)
{
	CtkSourceBufferOutputStream *ostream;

	ostream = CTK_SOURCE_BUFFER_OUTPUT_STREAM (stream);

	if (ostream->priv->is_closed ||
	    ostream->priv->source_buffer == NULL)
	{
		return TRUE;
	}

	/* if we have converted something flush residual data, validate and insert */
	if (ostream->priv->iconv != NULL)
	{
		gchar *outbuf;
		gsize outbuf_len;

		if (convert_text (ostream, NULL, 0, &outbuf, &outbuf_len, error))
		{
			validate_and_insert (ostream, outbuf, outbuf_len, TRUE);
			g_free (outbuf);
		}
		else
		{
			return FALSE;
		}
	}

	if (ostream->priv->buflen > 0 && *ostream->priv->buffer != '\r')
	{
		/* If we reached here is because the last insertion was a half
		   correct char, which has to be inserted as fallback */
		gchar *text;

		if (ostream->priv->error_offset == -1)
		{
			ostream->priv->error_offset = ctk_text_iter_get_offset (&ostream->priv->pos);
		}

		text = ostream->priv->buffer;
		while (ostream->priv->buflen != 0)
		{
			insert_fallback (ostream, text);
			++text;
			--ostream->priv->buflen;
		}

		g_free (ostream->priv->buffer);
		ostream->priv->buffer = NULL;
	}
	else if (ostream->priv->buflen == 1 && *ostream->priv->buffer == '\r')
	{
		/* The previous chars can be invalid */
		apply_error_tag (ostream);

		/* See special case above, flush this */
		ctk_text_buffer_insert (CTK_TEXT_BUFFER (ostream->priv->source_buffer),
		                        &ostream->priv->pos,
		                        "\r",
		                        1);

		g_free (ostream->priv->buffer);
		ostream->priv->buffer = NULL;
		ostream->priv->buflen = 0;
	}

	if (ostream->priv->iconv_buflen > 0 )
	{
		/* If we reached here is because the last insertion was a half
		   correct char, which has to be inserted as fallback */
		gchar *text;

		if (ostream->priv->error_offset == -1)
		{
			ostream->priv->error_offset = ctk_text_iter_get_offset (&ostream->priv->pos);
		}

		text = ostream->priv->iconv_buffer;
		while (ostream->priv->iconv_buflen != 0)
		{
			insert_fallback (ostream, text);
			++text;
			--ostream->priv->iconv_buflen;
		}

		g_free (ostream->priv->iconv_buffer);
		ostream->priv->iconv_buffer = NULL;
	}

	apply_error_tag (ostream);

	return TRUE;
}

static gboolean
ctk_source_buffer_output_stream_close (GOutputStream  *stream,
				       GCancellable   *cancellable,
				       GError        **error)
{
	CtkSourceBufferOutputStream *ostream = CTK_SOURCE_BUFFER_OUTPUT_STREAM (stream);

	if (!ostream->priv->is_closed && ostream->priv->is_initialized)
	{
		end_append_text_to_document (ostream);

		if (ostream->priv->iconv != NULL)
		{
			g_iconv_close (ostream->priv->iconv);
		}

		ostream->priv->is_closed = TRUE;
	}

	if (ostream->priv->buflen > 0 || ostream->priv->iconv_buflen > 0)
	{
		g_set_error (error,
		             G_IO_ERROR,
		             G_IO_ERROR_INVALID_DATA,
		             _("Incomplete UTF-8 sequence in input"));

		return FALSE;
	}

	return TRUE;
}
