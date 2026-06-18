/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8; coding: utf-8 -*-
 *
 * This file is part of CtkSourceView
 *
 * Copyright (C) 2010 - Jesse van den Kieboom
 * Copyright (C) 2010 - Krzesimir Nowak
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

#include "ctksourcemarkattributes.h"
#include "ctksourcemark.h"
#include "ctksource-marshal.h"
#include "ctksourcepixbufhelper.h"

/**
 * SECTION:markattributes
 * @short_description: The source mark attributes object
 * @title: CtkSourceMarkAttributes
 * @see_also: #CtkSourceMark
 *
 * #CtkSourceMarkAttributes is an object specifying attributes used by
 * a #CtkSourceView to visually show lines marked with #CtkSourceMark<!-- -->s
 * of a specific category. It allows you to define a background color of a line,
 * an icon shown in gutter and tooltips.
 *
 * The background color is used as a background of a line where a mark is placed
 * and it can be set with ctk_source_mark_attributes_set_background(). To check
 * if any custom background color was defined and what color it is, use
 * ctk_source_mark_attributes_get_background().
 *
 * An icon is a graphic element which is shown in the gutter of a view. An
 * example use is showing a red filled circle in a debugger to show that a
 * breakpoint was set in certain line. To get an icon that will be placed in
 * a gutter, first a base for it must be specified and then
 * ctk_source_mark_attributes_render_icon() must be called.
 * There are several ways to specify a base for an icon:
 * <itemizedlist>
 *  <listitem>
 *   <para>
 *    ctk_source_mark_attributes_set_icon_name()
 *   </para>
 *  </listitem>
 *  <listitem>
 *   <para>
 *    ctk_source_mark_attributes_set_gicon()
 *   </para>
 *  </listitem>
 *  <listitem>
 *   <para>
 *    ctk_source_mark_attributes_set_pixbuf()
 *   </para>
 *  </listitem>
 * </itemizedlist>
 * Using any of the above functions overrides the one used earlier. But note
 * that a getter counterpart of earlier used function can still return some
 * value, but it is just not used when rendering the proper icon.
 *
 * To provide meaningful tooltips for a given mark of a category, you should
 * connect to #CtkSourceMarkAttributes::query-tooltip-text or
 * #CtkSourceMarkAttributes::query-tooltip-markup where the latter
 * takes precedence.
 */

struct _CtkSourceMarkAttributesPrivate
{
	CdkRGBA background;

	CtkSourcePixbufHelper *helper;

	guint background_set : 1;
};

G_DEFINE_TYPE_WITH_PRIVATE (CtkSourceMarkAttributes, ctk_source_mark_attributes, G_TYPE_OBJECT)

enum
{
	PROP_0,
	PROP_BACKGROUND,
	PROP_PIXBUF,
	PROP_ICON_NAME,
	PROP_GICON
};

enum
{
	QUERY_TOOLTIP_TEXT,
	QUERY_TOOLTIP_MARKUP,
	N_SIGNALS
};

static guint signals[N_SIGNALS];

static void
ctk_source_mark_attributes_finalize (GObject *object)
{
	CtkSourceMarkAttributes *attributes = CTK_SOURCE_MARK_ATTRIBUTES (object);

	ctk_source_pixbuf_helper_free (attributes->priv->helper);

	G_OBJECT_CLASS (ctk_source_mark_attributes_parent_class)->finalize (object);
}

static void
set_background (CtkSourceMarkAttributes *attributes,
		const CdkRGBA           *color)
{
	if (color)
	{
		attributes->priv->background = *color;
	}

	attributes->priv->background_set = color != NULL;

	g_object_notify (G_OBJECT (attributes), "background");
}

static void
set_icon_name (CtkSourceMarkAttributes *attributes,
	       const gchar             *icon_name)
{
	if (g_strcmp0 (ctk_source_pixbuf_helper_get_icon_name (attributes->priv->helper),
	                                                       icon_name) == 0)
	{
		return;
	}

	ctk_source_pixbuf_helper_set_icon_name (attributes->priv->helper,
	                                        icon_name);

	g_object_notify (G_OBJECT (attributes), "icon-name");
}

