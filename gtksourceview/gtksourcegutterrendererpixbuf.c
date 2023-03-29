/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8; coding: utf-8 -*-
 *
 * This file is part of CtkSourceView
 *
 * Copyright (C) 2010 - Jesse van den Kieboom
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

#include "ctksourcegutterrendererpixbuf.h"
#include "ctksourcepixbufhelper.h"

/**
 * SECTION:gutterrendererpixbuf
 * @Short_description: Renders a pixbuf in the gutter
 * @Title: CtkSourceGutterRendererPixbuf
 * @See_also: #CtkSourceGutterRenderer, #CtkSourceGutter
 *
 * A #CtkSourceGutterRendererPixbuf can be used to render an image in a cell of
 * #CtkSourceGutter.
 */

struct _CtkSourceGutterRendererPixbufPrivate
{
	CtkSourcePixbufHelper *helper;
};

G_DEFINE_TYPE_WITH_PRIVATE (CtkSourceGutterRendererPixbuf, ctk_source_gutter_renderer_pixbuf, CTK_SOURCE_TYPE_GUTTER_RENDERER)

enum
{
	PROP_0,
	PROP_PIXBUF,
	PROP_ICON_NAME,
	PROP_GICON,
};

static void
center_on (CtkSourceGutterRenderer *renderer,
           GdkRectangle            *cell_area,
           CtkTextIter             *iter,
           gint                     width,
           gint                     height,
           gfloat                   xalign,
           gfloat                   yalign,
           gint                    *x,
           gint                    *y)
{
	CtkTextView *view;
	CtkTextWindowType window_type;
	GdkRectangle buffer_location;
	gint window_y;

	view = ctk_source_gutter_renderer_get_view (renderer);
	window_type = ctk_source_gutter_renderer_get_window_type (renderer);

	ctk_text_view_get_iter_location (view, iter, &buffer_location);

	ctk_text_view_buffer_to_window_coords (view,
					       window_type,
					       0, buffer_location.y,
					       NULL, &window_y);

	*x = cell_area->x + (cell_area->width - width) * xalign;
	*y = window_y + (buffer_location.height - height) * yalign;
}

static void
gutter_renderer_pixbuf_draw (CtkSourceGutterRenderer      *renderer,
                             cairo_t                      *cr,
                             GdkRectangle                 *background_area,
                             GdkRectangle                 *cell_area,
                             CtkTextIter                  *start,
                             CtkTextIter                  *end,
                             CtkSourceGutterRendererState  state)
{
	CtkSourceGutterRendererPixbuf *pix = CTK_SOURCE_GUTTER_RENDERER_PIXBUF (renderer);
	gint width;
	gint height;
	gfloat xalign;
	gfloat yalign;
	CtkSourceGutterRendererAlignmentMode mode;
	CtkTextView *view;
	gint scale;
	gint x = 0;
	gint y = 0;
	GdkPixbuf *pixbuf;
	cairo_surface_t *surface;

	/* Chain up to draw background */
	if (CTK_SOURCE_GUTTER_RENDERER_CLASS (ctk_source_gutter_renderer_pixbuf_parent_class)->draw != NULL)
	{
		CTK_SOURCE_GUTTER_RENDERER_CLASS (ctk_source_gutter_renderer_pixbuf_parent_class)->draw (renderer,
													 cr,
													 background_area,
													 cell_area,
													 start,
													 end,
													 state);
	}

	view = ctk_source_gutter_renderer_get_view (renderer);

	pixbuf = ctk_source_pixbuf_helper_render (pix->priv->helper,
	                                          CTK_WIDGET (view),
	                                          cell_area->width);

	if (!pixbuf)
	{
		return;
	}

	width = gdk_pixbuf_get_width (pixbuf);
	height = gdk_pixbuf_get_height (pixbuf);

	/*
	 * We might have gotten a pixbuf back from the helper that will allow
	 * us to render for HiDPI. If we detect this, we pretend that we got a
	 * different size back and then gdk_cairo_surface_create_from_pixbuf()
	 * will take care of the rest.
	 */
	scale = ctk_widget_get_scale_factor (CTK_WIDGET (view));
	if ((scale > 1) &&
	    ((width > cell_area->width) || (height > cell_area->height)) &&
	    (width <= (cell_area->width * scale)) &&
	    (height <= (cell_area->height * scale)))
	{
		width = width / scale;
		height = height / scale;
	}

	ctk_source_gutter_renderer_get_alignment (renderer,
	                                          &xalign,
	                                          &yalign);

	mode = ctk_source_gutter_renderer_get_alignment_mode (renderer);

	switch (mode)
	{
		case CTK_SOURCE_GUTTER_RENDERER_ALIGNMENT_MODE_CELL:
			x = cell_area->x + (cell_area->width - width) * xalign;
			y = cell_area->y + (cell_area->height - height) * yalign;
			break;
		case CTK_SOURCE_GUTTER_RENDERER_ALIGNMENT_MODE_FIRST:
			center_on (renderer,
			           cell_area,
			           start,
			           width,
			           height,
			           xalign,
			           yalign,
			           &x,
			           &y);
			break;
		case CTK_SOURCE_GUTTER_RENDERER_ALIGNMENT_MODE_LAST:
			center_on (renderer,
			           cell_area,
			           end,
			           width,
			           height,
			           xalign,
			           yalign,
			           &x,
			           &y);
			break;
		default:
			g_assert_not_reached ();
	}

	surface = gdk_cairo_surface_create_from_pixbuf (pixbuf, scale, NULL);
	cairo_set_source_surface (cr, surface, x, y);

	cairo_paint (cr);

	cairo_surface_destroy (surface);
}

