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

#include "ctksourcegutterrenderertext.h"

/**
 * SECTION:gutterrenderertext
 * @Short_description: Renders text in the gutter
 * @Title: CtkSourceGutterRendererText
 * @See_also: #CtkSourceGutterRenderer, #CtkSourceGutter
 *
 * A #CtkSourceGutterRendererText can be used to render text in a cell of
 * #CtkSourceGutter.
 */

struct _CtkSourceGutterRendererTextPrivate
{
	gchar *text;

	PangoLayout *cached_layout;

	guint is_markup : 1;
};

G_DEFINE_TYPE_WITH_PRIVATE (CtkSourceGutterRendererText, ctk_source_gutter_renderer_text, CTK_SOURCE_TYPE_GUTTER_RENDERER)

enum
{
	PROP_0,
	PROP_MARKUP,
	PROP_TEXT
};

static void
gutter_renderer_text_begin (CtkSourceGutterRenderer *renderer,
			    cairo_t                 *cr,
			    CdkRectangle            *background_area,
			    CdkRectangle            *cell_area,
			    CtkTextIter             *start,
			    CtkTextIter             *end)
{
	CtkSourceGutterRendererText *text = CTK_SOURCE_GUTTER_RENDERER_TEXT (renderer);
	CtkTextView *view;

	view = ctk_source_gutter_renderer_get_view (renderer);

	g_clear_object (&text->priv->cached_layout);
	text->priv->cached_layout = ctk_widget_create_pango_layout (CTK_WIDGET (view), NULL);

	if (CTK_SOURCE_GUTTER_RENDERER_CLASS (ctk_source_gutter_renderer_text_parent_class)->begin != NULL)
	{
		CTK_SOURCE_GUTTER_RENDERER_CLASS (ctk_source_gutter_renderer_text_parent_class)->begin (renderer,
													cr,
													background_area,
													cell_area,
													start,
													end);
	}
}

static void
center_on (CtkTextView  *view,
           CdkRectangle *cell_area,
           CtkTextIter  *iter,
           gint          width,
           gint          height,
           gfloat        xalign,
           gfloat        yalign,
           gint         *x,
           gint         *y)
{
	CdkRectangle location;

	ctk_text_view_get_iter_location (view, iter, &location);

	*x = cell_area->x + (cell_area->width - width) * xalign;
	*y = cell_area->y + (location.height - height) * yalign;
}

