/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8; coding: utf-8 -*- */
/*
 * This file is part of CtkSourceView
 *
 * Copyright (C) 2014, 2015 - Sébastien Wilmet <swilmet@gnome.org>
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

#ifndef CTK_SOURCE_FILE_H
#define CTK_SOURCE_FILE_H

#if !defined (CTK_SOURCE_H_INSIDE) && !defined (CTK_SOURCE_COMPILATION)
#error "Only <ctksourceview/ctksource.h> can be included directly."
#endif

#include <ctk/ctk.h>
#include <ctksourceview/ctksourcetypes.h>

G_BEGIN_DECLS

#define CTK_SOURCE_TYPE_FILE             (ctk_source_file_get_type ())
#define CTK_SOURCE_FILE(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), CTK_SOURCE_TYPE_FILE, CtkSourceFile))
#define CTK_SOURCE_FILE_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), CTK_SOURCE_TYPE_FILE, CtkSourceFileClass))
#define CTK_SOURCE_IS_FILE(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), CTK_SOURCE_TYPE_FILE))
#define CTK_SOURCE_IS_FILE_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), CTK_SOURCE_TYPE_FILE))
#define CTK_SOURCE_FILE_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), CTK_SOURCE_TYPE_FILE, CtkSourceFileClass))

typedef struct _CtkSourceFileClass    CtkSourceFileClass;
typedef struct _CtkSourceFilePrivate  CtkSourceFilePrivate;

/**
 * CtkSourceNewlineType:
 * @CTK_SOURCE_NEWLINE_TYPE_LF: line feed, used on UNIX.
 * @CTK_SOURCE_NEWLINE_TYPE_CR: carriage return, used on Mac.
 * @CTK_SOURCE_NEWLINE_TYPE_CR_LF: carriage return followed by a line feed, used
 *   on Windows.
 *
 * Since: 3.14
 */
typedef enum _CtkSourceNewlineType
{
	CTK_SOURCE_NEWLINE_TYPE_LF,
	CTK_SOURCE_NEWLINE_TYPE_CR,
	CTK_SOURCE_NEWLINE_TYPE_CR_LF
} CtkSourceNewlineType;

/**
 * CTK_SOURCE_NEWLINE_TYPE_DEFAULT:
 *
 * The default newline type on the current OS.
 *
 * Since: 3.14
 */
#ifdef G_OS_WIN32
#define CTK_SOURCE_NEWLINE_TYPE_DEFAULT CTK_SOURCE_NEWLINE_TYPE_CR_LF
#else
#define CTK_SOURCE_NEWLINE_TYPE_DEFAULT CTK_SOURCE_NEWLINE_TYPE_LF
#endif

/**
 * CtkSourceCompressionType:
 * @CTK_SOURCE_COMPRESSION_TYPE_NONE: plain text.
 * @CTK_SOURCE_COMPRESSION_TYPE_GZIP: gzip compression.
 *
 * Since: 3.14
 */
typedef enum _CtkSourceCompressionType
{
	CTK_SOURCE_COMPRESSION_TYPE_NONE,
	CTK_SOURCE_COMPRESSION_TYPE_GZIP
} CtkSourceCompressionType;

/**
 * CtkSourceMountOperationFactory:
 * @file: a #CtkSourceFile.
 * @userdata: user data
 *
 * Type definition for a function that will be called to create a
 * #GMountOperation. This is useful for creating a #CtkMountOperation.
 *
 * Since: 3.14
 */
typedef GMountOperation *(*CtkSourceMountOperationFactory) (CtkSourceFile *file,
							    gpointer       userdata);

struct _CtkSourceFile
{
	GObject parent;

	CtkSourceFilePrivate *priv;
};

struct _CtkSourceFileClass
{
	GObjectClass parent_class;

	gpointer padding[10];
};

CTK_SOURCE_AVAILABLE_IN_3_14
GType		 ctk_source_file_get_type			(void) G_GNUC_CONST;

CTK_SOURCE_AVAILABLE_IN_3_14
CtkSourceFile	*ctk_source_file_new				(void);

CTK_SOURCE_AVAILABLE_IN_3_14
GFile		*ctk_source_file_get_location			(CtkSourceFile *file);

CTK_SOURCE_AVAILABLE_IN_3_14
void		 ctk_source_file_set_location			(CtkSourceFile *file,
								 GFile         *location);

CTK_SOURCE_AVAILABLE_IN_3_14
const CtkSourceEncoding *
		 ctk_source_file_get_encoding			(CtkSourceFile *file);

CTK_SOURCE_AVAILABLE_IN_3_14
CtkSourceNewlineType
		 ctk_source_file_get_newline_type		(CtkSourceFile *file);

CTK_SOURCE_AVAILABLE_IN_3_14
CtkSourceCompressionType
		 ctk_source_file_get_compression_type		(CtkSourceFile *file);

CTK_SOURCE_AVAILABLE_IN_3_14
void		 ctk_source_file_set_mount_operation_factory	(CtkSourceFile                  *file,
								 CtkSourceMountOperationFactory  callback,
								 gpointer                        user_data,
								 GDestroyNotify                  notify);

CTK_SOURCE_AVAILABLE_IN_3_18
void		 ctk_source_file_check_file_on_disk		(CtkSourceFile *file);

CTK_SOURCE_AVAILABLE_IN_3_18
gboolean	 ctk_source_file_is_local			(CtkSourceFile *file);

CTK_SOURCE_AVAILABLE_IN_3_18
gboolean	 ctk_source_file_is_externally_modified		(CtkSourceFile *file);

CTK_SOURCE_AVAILABLE_IN_3_18
gboolean	 ctk_source_file_is_deleted			(CtkSourceFile *file);

CTK_SOURCE_AVAILABLE_IN_3_18
gboolean	 ctk_source_file_is_readonly			(CtkSourceFile *file);

G_GNUC_INTERNAL
void		 _ctk_source_file_set_encoding			(CtkSourceFile           *file,
								 const CtkSourceEncoding *encoding);

G_GNUC_INTERNAL
void		 _ctk_source_file_set_newline_type		(CtkSourceFile        *file,
								 CtkSourceNewlineType  newline_type);

G_GNUC_INTERNAL
void		 _ctk_source_file_set_compression_type		(CtkSourceFile            *file,
								 CtkSourceCompressionType  compression_type);

G_GNUC_INTERNAL
GMountOperation	*_ctk_source_file_create_mount_operation	(CtkSourceFile *file);

G_GNUC_INTERNAL
gboolean	 _ctk_source_file_get_modification_time		(CtkSourceFile *file,
								 gint64        *modification_time);

G_GNUC_INTERNAL
void		 _ctk_source_file_set_modification_time		(CtkSourceFile *file,
								 gint64         modification_time);

G_GNUC_INTERNAL
void		 _ctk_source_file_set_externally_modified	(CtkSourceFile *file,
								 gboolean       externally_modified);

G_GNUC_INTERNAL
void		 _ctk_source_file_set_deleted			(CtkSourceFile *file,
								 gboolean       deleted);

G_GNUC_INTERNAL
void		 _ctk_source_file_set_readonly			(CtkSourceFile *file,
								 gboolean       readonly);

G_END_DECLS

#endif /* CTK_SOURCE_FILE_H */
