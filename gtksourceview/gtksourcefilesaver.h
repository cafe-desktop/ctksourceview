/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8; coding: utf-8 -*- */
/*
 * This file is part of GtkSourceView
 *
 * Copyright (C) 2005, 2007 - Paolo Maggi
 * Copyrhing (C) 2007 - Steve Frécinaux
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

#ifndef CTK_SOURCE_FILE_SAVER_H
#define CTK_SOURCE_FILE_SAVER_H

#if !defined (CTK_SOURCE_H_INSIDE) && !defined (CTK_SOURCE_COMPILATION)
#error "Only <ctksourceview/ctksource.h> can be included directly."
#endif

#include <ctk/ctk.h>
#include <ctksourceview/ctksourcetypes.h>
#include <ctksourceview/ctksourcefile.h>

G_BEGIN_DECLS

#define CTK_SOURCE_TYPE_FILE_SAVER              (ctk_source_file_saver_get_type())
#define CTK_SOURCE_FILE_SAVER(obj)              (G_TYPE_CHECK_INSTANCE_CAST((obj), CTK_SOURCE_TYPE_FILE_SAVER, GtkSourceFileSaver))
#define CTK_SOURCE_FILE_SAVER_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass), CTK_SOURCE_TYPE_FILE_SAVER, GtkSourceFileSaverClass))
#define CTK_SOURCE_IS_FILE_SAVER(obj)           (G_TYPE_CHECK_INSTANCE_TYPE((obj), CTK_SOURCE_TYPE_FILE_SAVER))
#define CTK_SOURCE_IS_FILE_SAVER_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), CTK_SOURCE_TYPE_FILE_SAVER))
#define CTK_SOURCE_FILE_SAVER_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS((obj), CTK_SOURCE_TYPE_FILE_SAVER, GtkSourceFileSaverClass))

typedef struct _GtkSourceFileSaverClass   GtkSourceFileSaverClass;
typedef struct _GtkSourceFileSaverPrivate GtkSourceFileSaverPrivate;

#define CTK_SOURCE_FILE_SAVER_ERROR ctk_source_file_saver_error_quark ()

/**
 * GtkSourceFileSaverError:
 * @CTK_SOURCE_FILE_SAVER_ERROR_INVALID_CHARS: The buffer contains invalid
 *   characters.
 * @CTK_SOURCE_FILE_SAVER_ERROR_EXTERNALLY_MODIFIED: The file is externally
 *   modified.
 *
 * An error code used with the %CTK_SOURCE_FILE_SAVER_ERROR domain.
 * Since: 3.14
 */
typedef enum _GtkSourceFileSaverError
{
	CTK_SOURCE_FILE_SAVER_ERROR_INVALID_CHARS,
	CTK_SOURCE_FILE_SAVER_ERROR_EXTERNALLY_MODIFIED
} GtkSourceFileSaverError;

/**
 * GtkSourceFileSaverFlags:
 * @CTK_SOURCE_FILE_SAVER_FLAGS_NONE: No flags.
 * @CTK_SOURCE_FILE_SAVER_FLAGS_IGNORE_INVALID_CHARS: Ignore invalid characters.
 * @CTK_SOURCE_FILE_SAVER_FLAGS_IGNORE_MODIFICATION_TIME: Save file despite external modifications.
 * @CTK_SOURCE_FILE_SAVER_FLAGS_CREATE_BACKUP: Create a backup before saving the file.
 *
 * Flags to define the behavior of a #GtkSourceFileSaver.
 * Since: 3.14
 */
typedef enum _GtkSourceFileSaverFlags
{
	CTK_SOURCE_FILE_SAVER_FLAGS_NONE			= 0,
	CTK_SOURCE_FILE_SAVER_FLAGS_IGNORE_INVALID_CHARS	= 1 << 0,
	CTK_SOURCE_FILE_SAVER_FLAGS_IGNORE_MODIFICATION_TIME	= 1 << 1,
	CTK_SOURCE_FILE_SAVER_FLAGS_CREATE_BACKUP		= 1 << 2
} GtkSourceFileSaverFlags;

