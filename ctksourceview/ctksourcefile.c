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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "ctksourcefile.h"
#include "ctksourceencoding.h"
#include "ctksource-enumtypes.h"

/**
 * SECTION:file
 * @Short_description: On-disk representation of a CtkSourceBuffer
 * @Title: CtkSourceFile
 * @See_also: #CtkSourceFileLoader, #CtkSourceFileSaver
 *
 * A #CtkSourceFile object is the on-disk representation of a #CtkSourceBuffer.
 * With a #CtkSourceFile, you can create and configure a #CtkSourceFileLoader
 * and #CtkSourceFileSaver which take by default the values of the
 * #CtkSourceFile properties (except for the file loader which auto-detect some
 * properties). On a successful load or save operation, the #CtkSourceFile
 * properties are updated. If an operation fails, the #CtkSourceFile properties
 * have still the previous valid values.
 */

enum
{
	PROP_0,
	PROP_LOCATION,
	PROP_ENCODING,
	PROP_NEWLINE_TYPE,
	PROP_COMPRESSION_TYPE,
	PROP_READ_ONLY
};

struct _CtkSourceFilePrivate
{
	GFile *location;
	const CtkSourceEncoding *encoding;
	CtkSourceNewlineType newline_type;
	CtkSourceCompressionType compression_type;

	CtkSourceMountOperationFactory mount_operation_factory;
	gpointer mount_operation_userdata;
	GDestroyNotify mount_operation_notify;

	/* Last known modification time of 'location'. The value is updated on a
	 * file loading or file saving.
	 */
	gint64 modification_time;

	guint modification_time_set : 1;

	guint externally_modified : 1;
	guint deleted : 1;
	guint readonly : 1;
};

G_DEFINE_TYPE_WITH_PRIVATE (CtkSourceFile, ctk_source_file, G_TYPE_OBJECT)

