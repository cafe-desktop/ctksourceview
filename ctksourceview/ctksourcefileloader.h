/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8; coding: utf-8 -*- */
/*
 * This file is part of CtkSourceView
 *
 * Copyright (C) 2005 - Paolo Maggi
 * Copyright (C) 2007 - Paolo Maggi, Steve Frécinaux
 * Copyright (C) 2008 - Jesse van den Kieboom
 * Copyright (C) 2014 - Sébastien Wilmet
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
#define CTK_SOURCE_FILE_LOADER(obj)              (G_TYPE_CHECK_INSTANCE_CAST((obj), CTK_SOURCE_TYPE_FILE_LOADER, CtkSourceFileLoader))
#define CTK_SOURCE_FILE_LOADER_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass), CTK_SOURCE_TYPE_FILE_LOADER, CtkSourceFileLoaderClass))
#define CTK_SOURCE_IS_FILE_LOADER(obj)           (G_TYPE_CHECK_INSTANCE_TYPE((obj), CTK_SOURCE_TYPE_FILE_LOADER))
#define CTK_SOURCE_IS_FILE_LOADER_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), CTK_SOURCE_TYPE_FILE_LOADER))
#define CTK_SOURCE_FILE_LOADER_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS((obj), CTK_SOURCE_TYPE_FILE_LOADER, CtkSourceFileLoaderClass))

typedef struct _CtkSourceFileLoaderClass   CtkSourceFileLoaderClass;
typedef struct _CtkSourceFileLoaderPrivate CtkSourceFileLoaderPrivate;

#define CTK_SOURCE_FILE_LOADER_ERROR ctk_source_file_loader_error_quark ()

/**
 * CtkSourceFileLoaderError:
 * @CTK_SOURCE_FILE_LOADER_ERROR_TOO_BIG: The file is too big.
 * @CTK_SOURCE_FILE_LOADER_ERROR_ENCODING_AUTO_DETECTION_FAILED: It is not
 * possible to detect the encoding automatically.
 * @CTK_SOURCE_FILE_LOADER_ERROR_CONVERSION_FALLBACK: There was an encoding
 * conversion error and it was needed to use a fallback character.
 *
 * An error code used with the %CTK_SOURCE_FILE_LOADER_ERROR domain.
 */
typedef enum _CtkSourceFileLoaderError
{
	CTK_SOURCE_FILE_LOADER_ERROR_TOO_BIG,
	CTK_SOURCE_FILE_LOADER_ERROR_ENCODING_AUTO_DETECTION_FAILED,
	CTK_SOURCE_FILE_LOADER_ERROR_CONVERSION_FALLBACK
} CtkSourceFileLoaderError;

struct _CtkSourceFileLoader
{
	GObject parent;

	CtkSourceFileLoaderPrivate *priv;
};

struct _CtkSourceFileLoaderClass
{
	GObjectClass parent_class;

	gpointer padding[10];
};

CTK_SOURCE_AVAILABLE_IN_3_14
GType		 	 ctk_source_file_loader_get_type	(void) G_GNUC_CONST;

CTK_SOURCE_AVAILABLE_IN_3_14
GQuark			 ctk_source_file_loader_error_quark	(void);

CTK_SOURCE_AVAILABLE_IN_3_14
CtkSourceFileLoader	*ctk_source_file_loader_new		(CtkSourceBuffer         *buffer,
								 CtkSourceFile           *file);

CTK_SOURCE_AVAILABLE_IN_3_14
CtkSourceFileLoader	*ctk_source_file_loader_new_from_stream	(CtkSourceBuffer         *buffer,
								 CtkSourceFile           *file,
								 GInputStream            *stream);

CTK_SOURCE_AVAILABLE_IN_3_14
void			 ctk_source_file_loader_set_candidate_encodings
								(CtkSourceFileLoader     *loader,
								 GSList                  *candidate_encodings);

CTK_SOURCE_AVAILABLE_IN_3_14
CtkSourceBuffer		*ctk_source_file_loader_get_buffer	(CtkSourceFileLoader     *loader);

CTK_SOURCE_AVAILABLE_IN_3_14
CtkSourceFile		*ctk_source_file_loader_get_file	(CtkSourceFileLoader     *loader);

CTK_SOURCE_AVAILABLE_IN_3_14
GFile			*ctk_source_file_loader_get_location	(CtkSourceFileLoader     *loader);

CTK_SOURCE_AVAILABLE_IN_3_14
GInputStream		*ctk_source_file_loader_get_input_stream
								(CtkSourceFileLoader     *loader);

CTK_SOURCE_AVAILABLE_IN_3_14
void			 ctk_source_file_loader_load_async	(CtkSourceFileLoader     *loader,
								 gint                     io_priority,
								 GCancellable            *cancellable,
								 GFileProgressCallback    progress_callback,
								 gpointer                 progress_callback_data,
								 GDestroyNotify           progress_callback_notify,
								 GAsyncReadyCallback      callback,
								 gpointer                 user_data);

CTK_SOURCE_AVAILABLE_IN_3_14
gboolean		 ctk_source_file_loader_load_finish	(CtkSourceFileLoader     *loader,
								 GAsyncResult            *result,
								 GError                 **error);

CTK_SOURCE_AVAILABLE_IN_3_14
const CtkSourceEncoding	*ctk_source_file_loader_get_encoding	(CtkSourceFileLoader     *loader);

CTK_SOURCE_AVAILABLE_IN_3_14
CtkSourceNewlineType	 ctk_source_file_loader_get_newline_type (CtkSourceFileLoader    *loader);

CTK_SOURCE_AVAILABLE_IN_3_14
CtkSourceCompressionType ctk_source_file_loader_get_compression_type
								(CtkSourceFileLoader     *loader);

G_END_DECLS

#endif  /* CTK_SOURCE_FILE_LOADER_H  */