static void
gutter_renderer_text_draw (CtkSourceGutterRenderer      *renderer,
			   cairo_t                      *cr,
			   CdkRectangle                 *background_area,
			   CdkRectangle                 *cell_area,
			   CtkTextIter                  *start,
			   CtkTextIter                  *end,
			   CtkSourceGutterRendererState  state)
{
	CtkSourceGutterRendererText *text = CTK_SOURCE_GUTTER_RENDERER_TEXT (renderer);
	CtkTextView *view;
	gint width;
	gint height;
	gfloat xalign;
	gfloat yalign;
	CtkSourceGutterRendererAlignmentMode mode;
	gint x = 0;
	gint y = 0;
	CtkStyleContext *context;

	/* Chain up to draw background */
	if (CTK_SOURCE_GUTTER_RENDERER_CLASS (ctk_source_gutter_renderer_text_parent_class)->draw != NULL)
	{
		CTK_SOURCE_GUTTER_RENDERER_CLASS (ctk_source_gutter_renderer_text_parent_class)->draw (renderer,
												       cr,
												       background_area,
												       cell_area,
												       start,
												       end,
												       state);
	}

	view = ctk_source_gutter_renderer_get_view (renderer);

	if (text->priv->is_markup)
	{
		pango_layout_set_markup (text->priv->cached_layout,
		                         text->priv->text,
		                         -1);
	}
	else
	{
		pango_layout_set_text (text->priv->cached_layout,
		                       text->priv->text,
		                       -1);
	}

	pango_layout_get_pixel_size (text->priv->cached_layout, &width, &height);

	ctk_source_gutter_renderer_get_alignment (renderer,
	                                          &xalign,
	                                          &yalign);

	/* Avoid calculations if we don't wrap text */
	if (ctk_text_view_get_wrap_mode (view) == CTK_WRAP_NONE)
	{
		mode = CTK_SOURCE_GUTTER_RENDERER_ALIGNMENT_MODE_CELL;
	}
	else
	{
		mode = ctk_source_gutter_renderer_get_alignment_mode (renderer);
	}

	switch (mode)
	{
		case CTK_SOURCE_GUTTER_RENDERER_ALIGNMENT_MODE_CELL:
			x = cell_area->x + (cell_area->width - width) * xalign;
			y = cell_area->y + (cell_area->height - height) * yalign;
			break;

		case CTK_SOURCE_GUTTER_RENDERER_ALIGNMENT_MODE_FIRST:
			center_on (view,
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
			center_on (view,
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

	context = ctk_widget_get_style_context (CTK_WIDGET (view));
	ctk_render_layout (context, cr, x, y, text->priv->cached_layout);
}

static void
gutter_renderer_text_end (CtkSourceGutterRenderer *renderer)
{
	CtkSourceGutterRendererText *text = CTK_SOURCE_GUTTER_RENDERER_TEXT (renderer);

	g_clear_object (&text->priv->cached_layout);

	if (CTK_SOURCE_GUTTER_RENDERER_CLASS (ctk_source_gutter_renderer_text_parent_class)->end != NULL)
	{
		CTK_SOURCE_GUTTER_RENDERER_CLASS (ctk_source_gutter_renderer_text_parent_class)->end (renderer);
	}
}

static void
measure_text (CtkSourceGutterRendererText *renderer,
              const gchar                 *markup,
              const gchar                 *text,
              gint                        *width,
              gint                        *height)
{
	CtkTextView *view;
	PangoLayout *layout;

	view = ctk_source_gutter_renderer_get_view (CTK_SOURCE_GUTTER_RENDERER (renderer));

	layout = ctk_widget_create_pango_layout (CTK_WIDGET (view), NULL);

	if (markup != NULL)
	{
		pango_layout_set_markup (layout, markup, -1);
	}
	else
	{
		pango_layout_set_text (layout, text, -1);
	}

	pango_layout_get_pixel_size (layout, width, height);

	g_object_unref (layout);
}

/**
 * ctk_source_gutter_renderer_text_measure:
 * @renderer: a #CtkSourceGutterRendererText.
 * @text: the text to measure.
 * @width: (out) (optional): location to store the width of the text in pixels,
 *   or %NULL.
 * @height: (out) (optional): location to store the height of the text in
 *   pixels, or %NULL.
 *
 * Measures the text provided using the pango layout used by the
 * #CtkSourceGutterRendererText.
 */
void
ctk_source_gutter_renderer_text_measure (CtkSourceGutterRendererText *renderer,
                                         const gchar                 *text,
                                         gint                        *width,
                                         gint                        *height)
{
	g_return_if_fail (CTK_SOURCE_IS_GUTTER_RENDERER_TEXT (renderer));
	g_return_if_fail (text != NULL);

	measure_text (renderer, NULL, text, width, height);
}

/**
 * ctk_source_gutter_renderer_text_measure_markup:
 * @renderer: a #CtkSourceGutterRendererText.
 * @markup: the pango markup to measure.
 * @width: (out) (optional): location to store the width of the text in pixels,
 *   or %NULL.
 * @height: (out) (optional): location to store the height of the text in
 *   pixels, or %NULL.
 *
 * Measures the pango markup provided using the pango layout used by the
 * #CtkSourceGutterRendererText.
 */
void
ctk_source_gutter_renderer_text_measure_markup (CtkSourceGutterRendererText *renderer,
                                                const gchar                 *markup,
                                                gint                        *width,
                                                gint                        *height)
{
	g_return_if_fail (CTK_SOURCE_IS_GUTTER_RENDERER_TEXT (renderer));
	g_return_if_fail (markup != NULL);

	measure_text (renderer, markup, NULL, width, height);
}

static void
ctk_source_gutter_renderer_text_finalize (GObject *object)
{
	CtkSourceGutterRendererText *renderer = CTK_SOURCE_GUTTER_RENDERER_TEXT (object);

	g_free (renderer->priv->text);
	g_clear_object (&renderer->priv->cached_layout);

	G_OBJECT_CLASS (ctk_source_gutter_renderer_text_parent_class)->finalize (object);
}

static void
set_text (CtkSourceGutterRendererText *renderer,
          const gchar                 *text,
          gint                         length,
          gboolean                     is_markup)
{
	g_free (renderer->priv->text);

	renderer->priv->text = length >= 0 ? g_strndup (text, length) : g_strdup (text);
	renderer->priv->is_markup = is_markup;
}

static void
ctk_source_gutter_renderer_text_set_property (GObject      *object,
                                              guint         prop_id,
                                              const GValue *value,
                                              GParamSpec   *pspec)
{
	CtkSourceGutterRendererText *renderer;

	renderer = CTK_SOURCE_GUTTER_RENDERER_TEXT (object);

	switch (prop_id)
	{
		case PROP_MARKUP:
			set_text (renderer, g_value_get_string (value), -1, TRUE);
			break;
		case PROP_TEXT:
			set_text (renderer, g_value_get_string (value), -1, FALSE);
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
ctk_source_gutter_renderer_text_get_property (GObject    *object,
                                              guint       prop_id,
                                              GValue     *value,
                                              GParamSpec *pspec)
{
	CtkSourceGutterRendererText *renderer;

	renderer = CTK_SOURCE_GUTTER_RENDERER_TEXT (object);

	switch (prop_id)
	{
		case PROP_MARKUP:
			g_value_set_string (value, renderer->priv->is_markup ? renderer->priv->text : NULL);
			break;
		case PROP_TEXT:
			g_value_set_string (value, !renderer->priv->is_markup ? renderer->priv->text : NULL);
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
ctk_source_gutter_renderer_text_class_init (CtkSourceGutterRendererTextClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	CtkSourceGutterRendererClass *renderer_class = CTK_SOURCE_GUTTER_RENDERER_CLASS (klass);

	object_class->finalize = ctk_source_gutter_renderer_text_finalize;

	object_class->get_property = ctk_source_gutter_renderer_text_get_property;
	object_class->set_property = ctk_source_gutter_renderer_text_set_property;

	renderer_class->begin = gutter_renderer_text_begin;
	renderer_class->draw = gutter_renderer_text_draw;
	renderer_class->end = gutter_renderer_text_end;

	g_object_class_install_property (object_class,
	                                 PROP_MARKUP,
	                                 g_param_spec_string ("markup",
	                                                      "Markup",
	                                                      "The markup",
	                                                      NULL,
	                                                      G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

	g_object_class_install_property (object_class,
	                                 PROP_TEXT,
	                                 g_param_spec_string ("text",
	                                                      "Text",
	                                                      "The text",
	                                                      NULL,
	                                                      G_PARAM_READWRITE | G_PARAM_CONSTRUCT));
}

static void
ctk_source_gutter_renderer_text_init (CtkSourceGutterRendererText *self)
{
	self->priv = ctk_source_gutter_renderer_text_get_instance_private (self);

	self->priv->is_markup = TRUE;
}

/**
 * ctk_source_gutter_renderer_text_new:
 *
 * Create a new #CtkSourceGutterRendererText.
 *
 * Returns: (transfer full): A #CtkSourceGutterRenderer
 *
 **/
CtkSourceGutterRenderer *
ctk_source_gutter_renderer_text_new (void)
{
	return g_object_new (CTK_SOURCE_TYPE_GUTTER_RENDERER_TEXT, NULL);
}

void
ctk_source_gutter_renderer_text_set_markup (CtkSourceGutterRendererText *renderer,
                                            const gchar                 *markup,
                                            gint                         length)
{
	g_return_if_fail (CTK_SOURCE_IS_GUTTER_RENDERER_TEXT (renderer));

	set_text (renderer, markup, length, TRUE);
}

void
ctk_source_gutter_renderer_text_set_text (CtkSourceGutterRendererText *renderer,
                                          const gchar                 *text,
                                          gint                         length)
{
	g_return_if_fail (CTK_SOURCE_IS_GUTTER_RENDERER_TEXT (renderer));

	set_text (renderer, text, length, FALSE);
}