static void
ctk_source_file_get_property (GObject    *object,
			      guint       prop_id,
			      GValue     *value,
			      GParamSpec *pspec)
{
	CtkSourceFile *file;

	g_return_if_fail (CTK_SOURCE_IS_FILE (object));

	file = CTK_SOURCE_FILE (object);

	switch (prop_id)
	{
		case PROP_LOCATION:
			g_value_set_object (value, file->priv->location);
			break;

		case PROP_ENCODING:
			g_value_set_boxed (value, file->priv->encoding);
			break;

		case PROP_NEWLINE_TYPE:
			g_value_set_enum (value, file->priv->newline_type);
			break;

		case PROP_COMPRESSION_TYPE:
			g_value_set_enum (value, file->priv->compression_type);
			break;

		case PROP_READ_ONLY:
			g_value_set_boolean (value, file->priv->readonly);
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
ctk_source_file_set_property (GObject      *object,
			      guint         prop_id,
			      const GValue *value,
			      GParamSpec   *pspec)
{
	CtkSourceFile *file;

	g_return_if_fail (CTK_SOURCE_IS_FILE (object));

	file = CTK_SOURCE_FILE (object);

	switch (prop_id)
	{
		case PROP_LOCATION:
			ctk_source_file_set_location (file, g_value_get_object (value));
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
ctk_source_file_dispose (GObject *object)
{
	CtkSourceFile *file = CTK_SOURCE_FILE (object);

	g_clear_object (&file->priv->location);

	if (file->priv->mount_operation_notify != NULL)
	{
		file->priv->mount_operation_notify (file->priv->mount_operation_userdata);
		file->priv->mount_operation_notify = NULL;
	}

	G_OBJECT_CLASS (ctk_source_file_parent_class)->dispose (object);
}

static void
ctk_source_file_class_init (CtkSourceFileClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->get_property = ctk_source_file_get_property;
	object_class->set_property = ctk_source_file_set_property;
	object_class->dispose = ctk_source_file_dispose;

	/**
	 * CtkSourceFile:location:
	 *
	 * The location.
	 *
	 * Since: 3.14
	 */
	g_object_class_install_property (object_class,
					 PROP_LOCATION,
					 g_param_spec_object ("location",
							      "Location",
							      "",
							      G_TYPE_FILE,
							      G_PARAM_READWRITE |
							      G_PARAM_CONSTRUCT |
							      G_PARAM_STATIC_STRINGS));

	/**
	 * CtkSourceFile:encoding:
	 *
	 * The character encoding, initially %NULL. After a successful file
	 * loading or saving operation, the encoding is non-%NULL.
	 *
	 * Since: 3.14
	 */
	g_object_class_install_property (object_class,
					 PROP_ENCODING,
					 g_param_spec_boxed ("encoding",
							     "Encoding",
							     "",
							     CTK_SOURCE_TYPE_ENCODING,
							     G_PARAM_READABLE |
							     G_PARAM_STATIC_STRINGS));

	/**
	 * CtkSourceFile:newline-type:
	 *
	 * The line ending type.
	 *
	 * Since: 3.14
	 */
	g_object_class_install_property (object_class,
					 PROP_NEWLINE_TYPE,
					 g_param_spec_enum ("newline-type",
							    "Newline type",
							    "",
							    CTK_SOURCE_TYPE_NEWLINE_TYPE,
							    CTK_SOURCE_NEWLINE_TYPE_LF,
							    G_PARAM_READABLE |
							    G_PARAM_STATIC_STRINGS));

	/**
	 * CtkSourceFile:compression-type:
	 *
	 * The compression type.
	 *
	 * Since: 3.14
	 */
	g_object_class_install_property (object_class,
					 PROP_COMPRESSION_TYPE,
					 g_param_spec_enum ("compression-type",
							    "Compression type",
							    "",
							    CTK_SOURCE_TYPE_COMPRESSION_TYPE,
							    CTK_SOURCE_COMPRESSION_TYPE_NONE,
							    G_PARAM_READABLE |
							    G_PARAM_STATIC_STRINGS));

	/**
	 * CtkSourceFile:read-only:
	 *
	 * Whether the file is read-only or not. The value of this property is
	 * not updated automatically (there is no file monitors).
	 *
	 * Since: 3.18
	 */
	g_object_class_install_property (object_class,
					 PROP_READ_ONLY,
					 g_param_spec_boolean ("read-only",
							       "Read Only",
							       "",
							       FALSE,
							       G_PARAM_READABLE |
							       G_PARAM_STATIC_STRINGS));
}

static void
ctk_source_file_init (CtkSourceFile *file)
{
	file->priv = ctk_source_file_get_instance_private (file);

	file->priv->encoding = NULL;
	file->priv->newline_type = CTK_SOURCE_NEWLINE_TYPE_LF;
	file->priv->compression_type = CTK_SOURCE_COMPRESSION_TYPE_NONE;
}

/**
 * ctk_source_file_new:
 *
 * Returns: a new #CtkSourceFile object.
 * Since: 3.14
 */
CtkSourceFile *
ctk_source_file_new (void)
{
	return g_object_new (CTK_SOURCE_TYPE_FILE, NULL);
}

/**
 * ctk_source_file_set_location:
 * @file: a #CtkSourceFile.
 * @location: (nullable): the new #GFile, or %NULL.
 *
 * Sets the location.
 *
 * Since: 3.14
 */
void
ctk_source_file_set_location (CtkSourceFile *file,
			      GFile         *location)
{
	g_return_if_fail (CTK_SOURCE_IS_FILE (file));
	g_return_if_fail (location == NULL || G_IS_FILE (location));

	if (g_set_object (&file->priv->location, location))
	{
		g_object_notify (G_OBJECT (file), "location");

		/* The modification_time is for the old location. */
		file->priv->modification_time_set = FALSE;

		file->priv->externally_modified = FALSE;
		file->priv->deleted = FALSE;
	}
}

/**
 * ctk_source_file_get_location:
 * @file: a #CtkSourceFile.
 *
 * Returns: (transfer none): the #GFile.
 * Since: 3.14
 */
GFile *
ctk_source_file_get_location (CtkSourceFile *file)
{
	g_return_val_if_fail (CTK_SOURCE_IS_FILE (file), NULL);

	return file->priv->location;
}

void
_ctk_source_file_set_encoding (CtkSourceFile           *file,
			       const CtkSourceEncoding *encoding)
{
	g_return_if_fail (CTK_SOURCE_IS_FILE (file));

	if (file->priv->encoding != encoding)
	{
		file->priv->encoding = encoding;
		g_object_notify (G_OBJECT (file), "encoding");
	}
}

/**
 * ctk_source_file_get_encoding:
 * @file: a #CtkSourceFile.
 *
 * The encoding is initially %NULL. After a successful file loading or saving
 * operation, the encoding is non-%NULL.
 *
 * Returns: the character encoding.
 * Since: 3.14
 */
const CtkSourceEncoding *
ctk_source_file_get_encoding (CtkSourceFile *file)
{
	g_return_val_if_fail (CTK_SOURCE_IS_FILE (file), NULL);

	return file->priv->encoding;
}

void
_ctk_source_file_set_newline_type (CtkSourceFile        *file,
				   CtkSourceNewlineType  newline_type)
{
	g_return_if_fail (CTK_SOURCE_IS_FILE (file));

	if (file->priv->newline_type != newline_type)
	{
		file->priv->newline_type = newline_type;
		g_object_notify (G_OBJECT (file), "newline-type");
	}
}

/**
 * ctk_source_file_get_newline_type:
 * @file: a #CtkSourceFile.
 *
 * Returns: the newline type.
 * Since: 3.14
 */
CtkSourceNewlineType
ctk_source_file_get_newline_type (CtkSourceFile *file)
{
	g_return_val_if_fail (CTK_SOURCE_IS_FILE (file), CTK_SOURCE_NEWLINE_TYPE_DEFAULT);

	return file->priv->newline_type;
}

void
_ctk_source_file_set_compression_type (CtkSourceFile            *file,
				       CtkSourceCompressionType  compression_type)
{
	g_return_if_fail (CTK_SOURCE_IS_FILE (file));

	if (file->priv->compression_type != compression_type)
	{
		file->priv->compression_type = compression_type;
		g_object_notify (G_OBJECT (file), "compression-type");
	}
}

/**
 * ctk_source_file_get_compression_type:
 * @file: a #CtkSourceFile.
 *
 * Returns: the compression type.
 * Since: 3.14
 */
CtkSourceCompressionType
ctk_source_file_get_compression_type (CtkSourceFile *file)
{
	g_return_val_if_fail (CTK_SOURCE_IS_FILE (file), CTK_SOURCE_COMPRESSION_TYPE_NONE);

	return file->priv->compression_type;
}

/**
 * ctk_source_file_set_mount_operation_factory:
 * @file: a #CtkSourceFile.
 * @callback: (scope notified): a #CtkSourceMountOperationFactory to call when a
 *   #GMountOperation is needed.
 * @user_data: (closure): the data to pass to the @callback function.
 * @notify: (nullable): function to call on @user_data when the @callback is no
 *   longer needed, or %NULL.
 *
 * Sets a #CtkSourceMountOperationFactory function that will be called when a
 * #GMountOperation must be created. This is useful for creating a
 * #CtkMountOperation with the parent #CtkWindow.
 *
 * If a mount operation factory isn't set, g_mount_operation_new() will be
 * called.
 *
 * Since: 3.14
 */
void
ctk_source_file_set_mount_operation_factory (CtkSourceFile                  *file,
					     CtkSourceMountOperationFactory  callback,
					     gpointer                        user_data,
					     GDestroyNotify                  notify)
{
	g_return_if_fail (CTK_SOURCE_IS_FILE (file));

	if (file->priv->mount_operation_notify != NULL)
	{
		file->priv->mount_operation_notify (file->priv->mount_operation_userdata);
	}

	file->priv->mount_operation_factory = callback;
	file->priv->mount_operation_userdata = user_data;
	file->priv->mount_operation_notify = notify;
}

GMountOperation *
_ctk_source_file_create_mount_operation (CtkSourceFile *file)
{
	return (file != NULL && file->priv->mount_operation_factory != NULL) ?
		file->priv->mount_operation_factory (file, file->priv->mount_operation_userdata) :
		g_mount_operation_new ();
}

gboolean
_ctk_source_file_get_modification_time (CtkSourceFile *file,
					gint64        *modification_time)
{
	g_assert (modification_time != NULL);

	if (file == NULL)
	{
		return FALSE;
	}

	g_return_val_if_fail (CTK_SOURCE_IS_FILE (file), FALSE);

	if (file->priv->modification_time_set)
	{
		*modification_time = file->priv->modification_time;
	}

	return file->priv->modification_time_set;
}

void
_ctk_source_file_set_modification_time (CtkSourceFile *file,
					gint64         modification_time)
{
	if (file != NULL)
	{
		g_return_if_fail (CTK_SOURCE_IS_FILE (file));

		file->priv->modification_time = modification_time;
		file->priv->modification_time_set = TRUE;
	}
}

/**
 * ctk_source_file_is_local:
 * @file: a #CtkSourceFile.
 *
 * Returns whether the file is local. If the #CtkSourceFile:location is %NULL,
 * returns %FALSE.
 *
 * Returns: whether the file is local.
 * Since: 3.18
 */
gboolean
ctk_source_file_is_local (CtkSourceFile *file)
{
	g_return_val_if_fail (CTK_SOURCE_IS_FILE (file), FALSE);

	if (file->priv->location == NULL)
	{
		return FALSE;
	}

	return g_file_has_uri_scheme (file->priv->location, "file");
}

/**
 * ctk_source_file_check_file_on_disk:
 * @file: a #CtkSourceFile.
 *
 * Checks synchronously the file on disk, to know whether the file is externally
 * modified, or has been deleted, and whether the file is read-only.
 *
 * #CtkSourceFile doesn't create a #GFileMonitor to track those properties, so
 * this function needs to be called instead. Creating lots of #GFileMonitor's
 * would take lots of resources.
 *
 * Since this function is synchronous, it is advised to call it only on local
 * files. See ctk_source_file_is_local().
 *
 * Since: 3.18
 */
void
ctk_source_file_check_file_on_disk (CtkSourceFile *file)
{
	GFileInfo *info;

	if (file->priv->location == NULL)
	{
		return;
	}

	info = g_file_query_info (file->priv->location,
				  G_FILE_ATTRIBUTE_TIME_MODIFIED ","
				  G_FILE_ATTRIBUTE_ACCESS_CAN_WRITE,
				  G_FILE_QUERY_INFO_NONE,
				  NULL,
				  NULL);

	if (info == NULL)
	{
		file->priv->deleted = TRUE;
		return;
	}

	if (g_file_info_has_attribute (info, G_FILE_ATTRIBUTE_TIME_MODIFIED) &&
	    file->priv->modification_time_set)
	{
		GDateTime *dt;
		gint64 mtime;

		dt = g_file_info_get_modification_date_time (info);
		mtime = g_date_time_to_unix (dt);

		/* Note that the modification time can even go backwards if the
		 * user is copying over an old file.
		 */
		if (mtime != file->priv->modification_time)
		{
			file->priv->externally_modified = TRUE;
		}

		g_date_time_unref (dt);
	}

	if (g_file_info_has_attribute (info, G_FILE_ATTRIBUTE_ACCESS_CAN_WRITE))
	{
		gboolean readonly;

		readonly = !g_file_info_get_attribute_boolean (info, G_FILE_ATTRIBUTE_ACCESS_CAN_WRITE);

		_ctk_source_file_set_readonly (file, readonly);
	}

	g_object_unref (info);
}

void
_ctk_source_file_set_externally_modified (CtkSourceFile *file,
					  gboolean       externally_modified)
{
	g_return_if_fail (CTK_SOURCE_IS_FILE (file));

	file->priv->externally_modified = externally_modified != FALSE;
}

/**
 * ctk_source_file_is_externally_modified:
 * @file: a #CtkSourceFile.
 *
 * Returns whether the file is externally modified. If the
 * #CtkSourceFile:location is %NULL, returns %FALSE.
 *
 * To have an up-to-date value, you must first call
 * ctk_source_file_check_file_on_disk().
 *
 * Returns: whether the file is externally modified.
 * Since: 3.18
 */
gboolean
ctk_source_file_is_externally_modified (CtkSourceFile *file)
{
	g_return_val_if_fail (CTK_SOURCE_IS_FILE (file), FALSE);

	return file->priv->externally_modified;
}

void
_ctk_source_file_set_deleted (CtkSourceFile *file,
			      gboolean       deleted)
{
	g_return_if_fail (CTK_SOURCE_IS_FILE (file));

	file->priv->deleted = deleted != FALSE;
}

/**
 * ctk_source_file_is_deleted:
 * @file: a #CtkSourceFile.
 *
 * Returns whether the file has been deleted. If the
 * #CtkSourceFile:location is %NULL, returns %FALSE.
 *
 * To have an up-to-date value, you must first call
 * ctk_source_file_check_file_on_disk().
 *
 * Returns: whether the file has been deleted.
 * Since: 3.18
 */
gboolean
ctk_source_file_is_deleted (CtkSourceFile *file)
{
	g_return_val_if_fail (CTK_SOURCE_IS_FILE (file), FALSE);

	return file->priv->deleted;
}

void
_ctk_source_file_set_readonly (CtkSourceFile *file,
			       gboolean       readonly)
{
	g_return_if_fail (CTK_SOURCE_IS_FILE (file));

	readonly = readonly != FALSE;

	if (file->priv->readonly != readonly)
	{
		file->priv->readonly = readonly;
		g_object_notify (G_OBJECT (file), "read-only");
	}
}

/**
 * ctk_source_file_is_readonly:
 * @file: a #CtkSourceFile.
 *
 * Returns whether the file is read-only. If the
 * #CtkSourceFile:location is %NULL, returns %FALSE.
 *
 * To have an up-to-date value, you must first call
 * ctk_source_file_check_file_on_disk().
 *
 * Returns: whether the file is read-only.
 * Since: 3.18
 */
gboolean
ctk_source_file_is_readonly (CtkSourceFile *file)
{
	g_return_val_if_fail (CTK_SOURCE_IS_FILE (file), FALSE);

	return file->priv->readonly;
}
