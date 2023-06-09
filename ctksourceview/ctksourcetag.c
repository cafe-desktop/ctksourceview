/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8; coding: utf-8 -*- */
/*
 * This file is part of CtkSourceView
 *
 * Copyright (C) 2015 - Université Catholique de Louvain
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
 *
 * Author: Sébastien Wilmet
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "ctksourcetag.h"

/**
 * SECTION:tag
 * @Short_description: A tag that can be applied to text in a CtkSourceBuffer
 * @Title: CtkSourceTag
 * @See_also: #CtkSourceBuffer
 *
 * #CtkSourceTag is a subclass of #CtkTextTag that adds properties useful for
 * the CtkSourceView library.
 *
 * If, for a certain tag, #CtkTextTag is sufficient, it's better that you create
 * a #CtkTextTag, not a #CtkSourceTag.
 */

typedef struct _CtkSourceTagPrivate CtkSourceTagPrivate;

struct _CtkSourceTagPrivate
{
	guint draw_spaces : 1;
	guint draw_spaces_set : 1;
};

enum
{
	PROP_0,
	PROP_DRAW_SPACES,
	PROP_DRAW_SPACES_SET,
};

G_DEFINE_TYPE_WITH_PRIVATE (CtkSourceTag, ctk_source_tag, CTK_TYPE_TEXT_TAG)

static void
ctk_source_tag_get_property (GObject    *object,
			     guint       prop_id,
			     GValue     *value,
			     GParamSpec *pspec)
{
	CtkSourceTagPrivate *priv;

	priv = ctk_source_tag_get_instance_private (CTK_SOURCE_TAG (object));

	switch (prop_id)
	{
		case PROP_DRAW_SPACES:
			g_value_set_boolean (value, priv->draw_spaces);
			break;

		case PROP_DRAW_SPACES_SET:
			g_value_set_boolean (value, priv->draw_spaces_set);
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
ctk_source_tag_set_property (GObject      *object,
			     guint         prop_id,
			     const GValue *value,
			     GParamSpec   *pspec)
{
	CtkSourceTag *tag;
	CtkSourceTagPrivate *priv;
	gboolean size_changed = FALSE;

	tag = CTK_SOURCE_TAG (object);
	priv = ctk_source_tag_get_instance_private (tag);

	switch (prop_id)
	{
		case PROP_DRAW_SPACES:
			priv->draw_spaces = g_value_get_boolean (value);
			priv->draw_spaces_set = TRUE;
			g_object_notify (object, "draw-spaces-set");
			break;

		case PROP_DRAW_SPACES_SET:
			priv->draw_spaces_set = g_value_get_boolean (value);
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}

	ctk_text_tag_changed (CTK_TEXT_TAG (tag), size_changed);
}

static void
ctk_source_tag_class_init (CtkSourceTagClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->get_property = ctk_source_tag_get_property;
	object_class->set_property = ctk_source_tag_set_property;

	/**
	 * CtkSourceTag:draw-spaces:
	 *
	 * Whether to draw white spaces. This property takes precedence over the value
	 * defined by the CtkSourceSpaceDrawer's #CtkSourceSpaceDrawer:matrix property
	 * (only where the tag is applied).
	 *
	 * Setting this property also changes #CtkSourceTag:draw-spaces-set to
	 * %TRUE.
	 *
	 * Since: 3.20
	 */
	g_object_class_install_property (object_class,
					 PROP_DRAW_SPACES,
					 g_param_spec_boolean ("draw-spaces",
							       "Draw Spaces",
							       "",
							       FALSE,
							       G_PARAM_READWRITE |
							       G_PARAM_STATIC_STRINGS));

	/**
	 * CtkSourceTag:draw-spaces-set:
	 *
	 * Whether the #CtkSourceTag:draw-spaces property is set and must be
	 * taken into account.
	 *
	 * Since: 3.20
	 */
	g_object_class_install_property (object_class,
					 PROP_DRAW_SPACES_SET,
					 g_param_spec_boolean ("draw-spaces-set",
							       "Draw Spaces Set",
							       "",
							       FALSE,
							       G_PARAM_READWRITE |
							       G_PARAM_STATIC_STRINGS));
}

static void
ctk_source_tag_init (CtkSourceTag *tag)
{
}

/**
 * ctk_source_tag_new:
 * @name: (nullable): tag name, or %NULL.
 *
 * Creates a #CtkSourceTag. Configure the tag using object arguments,
 * i.e. using g_object_set().
 *
 * For usual cases, ctk_source_buffer_create_source_tag() is more convenient to
 * use.
 *
 * Returns: a new #CtkSourceTag.
 * Since: 3.20
 */
CtkTextTag *
ctk_source_tag_new (const gchar *name)
{
	return g_object_new (CTK_SOURCE_TYPE_TAG,
			     "name", name,
			     NULL);
}