static void
ctk_source_gutter_renderer_pixbuf_finalize (GObject *object)
{
	CtkSourceGutterRendererPixbuf *renderer = CTK_SOURCE_GUTTER_RENDERER_PIXBUF (object);

	ctk_source_pixbuf_helper_free (renderer->priv->helper);

	G_OBJECT_CLASS (ctk_source_gutter_renderer_pixbuf_parent_class)->finalize (object);
}

static void
set_pixbuf (CtkSourceGutterRendererPixbuf *renderer,
            GdkPixbuf                     *pixbuf)
{
	ctk_source_pixbuf_helper_set_pixbuf (renderer->priv->helper,
	                                     pixbuf);

	g_object_notify (G_OBJECT (renderer), "pixbuf");

	ctk_source_gutter_renderer_queue_draw (CTK_SOURCE_GUTTER_RENDERER (renderer));
}

static void
set_gicon (CtkSourceGutterRendererPixbuf *renderer,
           GIcon                         *icon)
{
	ctk_source_pixbuf_helper_set_gicon (renderer->priv->helper,
	                                    icon);

	g_object_notify (G_OBJECT (renderer), "gicon");

	ctk_source_gutter_renderer_queue_draw (CTK_SOURCE_GUTTER_RENDERER (renderer));
}

static void
set_icon_name (CtkSourceGutterRendererPixbuf *renderer,
               const gchar                   *icon_name)
{
	ctk_source_pixbuf_helper_set_icon_name (renderer->priv->helper,
	                                        icon_name);

	g_object_notify (G_OBJECT (renderer), "icon-name");

	ctk_source_gutter_renderer_queue_draw (CTK_SOURCE_GUTTER_RENDERER (renderer));
}