static void
set_pixbuf (CtkSourceMarkAttributes *attributes,
	    const GdkPixbuf         *pixbuf)
{
	if (ctk_source_pixbuf_helper_get_pixbuf (attributes->priv->helper) == pixbuf)
	{
		return;
	}

	ctk_source_pixbuf_helper_set_pixbuf (attributes->priv->helper,
	                                     pixbuf);

	g_object_notify (G_OBJECT (attributes), "pixbuf");
}

static void
set_gicon (CtkSourceMarkAttributes *attributes,
	   GIcon                   *gicon)
{
	if (ctk_source_pixbuf_helper_get_gicon (attributes->priv->helper) == gicon)
	{
		return;
	}

	ctk_source_pixbuf_helper_set_gicon (attributes->priv->helper,
	                                    gicon);

	g_object_notify (G_OBJECT (attributes), "gicon");
}

static void
ctk_source_mark_attributes_set_property (GObject      *object,
					 guint         prop_id,
					 const GValue *value,
					 GParamSpec   *pspec)
{
	CtkSourceMarkAttributes *self = CTK_SOURCE_MARK_ATTRIBUTES (object);

	switch (prop_id)
	{
		case PROP_BACKGROUND:
			set_background (self, g_value_get_boxed (value));
			break;
		case PROP_PIXBUF:
			set_pixbuf (self, g_value_get_object (value));
			break;
		case PROP_ICON_NAME:
			set_icon_name (self, g_value_get_string (value));
			break;
		case PROP_GICON:
			set_gicon (self, g_value_get_object (value));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
ctk_source_mark_attributes_get_property (GObject    *object,
					 guint       prop_id,
					 GValue     *value,
					 GParamSpec *pspec)
{
	CtkSourceMarkAttributes *self = CTK_SOURCE_MARK_ATTRIBUTES (object);

	switch (prop_id)
	{
		case PROP_BACKGROUND:
			if (self->priv->background_set)
			{
				g_value_set_boxed (value, &self->priv->background);
			}
			else
			{
				g_value_set_boxed (value, NULL);
			}
			break;
		case PROP_PIXBUF:
			g_value_set_object (value,
			                    ctk_source_pixbuf_helper_get_pixbuf (self->priv->helper));
			break;
		case PROP_ICON_NAME:
			g_value_set_string (value,
			                    ctk_source_pixbuf_helper_get_icon_name (self->priv->helper));
			break;
		case PROP_GICON:
			g_value_set_object (value,
			                    ctk_source_pixbuf_helper_get_gicon (self->priv->helper));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
ctk_source_mark_attributes_class_init (CtkSourceMarkAttributesClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->finalize = ctk_source_mark_attributes_finalize;

	object_class->get_property = ctk_source_mark_attributes_get_property;
	object_class->set_property = ctk_source_mark_attributes_set_property;

	/**
	 * CtkSourceMarkAttributes:background:
	 *
	 * A color used for background of a line.
	 */
	g_object_class_install_property (object_class,
	                                 PROP_BACKGROUND,
	                                 g_param_spec_boxed ("background",
	                                                     "Background",
	                                                     "The background",
	                                                     CDK_TYPE_RGBA,
	                                                     G_PARAM_READWRITE |
							     G_PARAM_STATIC_STRINGS));

	/**
	 * CtkSourceMarkAttributes:pixbuf:
	 *
	 * A #GdkPixbuf that may be a base of a rendered icon.
	 */
	g_object_class_install_property (object_class,
	                                 PROP_PIXBUF,
	                                 g_param_spec_object ("pixbuf",
	                                                      "Pixbuf",
	                                                      "The pixbuf",
	                                                      CDK_TYPE_PIXBUF,
	                                                      G_PARAM_READWRITE |
							      G_PARAM_STATIC_STRINGS));

	/**
	 * CtkSourceMarkAttributes:icon-name:
	 *
	 * An icon name that may be a base of a rendered icon.
	 */
	g_object_class_install_property (object_class,
	                                 PROP_ICON_NAME,
	                                 g_param_spec_string ("icon-name",
	                                                      "Icon Name",
	                                                      "The icon name",
	                                                      NULL,
	                                                      G_PARAM_READWRITE |
							      G_PARAM_STATIC_STRINGS));

	/**
	 * CtkSourceMarkAttributes:gicon:
	 *
	 * A #GIcon that may be a base of a rendered icon.
	 */
	g_object_class_install_property (object_class,
	                                 PROP_GICON,
	                                 g_param_spec_object ("gicon",
	                                                      "GIcon",
	                                                      "The GIcon",
	                                                      G_TYPE_ICON,
	                                                      G_PARAM_READWRITE |
							      G_PARAM_STATIC_STRINGS));

	/**
	 * CtkSourceMarkAttributes::query-tooltip-text:
	 * @attributes: The #CtkSourceMarkAttributes which emits the signal.
	 * @mark: The #CtkSourceMark.
	 *
	 * The code should connect to this signal to provide a tooltip for given
	 * @mark. The tooltip should be just a plain text.
	 *
	 * Returns: (transfer full): A tooltip. The string should be freed with
	 * g_free() when done with it.
	 */
	signals[QUERY_TOOLTIP_TEXT] =
		g_signal_new ("query-tooltip-text",
		              G_TYPE_FROM_CLASS (klass),
		              G_SIGNAL_RUN_LAST,
		              0,
		              NULL, NULL,
		              _ctk_source_marshal_STRING__OBJECT,
		              G_TYPE_STRING,
		              1,
		              CTK_SOURCE_TYPE_MARK);
	g_signal_set_va_marshaller (signals[QUERY_TOOLTIP_TEXT],
	                            G_TYPE_FROM_CLASS (klass),
	                            _ctk_source_marshal_STRING__OBJECTv);

	/**
	 * CtkSourceMarkAttributes::query-tooltip-markup:
	 * @attributes: The #CtkSourceMarkAttributes which emits the signal.
	 * @mark: The #CtkSourceMark.
	 *
	 * The code should connect to this signal to provide a tooltip for given
	 * @mark. The tooltip can contain a markup.
	 *
	 * Returns: (transfer full): A tooltip. The string should be freed with
	 * g_free() when done with it.
	 */
	signals[QUERY_TOOLTIP_MARKUP] =
		g_signal_new ("query-tooltip-markup",
		              G_TYPE_FROM_CLASS (klass),
		              G_SIGNAL_RUN_LAST,
		              0,
		              NULL, NULL,
		              _ctk_source_marshal_STRING__OBJECT,
		              G_TYPE_STRING,
		              1,
		              CTK_SOURCE_TYPE_MARK);
	g_signal_set_va_marshaller (signals[QUERY_TOOLTIP_TEXT],
	                            G_TYPE_FROM_CLASS (klass),
	                            _ctk_source_marshal_STRING__OBJECTv);
}

static void
ctk_source_mark_attributes_init (CtkSourceMarkAttributes *self)
{
	self->priv = ctk_source_mark_attributes_get_instance_private (self);

	self->priv->helper = ctk_source_pixbuf_helper_new ();
}

/**
 * ctk_source_mark_attributes_new:
 *
 * Creates a new source mark attributes.
 *
 * Returns: (transfer full): a new source mark attributes.
 */
CtkSourceMarkAttributes *
ctk_source_mark_attributes_new (void)
{
	return g_object_new (CTK_SOURCE_TYPE_MARK_ATTRIBUTES, NULL);
}

/**
 * ctk_source_mark_attributes_set_background:
 * @attributes: a #CtkSourceMarkAttributes.
 * @background: a #CdkRGBA.
 *
 * Sets background color to the one given in @background.
 */
void
ctk_source_mark_attributes_set_background (CtkSourceMarkAttributes *attributes,
					   const CdkRGBA           *background)
{
	g_return_if_fail (CTK_SOURCE_IS_MARK_ATTRIBUTES (attributes));

	set_background (attributes, background);
}

/**
 * ctk_source_mark_attributes_get_background:
 * @attributes: a #CtkSourceMarkAttributes.
 * @background: (out caller-allocates): a #CdkRGBA.
 *
 * Stores background color in @background.
 *
 * Returns: whether background color for @attributes was set.
 */
gboolean
ctk_source_mark_attributes_get_background (CtkSourceMarkAttributes *attributes,
					   CdkRGBA                 *background)
{
	g_return_val_if_fail (CTK_SOURCE_IS_MARK_ATTRIBUTES (attributes), FALSE);

	if (background)
	{
		*background = attributes->priv->background;
	}
	return attributes->priv->background_set;
}

/**
 * ctk_source_mark_attributes_set_icon_name:
 * @attributes: a #CtkSourceMarkAttributes.
 * @icon_name: name of an icon to be used.
 *
 * Sets a name of an icon to be used as a base for rendered icon.
 */
void
ctk_source_mark_attributes_set_icon_name (CtkSourceMarkAttributes *attributes,
					  const gchar             *icon_name)
{
	g_return_if_fail (CTK_SOURCE_IS_MARK_ATTRIBUTES (attributes));

	set_icon_name (attributes, icon_name);
}

/**
 * ctk_source_mark_attributes_get_icon_name:
 * @attributes: a #CtkSourceMarkAttributes.
 *
 * Gets a name of an icon to be used as a base for rendered icon. Note that the
 * icon name can be %NULL if it wasn't set earlier.
 *
 * Returns: (transfer none): An icon name. The string belongs to @attributes and
 * should not be freed.
 */
const gchar *
ctk_source_mark_attributes_get_icon_name (CtkSourceMarkAttributes *attributes)
{
	g_return_val_if_fail (CTK_SOURCE_IS_MARK_ATTRIBUTES (attributes), NULL);

	return ctk_source_pixbuf_helper_get_icon_name (attributes->priv->helper);
}

/**
 * ctk_source_mark_attributes_set_gicon:
 * @attributes: a #CtkSourceMarkAttributes.
 * @gicon: a #GIcon to be used.
 *
 * Sets an icon to be used as a base for rendered icon.
 */
void
ctk_source_mark_attributes_set_gicon (CtkSourceMarkAttributes *attributes,
				      GIcon                   *gicon)
{
	g_return_if_fail (CTK_SOURCE_IS_MARK_ATTRIBUTES (attributes));

	set_gicon (attributes, gicon);
}

/**
 * ctk_source_mark_attributes_get_gicon:
 * @attributes: a #CtkSourceMarkAttributes.
 *
 * Gets a #GIcon to be used as a base for rendered icon. Note that the icon can
 * be %NULL if it wasn't set earlier.
 *
 * Returns: (transfer none): An icon. The icon belongs to @attributes and should
 * not be unreffed.
 */
GIcon *
ctk_source_mark_attributes_get_gicon (CtkSourceMarkAttributes *attributes)
{
	g_return_val_if_fail (CTK_SOURCE_IS_MARK_ATTRIBUTES (attributes), NULL);

	return ctk_source_pixbuf_helper_get_gicon (attributes->priv->helper);
}

/**
 * ctk_source_mark_attributes_set_pixbuf:
 * @attributes: a #CtkSourceMarkAttributes.
 * @pixbuf: a #GdkPixbuf to be used.
 *
 * Sets a pixbuf to be used as a base for rendered icon.
 */
void
ctk_source_mark_attributes_set_pixbuf (CtkSourceMarkAttributes *attributes,
				       const GdkPixbuf         *pixbuf)
{
	g_return_if_fail (CTK_SOURCE_IS_MARK_ATTRIBUTES (attributes));

	set_pixbuf (attributes, pixbuf);
}

/**
 * ctk_source_mark_attributes_get_pixbuf:
 * @attributes: a #CtkSourceMarkAttributes.
 *
 * Gets a #GdkPixbuf to be used as a base for rendered icon. Note that the
 * pixbuf can be %NULL if it wasn't set earlier.
 *
 * Returns: (transfer none): A pixbuf. The pixbuf belongs to @attributes and
 * should not be unreffed.
 */
const GdkPixbuf *
ctk_source_mark_attributes_get_pixbuf (CtkSourceMarkAttributes *attributes)
{
	g_return_val_if_fail (CTK_SOURCE_IS_MARK_ATTRIBUTES (attributes), NULL);

	return ctk_source_pixbuf_helper_get_pixbuf (attributes->priv->helper);
}

/**
 * ctk_source_mark_attributes_render_icon:
 * @attributes: a #CtkSourceMarkAttributes.
 * @widget: widget of which style settings may be used.
 * @size: size of the rendered icon.
 *
 * Renders an icon of given size. The base of the icon is set by the last call
 * to one of: ctk_source_mark_attributes_set_pixbuf(),
 * ctk_source_mark_attributes_set_gicon() or
 * ctk_source_mark_attributes_set_icon_name(). @size cannot be lower than 1.
 *
 * Returns: (transfer none): A rendered pixbuf. The pixbuf belongs to @attributes
 * and should not be unreffed.
 */
const GdkPixbuf *
ctk_source_mark_attributes_render_icon (CtkSourceMarkAttributes *attributes,
					CtkWidget               *widget,
					gint                     size)
{
	g_return_val_if_fail (CTK_SOURCE_IS_MARK_ATTRIBUTES (attributes), NULL);
	g_return_val_if_fail (CTK_IS_WIDGET (widget), NULL);
	g_return_val_if_fail (size > 0, NULL);

	return ctk_source_pixbuf_helper_render (attributes->priv->helper,
	                                        widget,
	                                        size);
}

/**
 * ctk_source_mark_attributes_get_tooltip_text:
 * @attributes: a #CtkSourceMarkAttributes.
 * @mark: a #CtkSourceMark.
 *
 * Queries for a tooltip by emitting
 * a #CtkSourceMarkAttributes::query-tooltip-text signal. The tooltip is a plain
 * text.
 *
 * Returns: (transfer full): A tooltip. The returned string should be freed by
 * using g_free() when done with it.
 */
gchar *
ctk_source_mark_attributes_get_tooltip_text (CtkSourceMarkAttributes *attributes,
					     CtkSourceMark           *mark)
{
	gchar *ret;

	g_return_val_if_fail (CTK_SOURCE_IS_MARK_ATTRIBUTES (attributes), NULL);
	g_return_val_if_fail (CTK_SOURCE_IS_MARK (mark), NULL);

	ret = NULL;
	g_signal_emit (attributes, signals[QUERY_TOOLTIP_TEXT], 0, mark, &ret);

	return ret;
}

/**
 * ctk_source_mark_attributes_get_tooltip_markup:
 * @attributes: a #CtkSourceMarkAttributes.
 * @mark: a #CtkSourceMark.
 *
 * Queries for a tooltip by emitting
 * a #CtkSourceMarkAttributes::query-tooltip-markup signal. The tooltip may contain
 * a markup.
 *
 * Returns: (transfer full): A tooltip. The returned string should be freed by
 * using g_free() when done with it.
 */
gchar *
ctk_source_mark_attributes_get_tooltip_markup (CtkSourceMarkAttributes *attributes,
					       CtkSourceMark           *mark)
{
	gchar *ret;

	g_return_val_if_fail (CTK_SOURCE_IS_MARK_ATTRIBUTES (attributes), NULL);
	g_return_val_if_fail (CTK_SOURCE_IS_MARK (mark), NULL);

	ret = NULL;
	g_signal_emit (attributes, signals[QUERY_TOOLTIP_MARKUP], 0, mark, &ret);

	return ret;
}

