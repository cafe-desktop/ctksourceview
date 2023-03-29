/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8; coding: utf-8 -*- */
/*
 * This file is part of GtkSourceView
 *
 * Copyright (C) 2007 - Johannes Schmid <jhs@gnome.org>
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "ctksourcemark.h"
#include "ctksourcebuffer.h"
#include "ctksourcebuffer-private.h"

/**
 * SECTION:mark
 * @Short_description: Mark object for GtkSourceBuffer
 * @Title: GtkSourceMark
 * @See_also: #GtkSourceBuffer
 *
 * A #GtkSourceMark marks a position in the text where you want to display
 * additional info. It is based on #GtkTextMark and thus is still valid after
 * the text has changed though its position may change.
 *
 * #GtkSourceMark<!-- -->s are organised in categories which you have to set
 * when you create the mark. Each category can have a priority, a pixbuf and
 * other associated attributes. See ctk_source_view_set_mark_attributes().
 * The pixbuf will be displayed in the margin at the line where the mark
 * residents if the #GtkSourceView:show-line-marks property is set to %TRUE. If
 * there are multiple marks in the same line, the pixbufs will be drawn on top
 * of each other. The mark with the highest priority will be drawn on top.
 */

enum
{
	PROP_0,
	PROP_CATEGORY
};

struct _GtkSourceMarkPrivate
{
	gchar *category;
};

G_DEFINE_TYPE_WITH_PRIVATE (GtkSourceMark, ctk_source_mark, GTK_TYPE_TEXT_MARK);

static void
ctk_source_mark_set_property (GObject      *object,
			      guint         prop_id,
			      const GValue *value,
			      GParamSpec   *pspec)
{
	GtkSourceMarkPrivate *priv;

	g_return_if_fail (GTK_SOURCE_IS_MARK (object));

	priv = GTK_SOURCE_MARK (object)->priv;

	switch (prop_id)
	{
		case PROP_CATEGORY:
			g_return_if_fail (g_value_get_string (value) != NULL);
			g_free (priv->category);
			priv->category = g_value_dup_string (value);
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object,
							   prop_id,
							   pspec);
	}
}

static void
ctk_source_mark_get_property (GObject    *object,
			      guint       prop_id,
			      GValue     *value,
			      GParamSpec *pspec)
{
	GtkSourceMark *mark;

	g_return_if_fail (GTK_SOURCE_IS_MARK (object));

	mark = GTK_SOURCE_MARK (object);

	switch (prop_id)
	{
		case PROP_CATEGORY:
			g_value_set_string (value,
					    ctk_source_mark_get_category (mark));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object,
							   prop_id,
							   pspec);
	}
}

static void
ctk_source_mark_finalize (GObject *object)
{
	GtkSourceMark *mark = GTK_SOURCE_MARK (object);

	g_free (mark->priv->category);

	G_OBJECT_CLASS (ctk_source_mark_parent_class)->finalize (object);
}

static void
ctk_source_mark_class_init (GtkSourceMarkClass *klass)
{
	GObjectClass *object_class;

	object_class = G_OBJECT_CLASS (klass);

	object_class->set_property = ctk_source_mark_set_property;
	object_class->get_property = ctk_source_mark_get_property;
	object_class->finalize = ctk_source_mark_finalize;

	/**
	 * GtkSourceMark:category:
	 *
	 * The category of the #GtkSourceMark, classifies the mark and controls
	 * which pixbuf is used and with which priority it is drawn.
	 */
	g_object_class_install_property (object_class,
					 PROP_CATEGORY,
					 g_param_spec_string ("category",
							      "Category",
							      "The mark category",
							      NULL,
							      G_PARAM_READWRITE |
							      G_PARAM_CONSTRUCT_ONLY |
							      G_PARAM_STATIC_STRINGS));
}

static void
ctk_source_mark_init (GtkSourceMark *mark)
{
	mark->priv = ctk_source_mark_get_instance_private (mark);
}

/**
 * ctk_source_mark_new:
 * @name: (nullable): Name of the #GtkSourceMark or %NULL
 * @category: is used to classify marks according to common characteristics
 *   (e.g. all the marks representing a bookmark could belong to the "bookmark"
 *   category, or all the marks representing a compilation error could belong
 *   to "error" category).
 *
 * Creates a text mark. Add it to a buffer using ctk_text_buffer_add_mark().
 * If name is NULL, the mark is anonymous; otherwise, the mark can be retrieved
 * by name using ctk_text_buffer_get_mark().
 * Normally marks are created using the utility function
 * ctk_source_buffer_create_source_mark().
 *
 * Returns: a new #GtkSourceMark that can be added using ctk_text_buffer_add_mark().
 *
 * Since: 2.2
 */
GtkSourceMark *
ctk_source_mark_new (const gchar *name,
                     const gchar *category)
{
	g_return_val_if_fail (category != NULL, NULL);

	return GTK_SOURCE_MARK (g_object_new (GTK_SOURCE_TYPE_MARK,
	                                      "category", category,
	                                      "name", name,
	                                      "left-gravity", TRUE,
	                                      NULL));
}

/**
 * ctk_source_mark_get_category:
 * @mark: a #GtkSourceMark.
 *
 * Returns the mark category.
 *
 * Returns: the category of the #GtkSourceMark.
 *
 * Since: 2.2
 */
const gchar *
ctk_source_mark_get_category (GtkSourceMark *mark)
{
	g_return_val_if_fail (GTK_SOURCE_IS_MARK (mark), NULL);

	return mark->priv->category;
}

/**
 * ctk_source_mark_next:
 * @mark: a #GtkSourceMark.
 * @category: (nullable): a string specifying the mark category, or %NULL.
 *
 * Returns the next #GtkSourceMark in the buffer or %NULL if the mark
 * was not added to a buffer. If there is no next mark, %NULL will be returned.
 *
 * If @category is %NULL, looks for marks of any category.
 *
 * Returns: (nullable) (transfer none): the next #GtkSourceMark, or %NULL.
 *
 * Since: 2.2
 */
GtkSourceMark *
ctk_source_mark_next (GtkSourceMark *mark,
		      const gchar   *category)
{
	GtkTextBuffer *buffer;

	g_return_val_if_fail (GTK_SOURCE_IS_MARK (mark), NULL);

	buffer = ctk_text_mark_get_buffer (GTK_TEXT_MARK (mark));

	if (buffer == NULL)
	{
		return NULL;
	}

	return _ctk_source_buffer_source_mark_next (GTK_SOURCE_BUFFER (buffer),
						    mark,
						    category);
}

/**
 * ctk_source_mark_prev:
 * @mark: a #GtkSourceMark.
 * @category: a string specifying the mark category, or %NULL.
 *
 * Returns the previous #GtkSourceMark in the buffer or %NULL if the mark
 * was not added to a buffer. If there is no previous mark, %NULL is returned.
 *
 * If @category is %NULL, looks for marks of any category
 *
 * Returns: (nullable) (transfer none): the previous #GtkSourceMark, or %NULL.
 *
 * Since: 2.2
 */
GtkSourceMark *
ctk_source_mark_prev (GtkSourceMark *mark,
		      const gchar   *category)
{
	GtkTextBuffer *buffer;

	g_return_val_if_fail (GTK_SOURCE_IS_MARK (mark), NULL);

	buffer = ctk_text_mark_get_buffer (GTK_TEXT_MARK (mark));

	if (buffer == NULL)
	{
		return NULL;
	}

	return _ctk_source_buffer_source_mark_prev (GTK_SOURCE_BUFFER (buffer),
						    mark,
						    category);
}