static void
ctk_source_gutter_renderer_pixbuf_set_property (GObject      *object,
                                                guint         prop_id,
                                                const GValue *value,
                                                GParamSpec   *pspec)
{
	CtkSourceGutterRendererPixbuf *renderer;

	renderer = CTK_SOURCE_GUTTER_RENDERER_PIXBUF (object);

	switch (prop_id)
	{
		case PROP_PIXBUF:
			set_pixbuf (renderer, g_value_get_object (value));
			break;
		case PROP_ICON_NAME:
			set_icon_name (renderer, g_value_get_string (value));
			break;
		case PROP_GICON:
			set_gicon (renderer, g_value_get_object (value));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
ctk_source_gutter_renderer_pixbuf_get_property (GObject    *object,
                                                guint       prop_id,
                                                GValue     *value,
                                                GParamSpec *pspec)
{
	CtkSourceGutterRendererPixbuf *renderer;

	renderer = CTK_SOURCE_GUTTER_RENDERER_PIXBUF (object);

	switch (prop_id)
	{
		case PROP_PIXBUF:
			g_value_set_object (value,
			                    ctk_source_pixbuf_helper_get_pixbuf (renderer->priv->helper));
			break;
		case PROP_ICON_NAME:
			g_value_set_string (value,
			                    ctk_source_pixbuf_helper_get_icon_name (renderer->priv->helper));
			break;
		case PROP_GICON:
			g_value_set_object (value,
			                    ctk_source_pixbuf_helper_get_gicon (renderer->priv->helper));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
ctk_source_gutter_renderer_pixbuf_class_init (CtkSourceGutterRendererPixbufClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	CtkSourceGutterRendererClass *renderer_class = CTK_SOURCE_GUTTER_RENDERER_CLASS (klass);

	object_class->finalize = ctk_source_gutter_renderer_pixbuf_finalize;

	object_class->get_property = ctk_source_gutter_renderer_pixbuf_get_property;
	object_class->set_property = ctk_source_gutter_renderer_pixbuf_set_property;

	renderer_class->draw = gutter_renderer_pixbuf_draw;

	g_object_class_install_property (object_class,
	                                 PROP_PIXBUF,
	                                 g_param_spec_object ("pixbuf",
	                                                      "Pixbuf",
	                                                      "The pixbuf",
	                                                      GDK_TYPE_PIXBUF,
	                                                      G_PARAM_READWRITE));

	g_object_class_install_property (object_class,
	                                 PROP_ICON_NAME,
	                                 g_param_spec_string ("icon-name",
	                                                      "Icon Name",
	                                                      "The icon name",
	                                                      NULL,
	                                                      G_PARAM_READWRITE));

	g_object_class_install_property (object_class,
	                                 PROP_GICON,
	                                 g_param_spec_object ("gicon",
	                                                      "GIcon",
	                                                      "The gicon",
	                                                      G_TYPE_ICON,
	                                                      G_PARAM_READWRITE));
}

static void
ctk_source_gutter_renderer_pixbuf_init (CtkSourceGutterRendererPixbuf *self)
{
	self->priv = ctk_source_gutter_renderer_pixbuf_get_instance_private (self);

	self->priv->helper = ctk_source_pixbuf_helper_new ();
}

/**
 * ctk_source_gutter_renderer_pixbuf_new:
 *
 * Create a new #CtkSourceGutterRendererPixbuf.
 *
 * Returns: (transfer full): A #CtkSourceGutterRenderer
 *
 **/
CtkSourceGutterRenderer *
ctk_source_gutter_renderer_pixbuf_new (void)
{
	return g_object_new (CTK_SOURCE_TYPE_GUTTER_RENDERER_PIXBUF, NULL);
}

/**
 * ctk_source_gutter_renderer_pixbuf_set_pixbuf:
 * @renderer: a #CtkSourceGutterRendererPixbuf
 * @pixbuf: (nullable): the pixbuf, or %NULL.
 */
void
ctk_source_gutter_renderer_pixbuf_set_pixbuf (CtkSourceGutterRendererPixbuf *renderer,
                                              GdkPixbuf                     *pixbuf)
{
	g_return_if_fail (CTK_SOURCE_IS_GUTTER_RENDERER_PIXBUF (renderer));
	g_return_if_fail (renderer == NULL || GDK_IS_PIXBUF (pixbuf));

	set_pixbuf (renderer, pixbuf);
}


/**
 * ctk_source_gutter_renderer_pixbuf_get_pixbuf:
 * @renderer: a #CtkSourceGutterRendererPixbuf
 *
 * Get the pixbuf of the renderer.
 *
 * Returns: (transfer none): a #GdkPixbuf
 *
 **/
GdkPixbuf *
ctk_source_gutter_renderer_pixbuf_get_pixbuf (CtkSourceGutterRendererPixbuf *renderer)
{
	g_return_val_if_fail (CTK_SOURCE_IS_GUTTER_RENDERER_PIXBUF (renderer), NULL);

	return ctk_source_pixbuf_helper_get_pixbuf (renderer->priv->helper);
}

/**
 * ctk_source_gutter_renderer_pixbuf_set_gicon:
 * @renderer: a #CtkSourceGutterRendererPixbuf
 * @icon: (nullable): the icon, or %NULL.
 */
void
ctk_source_gutter_renderer_pixbuf_set_gicon (CtkSourceGutterRendererPixbuf *renderer,
                                             GIcon                         *icon)
{
	g_return_if_fail (CTK_SOURCE_IS_GUTTER_RENDERER_PIXBUF (renderer));
	g_return_if_fail (icon == NULL || G_IS_ICON (icon));

	set_gicon (renderer, icon);
}

/**
 * ctk_source_gutter_renderer_pixbuf_get_gicon:
 * @renderer: a #CtkSourceGutterRendererPixbuf
 *
 * Get the gicon of the renderer
 *
 * Returns: (transfer none): a #GIcon
 *
 **/
GIcon *
ctk_source_gutter_renderer_pixbuf_get_gicon (CtkSourceGutterRendererPixbuf *renderer)
{
	g_return_val_if_fail (CTK_SOURCE_IS_GUTTER_RENDERER_PIXBUF (renderer), NULL);

	return ctk_source_pixbuf_helper_get_gicon (renderer->priv->helper);
}

/**
 * ctk_source_gutter_renderer_pixbuf_set_icon_name:
 * @renderer: a #CtkSourceGutterRendererPixbuf
 * @icon_name: (nullable): the icon name, or %NULL.
 */
void
ctk_source_gutter_renderer_pixbuf_set_icon_name (CtkSourceGutterRendererPixbuf *renderer,
                                                 const gchar                   *icon_name)
{
	g_return_if_fail (CTK_SOURCE_IS_GUTTER_RENDERER_PIXBUF (renderer));

	set_icon_name (renderer, icon_name);
}

const gchar *
ctk_source_gutter_renderer_pixbuf_get_icon_name (CtkSourceGutterRendererPixbuf *renderer)
{
	g_return_val_if_fail (CTK_SOURCE_IS_GUTTER_RENDERER_PIXBUF (renderer), NULL);

	return ctk_source_pixbuf_helper_get_icon_name (renderer->priv->helper);
}