struct _GtkSourceFileSaver
{
	GObject object;

	GtkSourceFileSaverPrivate *priv;
};

struct _GtkSourceFileSaverClass
{
	GObjectClass parent_class;

	gpointer padding[10];
};

CTK_SOURCE_AVAILABLE_IN_3_14
GType			 ctk_source_file_saver_get_type		(void) G_GNUC_CONST;

CTK_SOURCE_AVAILABLE_IN_3_14
GQuark			 ctk_source_file_saver_error_quark	(void);

CTK_SOURCE_AVAILABLE_IN_3_14
GtkSourceFileSaver	*ctk_source_file_saver_new		(GtkSourceBuffer          *buffer,
								 GtkSourceFile            *file);

CTK_SOURCE_AVAILABLE_IN_3_14
GtkSourceFileSaver	*ctk_source_file_saver_new_with_target	(GtkSourceBuffer          *buffer,
								 GtkSourceFile            *file,
								 GFile                    *target_location);

CTK_SOURCE_AVAILABLE_IN_3_14
GtkSourceBuffer		*ctk_source_file_saver_get_buffer	(GtkSourceFileSaver       *saver);

CTK_SOURCE_AVAILABLE_IN_3_14
GtkSourceFile		*ctk_source_file_saver_get_file		(GtkSourceFileSaver       *saver);

CTK_SOURCE_AVAILABLE_IN_3_14
GFile			*ctk_source_file_saver_get_location	(GtkSourceFileSaver       *saver);

CTK_SOURCE_AVAILABLE_IN_3_14
void			 ctk_source_file_saver_set_encoding	(GtkSourceFileSaver       *saver,
								 const GtkSourceEncoding  *encoding);

CTK_SOURCE_AVAILABLE_IN_3_14
const GtkSourceEncoding *ctk_source_file_saver_get_encoding	(GtkSourceFileSaver       *saver);

CTK_SOURCE_AVAILABLE_IN_3_14
void			 ctk_source_file_saver_set_newline_type	(GtkSourceFileSaver       *saver,
								 GtkSourceNewlineType      newline_type);

CTK_SOURCE_AVAILABLE_IN_3_14
GtkSourceNewlineType	 ctk_source_file_saver_get_newline_type	(GtkSourceFileSaver       *saver);

CTK_SOURCE_AVAILABLE_IN_3_14
void			 ctk_source_file_saver_set_compression_type
								(GtkSourceFileSaver       *saver,
								 GtkSourceCompressionType  compression_type);

CTK_SOURCE_AVAILABLE_IN_3_14
GtkSourceCompressionType ctk_source_file_saver_get_compression_type
								(GtkSourceFileSaver       *saver);

CTK_SOURCE_AVAILABLE_IN_3_14
void			 ctk_source_file_saver_set_flags	(GtkSourceFileSaver       *saver,
								 GtkSourceFileSaverFlags   flags);

CTK_SOURCE_AVAILABLE_IN_3_14
GtkSourceFileSaverFlags	 ctk_source_file_saver_get_flags	(GtkSourceFileSaver       *saver);

CTK_SOURCE_AVAILABLE_IN_3_14
void			 ctk_source_file_saver_save_async	(GtkSourceFileSaver       *saver,
								 gint                      io_priority,
								 GCancellable             *cancellable,
								 GFileProgressCallback     progress_callback,
								 gpointer                  progress_callback_data,
								 GDestroyNotify            progress_callback_notify,
								 GAsyncReadyCallback       callback,
								 gpointer                  user_data);

CTK_SOURCE_AVAILABLE_IN_3_14
gboolean		 ctk_source_file_saver_save_finish	(GtkSourceFileSaver       *saver,
								 GAsyncResult             *result,
								 GError                  **error);

G_END_DECLS

#endif  /* CTK_SOURCE_FILE_SAVER_H  */
