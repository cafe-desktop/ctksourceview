/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8; coding: utf-8 -*- */
/*
 * This file is part of GtkSourceView
 *
 * Copyright (C) 2005 - Paolo Maggi
 * Copyright (C) 2007 - Paolo Maggi, Steve Frécinaux
 * Copyright (C) 2008 - Jesse van den Kieboom
 * Copyright (C) 2014 - Sébastien Wilmet
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

#ifndef CTK_SOURCE_FILE_LOADER_H
#define CTK_SOURCE_FILE_LOADER_H

#if !defined (CTK_SOURCE_H_INSIDE) && !defined (CTK_SOURCE_COMPILATION)
#error "Only <ctksourceview/ctksource.h> can be included directly."
#endif

#include <ctk/ctk.h>
#include <ctksourceview/ctksourcetypes.h>
#include <ctksourceview/ctksourcefile.h>

G_BEGIN_DECLS

#define CTK_SOURCE_TYPE_FILE_LOADER              (ctk_source_file_loader_get_type())
#define CTK_SOURCE_FILE_LOADER(obj)              (G_TYPE_CHECK_INSTANCE_CAST((obj), CTK_SOURCE_TYPE_FILE_LOADER, GtkSourceFileLoader))
#define CTK_SOURCE_FILE_LOADER_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass), CTK_SOURCE_TYPE_FILE_LOADER, GtkSourceFileLoaderClass))
#define CTK_SOURCE_IS_FILE_LOADER(obj)           (G_TYPE_CHECK_INSTANCE_TYPE((obj), CTK_SOURCE_TYPE_FILE_LOADER))
#define CTK_SOURCE_IS_FILE_LOADER_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), CTK_SOURCE_TYPE_FILE_LOADER))
#define CTK_SOURCE_FILE_LOADER_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS((obj), CTK_SOURCE_TYPE_FILE_LOADER, GtkSourceFileLoaderClass))

typedef struct _GtkSourceFileLoaderClass   GtkSourceFileLoaderClass;
typedef struct _GtkSourceFileLoaderPrivate GtkSourceFileLoaderPrivate;

#define CTK_SOURCE_FILE_LOADER_ERROR ctk_source_file_loader_error_quark ()

/**
 * GtkSourceFileLoaderError:
 * @CTK_SOURCE_FILE_LOADER_ERROR_TOO_BIG: The file is too big.
 * @CTK_SOURCE_FILE_LOADER_ERROR_ENCODING_AUTO_DETECTION_FAILED: It is not
 * possible to detect the encoding automatically.
 * @CTK_SOURCE_FILE_LOADER_ERROR_CONVERSION_FALLBACK: There was an encoding
 * conversion error and it was needed to use a fallback character.
 *
 * An error code used with the %CTK_SOURCE_FILE_LOADER_ERROR domain.
 */
typedef enum _GtkSourceFileLoaderError
{
	CTK_SOURCE_FILE_LOADER_ERROR_TOO_BIG,
	CTK_SOURCE_FILE_LOADER_ERROR_ENCODING_AUTO_DETECTION_FAILED,
	CTK_SOURCE_FILE_LOADER_ERROR_CONVERSION_FALLBACK
} GtkSourceFileLoaderError;

struct _GtkSourceFileLoader
{
	GObject parent;

	GtkSourceFileLoaderPrivate *priv;
};

struct _GtkSourceFileLoaderClass
{
	GObjectClass parent_class;

	gpointer padding[10];
};

CTK_SOURCE_AVAILABLE_IN_3_14
GType		 	 ctk_source_file_loader_get_type	(void) G_GNUC_CONST;

CTK_SOURCE_AVAILABLE_IN_3_14
GQuark			 ctk_source_file_loader_error_quark	(void);

CTK_SOURCE_AVAILABLE_IN_3_14
GtkSourceFileLoader	*ctk_source_file_loader_new		(GtkSourceBuffer         *buffer,
								 GtkSourceFile           *file);

CTK_SOURCE_AVAILABLE_IN_3_14
GtkSourceFileLoader	*ctk_source_file_loader_new_from_stream	(GtkSourceBuffer         *buffer,
								 GtkSourceFile           *file,
								 GInputStream            *stream);

CTK_SOURCE_AVAILABLE_IN_3_14
void			 ctk_source_file_loader_set_candidate_encodings
								(GtkSourceFileLoader     *loader,
								 GSList                  *candidate_encodings);

CTK_SOURCE_AVAILABLE_IN_3_14
GtkSourceBuffer		*ctk_source_file_loader_get_buffer	(GtkSourceFileLoader     *loader);

CTK_SOURCE_AVAILABLE_IN_3_14
GtkSourceFile		*ctk_source_file_loader_get_file	(GtkSourceFileLoader     *loader);

CTK_SOURCE_AVAILABLE_IN_3_14
GFile			*ctk_source_file_loader_get_location	(GtkSourceFileLoader     *loader);

CTK_SOURCE_AVAILABLE_IN_3_14
GInputStream		*ctk_source_file_loader_get_input_stream
								(GtkSourceFileLoader     *loader);

CTK_SOURCE_AVAILABLE_IN_3_14
void			 ctk_source_file_loader_load_async	(GtkSourceFileLoader     *loader,
								 gint                     io_priority,
								 GCancellable            *cancellable,
								 GFileProgressCallback    progress_callback,
								 gpointer                 progress_callback_data,
								 GDestroyNotify           progress_callback_notify,
								 GAsyncReadyCallback      callback,
								 gpointer                 user_data);

CTK_SOURCE_AVAILABLE_IN_3_14
gboolean		 ctk_source_file_loader_load_finish	(GtkSourceFileLoader     *loader,
								 GAsyncResult            *result,
								 GError                 **error);

CTK_SOURCE_AVAILABLE_IN_3_14
const GtkSourceEncoding	*ctk_source_file_loader_get_encoding	(GtkSourceFileLoader     *loader);

CTK_SOURCE_AVAILABLE_IN_3_14
GtkSourceNewlineType	 ctk_source_file_loader_get_newline_type (GtkSourceFileLoader    *loader);

CTK_SOURCE_AVAILABLE_IN_3_14
GtkSourceCompressionType ctk_source_file_loader_get_compression_type
								(GtkSourceFileLoader     *loader);

G_END_DECLS

#endif  /* CTK_SOURCE_FILE_LOADER_H  */
