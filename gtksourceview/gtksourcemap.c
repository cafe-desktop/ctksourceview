/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8; coding: utf-8 -*- */
/*
 * Copyright (C) 2015 Christian Hergert <christian@hergert.me>
 * Copyright (C) 2015 Ignacio Casal Quinteiro <icq@gnome.org>
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

#include "ctksourcemap.h"
#include <string.h>
#include "ctksourcebuffer.h"
#include "ctksourcecompletion.h"
#include "ctksourcestyle-private.h"
#include "ctksourcestylescheme.h"
#include "ctksourceutils-private.h"

/**
 * SECTION:map
 * @Short_description: Widget that displays a map for a specific #CtkSourceView
 * @Title: CtkSourceMap
 * @See_also: #CtkSourceView
 *
 * #CtkSourceMap is a widget that maps the content of a #CtkSourceView into
 * a smaller view so the user can have a quick overview of the whole document.
 *
 * This works by connecting a #CtkSourceView to to the #CtkSourceMap using
 * the #CtkSourceMap:view property or ctk_source_map_set_view().
 *
 * #CtkSourceMap is a #CtkSourceView object. This means that you can add a
 * #CtkSourceGutterRenderer to a gutter in the same way you would for a
 * #CtkSourceView. One example might be a #CtkSourceGutterRenderer that shows
 * which lines have changed in the document.
 *
 * Additionally, it is desirable to match the font of the #CtkSourceMap and
 * the #CtkSourceView used for editing. Therefore, #CtkSourceMap:font-desc
 * should be used to set the target font. You will need to adjust this to the
 * desired font size for the map. A 1pt font generally seems to be an
 * appropriate font size. "Monospace 1" is the default. See
 * pango_font_description_set_size() for how to alter the size of an existing
 * #PangoFontDescription.
 */

/*
 * Implementation Notes:
 *
 * I tried implementing this a few different ways. They are worth noting so
 * that we do not repeat the same mistakes.
 *
 * Originally, I thought using a CtkSourceView to do the rendering was overkill
 * and would likely slow things down too much. But it turns out to have been
 * the best option so far.
 *
 *   - CtkPixelCache support results in very few CtkTextLayout relayout and
 *     sizing changes. Since the pixel cache renders +/- half a screen outside
 *     the visible range, scrolling is also quite smooth as we very rarely
 *     perform a new ctk_text_layout_draw().
 *
 *   - Performance for this type of widget is dominated by text layout
 *     rendering. When you scale out this car, you increase the number of
 *     layouts to be rendered greatly.
 *
 *   - We can pack CtkSourceGutterRenderer into the child view to provide
 *     additional information. This is quite handy to show information such
 *     as errors, line changes, and anything else that can help the user
 *     quickly jump to the target location.
 *
 * I also tried drawing the contents of the CtkSourceView onto a widget after
 * performing a cairo_scale(). This does not help us much because we ignore
 * pixel cache when cair_scale is not 1-to-1. This results in layout
 * invalidation and worst case render paths.
 *
 * I also tried rendering the scrubber (overlay box) during the
 * CtkTextView::draw_layer() vfunc. The problem with this approach is that
 * the scrubber contents are actually pixel cached. So every time the scrubber
 * moves we have to invalidate the CtkTextLayout and redraw cached contents.
 * Where as drawing in the CtkTextView::draw() vfunc, after the pixel cache
 * contents have been drawn results in only a composite blend, not
 * invalidating any of the pixel cached text layouts.
 *
 * In the future, we might consider bundling a custom font for the source map.
 * Other overview maps have used a "block" font. However, they typically do
 * that because of the glyph rendering cost. Since we have pixel cache, that
 * deficiency is largely a non-issue. But Pango recently got support for
 * embedding fonts in the application, so it is at least possible to bundle
 * our own font as a resource.
 *
 * By default we use a 1pt Monospace font. However, if the Ctksourcemap:font-desc
 * property is set, we will use that instead.
 *
 * We do not render the background grid as it requires a bunch of
 * cpu time for something that will essentially just create a solid
 * color background.
 *
 * The width of the view is determined by the
 * #CtkSourceView:right-margin-position. We cache the width of a
 * single "X" character and multiple that by the right-margin-position.
 * That becomes our size-request width.
 *
 * We do not allow horizontal scrolling so that the overflow text
 * is simply not visible in the minimap.
 *
 * -- Christian
 */

#define DEFAULT_WIDTH 100

typedef struct
{
	/*
	 * By default, we use "Monospace 1pt". However, most text editing
	 * applications will have a custom font, so we allow them to set
	 * that here. Generally speaking, you will want to continue using
	 * a 1pt font, but if they set CtkSourceMap:font-desc, then they
	 * should also shrink the font to the desired size.
	 *
	 * For example:
	 *   pango_font_description_set_size(font_desc, 1 * PANGO_SCALE);
	 *
	 * Would set a 1pt font on whatever PangoFontDescription you have
	 * in your text editor.
	 */
	PangoFontDescription *font_desc;

	/*
	 * The easiest way to style the scrubber and the sourceview is
	 * by using CSS. This is necessary since we can't mess with the
	 * fonts used in the textbuffer (as one might using CtkTextTag).
	 */
	CtkCssProvider *css_provider;

	/* The CtkSourceView we are providing a map of */
	CtkSourceView *view;

	/* A weak pointer to the connected buffer */
	CtkTextBuffer *buffer;

	/* The location of the scrubber in widget coordinate space. */
	GdkRectangle scrubber_area;

	/* Weak pointer view to child view bindings */
	GBinding *buffer_binding;
	GBinding *indent_width_binding;
	GBinding *tab_width_binding;

	/* Our signal handler for buffer changes */
	gulong view_notify_buffer_handler;
	gulong view_vadj_value_changed_handler;
	gulong view_vadj_notify_upper_handler;

	/* Signals connected indirectly to the buffer */
	gulong buffer_notify_style_scheme_handler;

	/* Denotes if we are in a grab from button press */
	guint in_press : 1;
} CtkSourceMapPrivate;

enum
{
	PROP_0,
	PROP_VIEW,
	PROP_FONT_DESC,
	N_PROPERTIES
};

G_DEFINE_TYPE_WITH_PRIVATE (CtkSourceMap, ctk_source_map, CTK_SOURCE_TYPE_VIEW)

static GParamSpec *properties[N_PROPERTIES];

static void
update_scrubber_position (CtkSourceMap *map)
{
	CtkSourceMapPrivate *priv;
	CtkTextIter iter;
	GdkRectangle visible_area;
	GdkRectangle iter_area;
	GdkRectangle scrubber_area;
	CtkAllocation alloc;
	CtkAllocation view_alloc;
	gint child_height;
	gint view_height;
	gint y;

	priv = ctk_source_map_get_instance_private (map);

	if (priv->view == NULL)
	{
		return;
	}

	ctk_widget_get_allocation (CTK_WIDGET (priv->view), &view_alloc);
	ctk_widget_get_allocation (CTK_WIDGET (map), &alloc);

	ctk_widget_get_preferred_height (CTK_WIDGET (priv->view), NULL, &view_height);
	ctk_widget_get_preferred_height (CTK_WIDGET (map), NULL, &child_height);

	ctk_text_view_get_visible_rect (CTK_TEXT_VIEW (priv->view), &visible_area);
	ctk_text_view_get_iter_at_location (CTK_TEXT_VIEW (priv->view), &iter,
	                                    visible_area.x, visible_area.y);
	ctk_text_view_get_iter_location (CTK_TEXT_VIEW (map), &iter, &iter_area);
	ctk_text_view_buffer_to_window_coords (CTK_TEXT_VIEW (map),
	                                       CTK_TEXT_WINDOW_WIDGET,
	                                       iter_area.x, iter_area.y,
	                                       NULL, &y);

	scrubber_area.x = 0;
	scrubber_area.width = alloc.width;
	scrubber_area.y = y;
	scrubber_area.height = ((gdouble)view_alloc.height /
	                        (gdouble)view_height *
	                        (gdouble)child_height) +
	                       iter_area.height;

	if (memcmp (&scrubber_area, &priv->scrubber_area, sizeof scrubber_area) != 0)
	{
		GdkWindow *window;

		/*
		 * NOTE:
		 *
		 * Initially we had a ctk_widget_queue_draw() here thinking
		 * that we would hit the pixel cache and everything would be
		 * fine. However, it actually has a noticible improvement on
		 * interactivity to simply invalidate the old and new region
		 * in the widgets primary GdkWindow. Since the window is
		 * not the CTK_TEXT_WINDOW_TEXT, we don't seem to invalidate
		 * the pixel cache. This makes things as interactive as they
		 * were when drawing the scrubber from a parent widget.
		 */
		window = ctk_text_view_get_window (CTK_TEXT_VIEW (map), CTK_TEXT_WINDOW_WIDGET);
		if (window != NULL)
		{
			cdk_window_invalidate_rect (window, &priv->scrubber_area, FALSE);
			cdk_window_invalidate_rect (window, &scrubber_area, FALSE);
		}

		priv->scrubber_area = scrubber_area;
	}
}

static void
ctk_source_map_rebuild_css (CtkSourceMap *map)
{
	CtkSourceMapPrivate *priv;
	CtkSourceStyleScheme *style_scheme;
	CtkSourceStyle *style = NULL;
	CtkTextBuffer *buffer;
	GString *gstr;
	gboolean alter_alpha = TRUE;
	gchar *background = NULL;

	priv = ctk_source_map_get_instance_private (map);

	if (priv->view == NULL)
	{
		return;
	}

	/*
	 * This is where we calculate the CSS that maps the font for the
	 * minimap as well as the styling for the scrubber.
	 *
	 * The font is calculated from #CtkSourceMap:font-desc. We convert this
	 * to CSS using _ctk_source_utils_pango_font_description_to_css(). It
	 * gets applied to the minimap widget via the CSS style provider which
	 * we attach to the view in ctk_source_map_init().
	 *
	 * The rules for calculating the style for the scrubber are as follows.
	 *
	 * If the current style scheme provides a background color for the
	 * scrubber using the "map-overlay" style name, we use that without
	 * any transformations.
	 *
	 * If the style scheme contains a "selection" style scheme, used for
	 * selected text, we use that with a 0.75 alpha value.
	 *
	 * If none of these are met, we take the background from the
	 * #CtkStyleContext using the deprecated
	 * ctk_style_context_get_background_color(). This is non-ideal, but
	 * currently required since we cannot indicate that we want to
	 * alter the alpha for ctk_render_background().
	 */

	gstr = g_string_new (NULL);

	/* Calculate the font if one has been set */
	if (priv->font_desc != NULL)
	{
		gchar *css;

		css = _ctk_source_utils_pango_font_description_to_css (priv->font_desc);
		g_string_append_printf (gstr, "textview { %s }\n", css != NULL ? css : "");
		g_free (css);
	}

	buffer = ctk_text_view_get_buffer (CTK_TEXT_VIEW (priv->view));
	style_scheme = ctk_source_buffer_get_style_scheme (CTK_SOURCE_BUFFER (buffer));

	if (style_scheme != NULL)
	{

		style = ctk_source_style_scheme_get_style (style_scheme, "map-overlay");

		if (style != NULL)
		{
			/* styling is taking as is only if we found a "map-overlay". */
			alter_alpha = FALSE;
		}
		else
		{
			style = ctk_source_style_scheme_get_style (style_scheme, "selection");
		}
	}

	if (style != NULL)
	{
		g_object_get (style,
		              "background", &background,
		              NULL);
	}

	if (background == NULL)
	{
		CtkStyleContext *context;
		GdkRGBA color;

		/*
		 * We failed to locate a style for both "map-overlay" and for
		 * "selection". That means we need to fallback to using the
		 * selected color for the ctk+ theme. This uses deprecated
		 * API because we have no way to tell ctk_render_background()
		 * to render with an alpha.
		 */

		context = ctk_widget_get_style_context (CTK_WIDGET (priv->view));
		ctk_style_context_save (context);
		ctk_style_context_add_class (context, "view");
		ctk_style_context_set_state (context, CTK_STATE_FLAG_SELECTED);
		G_GNUC_BEGIN_IGNORE_DEPRECATIONS;
		ctk_style_context_get_background_color (context,
							ctk_style_context_get_state (context),
							&color);
		G_GNUC_END_IGNORE_DEPRECATIONS;
		ctk_style_context_restore (context);
		background = cdk_rgba_to_string (&color);

		/*
		 * Make sure we alter the alpha. It is possible this could be
		 * FALSE here if we found a style for map-overlay but it did
		 * not contain a background color.
		 */
		alter_alpha = TRUE;
	}

	if (alter_alpha)
	{
		GdkRGBA color;

		cdk_rgba_parse (&color, background);
		color.alpha = 0.75;
		g_free (background);
		background = cdk_rgba_to_string (&color);
	}


	if (background != NULL)
	{
		g_string_append_printf (gstr,
		                        "textview.scrubber {\n"
		                        "\tbackground-color: %s;\n"
		                        "\tborder-top: 1px solid shade(%s,0.9);\n"
		                        "\tborder-bottom: 1px solid shade(%s,0.9);\n"
		                        "}\n",
		                        background,
		                        background,
		                        background);
	}

	g_free (background);

	if (gstr->len > 0)
	{
		ctk_css_provider_load_from_data (priv->css_provider, gstr->str, gstr->len, NULL);
	}

	g_string_free (gstr, TRUE);
}

static void
update_child_vadjustment (CtkSourceMap *map)
{
	CtkSourceMapPrivate *priv;
	CtkAdjustment *vadj;
	CtkAdjustment *child_vadj;
	gdouble value;
	gdouble upper;
	gdouble page_size;
	gdouble child_upper;
	gdouble child_page_size;
	gdouble new_value = 0.0;

	priv = ctk_source_map_get_instance_private (map);

	vadj = ctk_scrollable_get_vadjustment (CTK_SCROLLABLE (priv->view));
	g_object_get (vadj,
	              "upper", &upper,
	              "value", &value,
	              "page-size", &page_size,
	              NULL);

	child_vadj = ctk_scrollable_get_vadjustment (CTK_SCROLLABLE (map));
	g_object_get (child_vadj,
	              "upper", &child_upper,
	              "page-size", &child_page_size,
	              NULL);

	/*
	 * FIXME:
	 * Technically we should take into account lower here, but in practice
	 *       it is always 0.0.
	 */
	if (child_page_size < child_upper)
	{
		new_value = (value / (upper - page_size)) * (child_upper - child_page_size);
	}

	ctk_adjustment_set_value (child_vadj, new_value);
}

static void
view_vadj_value_changed (CtkSourceMap  *map,
                         CtkAdjustment *vadj)
{
	update_child_vadjustment (map);
	update_scrubber_position (map);
}

static void
view_vadj_notify_upper (CtkSourceMap  *map,
                        GParamSpec    *pspec,
                        CtkAdjustment *vadj)
{
	update_scrubber_position (map);
}

static void
buffer_notify_style_scheme (CtkSourceMap  *map,
                            GParamSpec    *pspec,
                            CtkTextBuffer *buffer)
{
	ctk_source_map_rebuild_css (map);
}

static void
connect_buffer (CtkSourceMap  *map,
                CtkTextBuffer *buffer)
{
	CtkSourceMapPrivate *priv;

	priv = ctk_source_map_get_instance_private (map);

	priv->buffer = buffer;
	g_object_add_weak_pointer (G_OBJECT (buffer), (gpointer *)&priv->buffer);

	priv->buffer_notify_style_scheme_handler =
		g_signal_connect_object (buffer,
		                         "notify::style-scheme",
		                         G_CALLBACK (buffer_notify_style_scheme),
		                         map,
		                         G_CONNECT_SWAPPED);

	buffer_notify_style_scheme (map, NULL, buffer);
}

static void
disconnect_buffer (CtkSourceMap  *map)
{
	CtkSourceMapPrivate *priv;

	priv = ctk_source_map_get_instance_private (map);

	if (priv->buffer == NULL)
	{
		return;
	}

	if (priv->buffer_notify_style_scheme_handler != 0)
	{
		g_signal_handler_disconnect (priv->buffer,
		                             priv->buffer_notify_style_scheme_handler);
		priv->buffer_notify_style_scheme_handler = 0;
	}

	g_object_remove_weak_pointer (G_OBJECT (priv->buffer), (gpointer *)&priv->buffer);
	priv->buffer = NULL;
}

static void
view_notify_buffer (CtkSourceMap  *map,
                    GParamSpec    *pspec,
                    CtkSourceView *view)
{
	CtkSourceMapPrivate *priv;
	CtkTextBuffer *buffer;

	priv = ctk_source_map_get_instance_private (map);

	if (priv->buffer != NULL)
	{
		disconnect_buffer (map);
	}

	buffer = ctk_text_view_get_buffer (CTK_TEXT_VIEW (view));

	if (buffer != NULL)
	{
		connect_buffer (map, buffer);
	}
}

static void
ctk_source_map_set_font_desc (CtkSourceMap               *map,
                              const PangoFontDescription *font_desc)
{
	CtkSourceMapPrivate *priv;

	priv = ctk_source_map_get_instance_private (map);

	if (font_desc != priv->font_desc)
	{
		g_clear_pointer (&priv->font_desc, pango_font_description_free);

		if (font_desc)
		{
			priv->font_desc = pango_font_description_copy (font_desc);
		}
	}

	ctk_source_map_rebuild_css (map);
}

static void
ctk_source_map_set_font_name (CtkSourceMap *map,
                              const gchar  *font_name)
{
	PangoFontDescription *font_desc;

	if (font_name == NULL)
	{
		font_name = "Monospace 1";
	}

	font_desc = pango_font_description_from_string (font_name);
	ctk_source_map_set_font_desc (map, font_desc);
	pango_font_description_free (font_desc);
}

static void
ctk_source_map_get_preferred_width (CtkWidget *widget,
                                    gint      *mininum_width,
                                    gint      *natural_width)
{
	CtkSourceMap *map = CTK_SOURCE_MAP (widget);
	CtkSourceMapPrivate *priv;
	PangoLayout *layout;
	gint height;
	gint width;

	priv = ctk_source_map_get_instance_private (map);

	if (priv->font_desc == NULL)
	{
		*mininum_width = *natural_width = DEFAULT_WIDTH;
		return;
	}

	/*
	 * FIXME:
	 *
	 * This seems like the type of thing we should calculate when
	 * rebuilding our CSS since it gets used a bunch and changes
	 * very little.
	 */
	layout = ctk_widget_create_pango_layout (CTK_WIDGET (map), "X");
	pango_layout_get_pixel_size (layout, &width, &height);
	g_object_unref (layout);

	width *= ctk_source_view_get_right_margin_position (priv->view);

	*mininum_width = *natural_width = width;
}

static void
ctk_source_map_get_preferred_height (CtkWidget *widget,
                                     gint      *minimum_height,
                                     gint      *natural_height)
{
	CtkSourceMap *map = CTK_SOURCE_MAP (widget);
	CtkSourceMapPrivate *priv;

	priv = ctk_source_map_get_instance_private (map);

	if (priv->view == NULL)
	{
		*minimum_height = *natural_height = 0;
		return;
	}

	CTK_WIDGET_CLASS (ctk_source_map_parent_class)->get_preferred_height (widget,
	                                                                      minimum_height,
	                                                                      natural_height);

	*minimum_height = 0;
}


/*
 * This scrolls using buffer coordinates.
 * Translate your event location to a buffer coordinate before
 * calling this function.
 */
static void
scroll_to_child_point (CtkSourceMap   *map,
                       const GdkPoint *point)
{
	CtkSourceMapPrivate *priv;

	priv = ctk_source_map_get_instance_private (map);

	if (priv->view != NULL)
	{
		CtkAllocation alloc;
		CtkTextIter iter;

		ctk_widget_get_allocation (CTK_WIDGET (map), &alloc);

		ctk_text_view_get_iter_at_location (CTK_TEXT_VIEW (map),
		                                    &iter, point->x, point->y);

		_ctk_source_view_jump_to_iter (CTK_TEXT_VIEW (priv->view), &iter,
		                               0.0, TRUE, 1.0, 0.5);
	}
}

static void
ctk_source_map_size_allocate (CtkWidget     *widget,
                              CtkAllocation *alloc)
{
	CtkSourceMap *map = CTK_SOURCE_MAP (widget);

	CTK_WIDGET_CLASS (ctk_source_map_parent_class)->size_allocate (widget, alloc);

	update_scrubber_position (map);
}

static void
connect_view (CtkSourceMap  *map,
              CtkSourceView *view)
{
	CtkSourceMapPrivate *priv;
	CtkAdjustment *vadj;

	priv = ctk_source_map_get_instance_private (map);

	priv->view = view;
	g_object_add_weak_pointer (G_OBJECT (view), (gpointer *)&priv->view);

	vadj = ctk_scrollable_get_vadjustment (CTK_SCROLLABLE (view));

	priv->buffer_binding =
		g_object_bind_property (view, "buffer",
		                        map, "buffer",
		                        G_BINDING_SYNC_CREATE);
	g_object_add_weak_pointer (G_OBJECT (priv->buffer_binding),
	                           (gpointer *)&priv->buffer_binding);

	priv->indent_width_binding =
		g_object_bind_property (view, "indent-width",
		                        map, "indent-width",
		                        G_BINDING_SYNC_CREATE);
	g_object_add_weak_pointer (G_OBJECT (priv->indent_width_binding),
	                           (gpointer *)&priv->indent_width_binding);

	priv->tab_width_binding =
		g_object_bind_property (view, "tab-width",
		                        map, "tab-width",
		                        G_BINDING_SYNC_CREATE);
	g_object_add_weak_pointer (G_OBJECT (priv->tab_width_binding),
	                           (gpointer *)&priv->tab_width_binding);

	priv->view_notify_buffer_handler =
		g_signal_connect_object (view,
		                         "notify::buffer",
		                         G_CALLBACK (view_notify_buffer),
		                         map,
		                         G_CONNECT_SWAPPED);
	view_notify_buffer (map, NULL, view);

	priv->view_vadj_value_changed_handler =
		g_signal_connect_object (vadj,
		                         "value-changed",
		                         G_CALLBACK (view_vadj_value_changed),
		                         map,
		                         G_CONNECT_SWAPPED);

	priv->view_vadj_notify_upper_handler =
		g_signal_connect_object (vadj,
		                         "notify::upper",
		                         G_CALLBACK (view_vadj_notify_upper),
		                         map,
		                         G_CONNECT_SWAPPED);

	if ((ctk_widget_get_events (CTK_WIDGET (priv->view)) & GDK_ENTER_NOTIFY_MASK) == 0)
	{
		ctk_widget_add_events (CTK_WIDGET (priv->view), GDK_ENTER_NOTIFY_MASK);
	}

	if ((ctk_widget_get_events (CTK_WIDGET (priv->view)) & GDK_LEAVE_NOTIFY_MASK) == 0)
	{
		ctk_widget_add_events (CTK_WIDGET (priv->view), GDK_LEAVE_NOTIFY_MASK);
	}

	/* If we are not visible, we want to block certain signal handlers */
	if (!ctk_widget_get_visible (CTK_WIDGET (map)))
	{
		g_signal_handler_block (vadj, priv->view_vadj_value_changed_handler);
		g_signal_handler_block (vadj, priv->view_vadj_notify_upper_handler);
	}

	ctk_source_map_rebuild_css (map);
}

static void
disconnect_view (CtkSourceMap *map)
{
	CtkSourceMapPrivate *priv;
	CtkAdjustment *vadj;

	priv = ctk_source_map_get_instance_private (map);

	if (priv->view == NULL)
	{
		return;
	}

	disconnect_buffer (map);

	if (priv->buffer_binding != NULL)
	{
		g_object_remove_weak_pointer (G_OBJECT (priv->buffer_binding),
		                              (gpointer *)&priv->buffer_binding);
		g_binding_unbind (priv->buffer_binding);
		priv->buffer_binding = NULL;
	}

	if (priv->indent_width_binding != NULL)
	{
		g_object_remove_weak_pointer (G_OBJECT (priv->indent_width_binding),
		                              (gpointer *)&priv->indent_width_binding);
		g_binding_unbind (priv->indent_width_binding);
		priv->indent_width_binding = NULL;
	}

	if (priv->tab_width_binding != NULL)
	{
		g_object_remove_weak_pointer (G_OBJECT (priv->tab_width_binding),
		                              (gpointer *)&priv->tab_width_binding);
		g_binding_unbind (priv->tab_width_binding);
		priv->tab_width_binding = NULL;
	}

	if (priv->view_notify_buffer_handler != 0)
	{
		g_signal_handler_disconnect (priv->view, priv->view_notify_buffer_handler);
		priv->view_notify_buffer_handler = 0;
	}

	vadj = ctk_scrollable_get_vadjustment (CTK_SCROLLABLE (priv->view));
	if (vadj != NULL)
	{
		g_signal_handler_disconnect (vadj, priv->view_vadj_value_changed_handler);
		priv->view_vadj_value_changed_handler = 0;

		g_signal_handler_disconnect (vadj, priv->view_vadj_notify_upper_handler);
		priv->view_vadj_notify_upper_handler = 0;
	}

	g_object_remove_weak_pointer (G_OBJECT (priv->view), (gpointer *)&priv->view);
	priv->view = NULL;
}

static void
ctk_source_map_destroy (CtkWidget *widget)
{
	CtkSourceMap *map = CTK_SOURCE_MAP (widget);
	CtkSourceMapPrivate *priv;

	priv = ctk_source_map_get_instance_private (map);

	disconnect_buffer (map);
	disconnect_view (map);

	g_clear_object (&priv->css_provider);
	g_clear_pointer (&priv->font_desc, pango_font_description_free);

	CTK_WIDGET_CLASS (ctk_source_map_parent_class)->destroy (widget);
}

static gboolean
ctk_source_map_draw (CtkWidget *widget,
                     cairo_t   *cr)
{
	CtkSourceMap *map = CTK_SOURCE_MAP (widget);
	CtkSourceMapPrivate *priv;
	CtkStyleContext *style_context;

	priv = ctk_source_map_get_instance_private (map);

	style_context = ctk_widget_get_style_context (widget);

	CTK_WIDGET_CLASS (ctk_source_map_parent_class)->draw (widget, cr);

	ctk_style_context_save (style_context);
	ctk_style_context_add_class (style_context, "scrubber");
	ctk_render_background (style_context, cr,
	                       priv->scrubber_area.x, priv->scrubber_area.y,
	                       priv->scrubber_area.width, priv->scrubber_area.height);
	ctk_style_context_restore (style_context);

	return FALSE;
}

static void
ctk_source_map_get_property (GObject    *object,
                             guint       prop_id,
                             GValue     *value,
                             GParamSpec *pspec)
{
	CtkSourceMap *map = CTK_SOURCE_MAP (object);
	CtkSourceMapPrivate *priv;

	priv = ctk_source_map_get_instance_private (map);

	switch (prop_id)
	{
		case PROP_FONT_DESC:
			g_value_set_boxed (value, priv->font_desc);
			break;

		case PROP_VIEW:
			g_value_set_object (value, ctk_source_map_get_view (map));
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
	}
}

static void
ctk_source_map_set_property (GObject      *object,
                             guint         prop_id,
                             const GValue *value,
                             GParamSpec   *pspec)
{
	CtkSourceMap *map = CTK_SOURCE_MAP (object);

	switch (prop_id)
	{
		case PROP_VIEW:
			ctk_source_map_set_view (map, g_value_get_object (value));
			break;

		case PROP_FONT_DESC:
			ctk_source_map_set_font_desc (map, g_value_get_boxed (value));
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
	}
}

static gboolean
ctk_source_map_button_press_event (CtkWidget      *widget,
                                   GdkEventButton *event)
{
	CtkSourceMap *map = CTK_SOURCE_MAP (widget);
	CtkSourceMapPrivate *priv;
	GdkPoint point;

	priv = ctk_source_map_get_instance_private (map);

	point.x = event->x;
	point.y = event->y;

	ctk_text_view_window_to_buffer_coords (CTK_TEXT_VIEW (map),
	                                       CTK_TEXT_WINDOW_WIDGET,
	                                       event->x, event->y,
	                                       &point.x, &point.y);

	scroll_to_child_point (map, &point);

	ctk_grab_add (widget);

	priv->in_press = TRUE;

	return GDK_EVENT_STOP;
}

static gboolean
ctk_source_map_button_release_event (CtkWidget      *widget,
                                     GdkEventButton *event)
{
	CtkSourceMap *map = CTK_SOURCE_MAP (widget);
	CtkSourceMapPrivate *priv;

	priv = ctk_source_map_get_instance_private (map);

	ctk_grab_remove (widget);

	priv->in_press = FALSE;

	return GDK_EVENT_STOP;
}


static gboolean
ctk_source_map_motion_notify_event (CtkWidget      *widget,
                                    GdkEventMotion *event)
{
	CtkSourceMap *map = CTK_SOURCE_MAP (widget);
	CtkSourceMapPrivate *priv;

	priv = ctk_source_map_get_instance_private (map);

	if (priv->in_press && (priv->view != NULL))
	{
		CtkTextBuffer *buffer;
		CtkAllocation alloc;
		GdkRectangle area;
		CtkTextIter iter;
		GdkPoint point;
		gdouble yratio;
		gint height;

		ctk_widget_get_allocation (widget, &alloc);
		ctk_widget_get_preferred_height (widget, NULL, &height);
		if (height > 0)
		{
			height = MIN (height, alloc.height);
		}

		buffer = ctk_text_view_get_buffer (CTK_TEXT_VIEW (map));
		ctk_text_buffer_get_end_iter (buffer, &iter);
		ctk_text_view_get_iter_location (CTK_TEXT_VIEW (map), &iter, &area);

		yratio = CLAMP (event->y - alloc.y, 0, height) / (gdouble)height;

		point.x = 0;
		point.y = (area.y + area.height) * yratio;

		scroll_to_child_point (map, &point);
	}

	return GDK_EVENT_STOP;
}

static gboolean
ctk_source_map_scroll_event (CtkWidget      *widget,
                             GdkEventScroll *event)
{
	CtkSourceMap *map = CTK_SOURCE_MAP (widget);
	CtkSourceMapPrivate *priv;
	static const gint scroll_acceleration = 4;

	priv = ctk_source_map_get_instance_private (map);

	/*
	 * FIXME:
	 *
	 * This doesn't propagate kinetic scrolling or anything.
	 * We should probably make something that does that.
	 */
	if (priv->view != NULL)
	{
		gdouble x;
		gdouble y;
		gint count = 0;

		if (event->direction == GDK_SCROLL_UP)
		{
			count = -scroll_acceleration;
		}
		else if (event->direction == GDK_SCROLL_DOWN)
		{
			count = scroll_acceleration;
		}
		else
		{
			cdk_event_get_scroll_deltas ((GdkEvent *)event, &x, &y);

			if (y > 0)
			{
				count = scroll_acceleration;
			}
			else if (y < 0)
			{
				count = -scroll_acceleration;
			}
		}

		if (count != 0)
		{
			g_signal_emit_by_name (priv->view, "move-viewport",
			                       CTK_SCROLL_STEPS, count);
			return GDK_EVENT_STOP;
		}
	}

	return GDK_EVENT_PROPAGATE;
}

static void
set_view_cursor (CtkSourceMap *map)
{
	GdkWindow *window;

	window = ctk_text_view_get_window (CTK_TEXT_VIEW (map),
	                                   CTK_TEXT_WINDOW_TEXT);
	if (window != NULL)
	{
		cdk_window_set_cursor (window, NULL);
	}
}

static void
ctk_source_map_state_flags_changed (CtkWidget     *widget,
                                    CtkStateFlags  flags)
{
	CTK_WIDGET_CLASS (ctk_source_map_parent_class)->state_flags_changed (widget, flags);

	set_view_cursor (CTK_SOURCE_MAP (widget));
}

static void
ctk_source_map_realize (CtkWidget *widget)
{
	CTK_WIDGET_CLASS (ctk_source_map_parent_class)->realize (widget);

	set_view_cursor (CTK_SOURCE_MAP (widget));
}

static void
ctk_source_map_show (CtkWidget *widget)
{
	CtkSourceMap *map = CTK_SOURCE_MAP (widget);
	CtkSourceMapPrivate *priv;
	CtkAdjustment *vadj;

	CTK_WIDGET_CLASS (ctk_source_map_parent_class)->show (widget);

	priv = ctk_source_map_get_instance_private (map);

	if (priv->view != NULL)
	{
		vadj = ctk_scrollable_get_vadjustment (CTK_SCROLLABLE (priv->view));

		g_signal_handler_unblock (vadj, priv->view_vadj_value_changed_handler);
		g_signal_handler_unblock (vadj, priv->view_vadj_notify_upper_handler);

		g_object_notify (G_OBJECT (vadj), "upper");
		g_signal_emit_by_name (vadj, "value-changed");
	}
}

static void
ctk_source_map_hide (CtkWidget *widget)
{
	CtkSourceMap *map = CTK_SOURCE_MAP (widget);
	CtkSourceMapPrivate *priv;
	CtkAdjustment *vadj;

	CTK_WIDGET_CLASS (ctk_source_map_parent_class)->hide (widget);

	priv = ctk_source_map_get_instance_private (map);

	if (priv->view != NULL)
	{
		vadj = ctk_scrollable_get_vadjustment (CTK_SCROLLABLE (priv->view));
		g_signal_handler_block (vadj, priv->view_vadj_value_changed_handler);
		g_signal_handler_block (vadj, priv->view_vadj_notify_upper_handler);
	}
}

static void
ctk_source_map_class_init (CtkSourceMapClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	CtkWidgetClass *widget_class = CTK_WIDGET_CLASS (klass);

	object_class->get_property = ctk_source_map_get_property;
	object_class->set_property = ctk_source_map_set_property;

	widget_class->destroy = ctk_source_map_destroy;
	widget_class->draw = ctk_source_map_draw;
	widget_class->get_preferred_height = ctk_source_map_get_preferred_height;
	widget_class->get_preferred_width = ctk_source_map_get_preferred_width;
	widget_class->hide = ctk_source_map_hide;
	widget_class->size_allocate = ctk_source_map_size_allocate;
	widget_class->button_press_event = ctk_source_map_button_press_event;
	widget_class->button_release_event = ctk_source_map_button_release_event;
	widget_class->motion_notify_event = ctk_source_map_motion_notify_event;
	widget_class->scroll_event = ctk_source_map_scroll_event;
	widget_class->show = ctk_source_map_show;
	widget_class->state_flags_changed = ctk_source_map_state_flags_changed;
	widget_class->realize = ctk_source_map_realize;

	properties[PROP_VIEW] =
		g_param_spec_object ("view",
		                     "View",
		                     "The view this widget is mapping.",
		                     CTK_SOURCE_TYPE_VIEW,
		                     (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

	properties[PROP_FONT_DESC] =
		g_param_spec_boxed ("font-desc",
		                    "Font Description",
		                    "The Pango font description to use.",
		                    PANGO_TYPE_FONT_DESCRIPTION,
		                    (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

	g_object_class_install_properties (object_class, N_PROPERTIES, properties);
}

static void
ctk_source_map_init (CtkSourceMap *map)
{
	CtkSourceMapPrivate *priv;
	CtkSourceCompletion *completion;
	CtkStyleContext *context;

	priv = ctk_source_map_get_instance_private (map);

	priv->css_provider = ctk_css_provider_new ();

	context = ctk_widget_get_style_context (CTK_WIDGET (map));
	ctk_style_context_add_provider (context,
	                                CTK_STYLE_PROVIDER (priv->css_provider),
	                                CTK_SOURCE_STYLE_PROVIDER_PRIORITY + 1);

	g_object_set (map,
	              "auto-indent", FALSE,
	              "can-focus", FALSE,
	              "editable", FALSE,
	              "expand", FALSE,
	              "monospace", TRUE,
	              "show-line-numbers", FALSE,
	              "show-line-marks", FALSE,
	              "show-right-margin", FALSE,
	              "visible", TRUE,
	              NULL);

	ctk_widget_add_events (CTK_WIDGET (map), GDK_SCROLL_MASK);

	completion = ctk_source_view_get_completion (CTK_SOURCE_VIEW (map));
	ctk_source_completion_block_interactive (completion);

	ctk_source_map_set_font_name (map, "Monospace 1");
}

/**
 * ctk_source_map_new:
 *
 * Creates a new #CtkSourceMap.
 *
 * Returns: a new #CtkSourceMap.
 *
 * Since: 3.18
 */
CtkWidget *
ctk_source_map_new (void)
{
	return g_object_new (CTK_SOURCE_TYPE_MAP, NULL);
}

/**
 * ctk_source_map_set_view:
 * @map: a #CtkSourceMap
 * @view: a #CtkSourceView
 *
 * Sets the view that @map will be doing the mapping to.
 *
 * Since: 3.18
 */
void
ctk_source_map_set_view (CtkSourceMap  *map,
                         CtkSourceView *view)
{
	CtkSourceMapPrivate *priv;

	g_return_if_fail (CTK_SOURCE_IS_MAP (map));
	g_return_if_fail (view == NULL || CTK_SOURCE_IS_VIEW (view));

	priv = ctk_source_map_get_instance_private (map);

	if (priv->view == view)
	{
		return;
	}

	if (priv->view != NULL)
	{
		disconnect_view (map);
	}

	if (view != NULL)
	{
		connect_view (map, view);
	}

	g_object_notify_by_pspec (G_OBJECT (map), properties[PROP_VIEW]);
}

/**
 * ctk_source_map_get_view:
 * @map: a #CtkSourceMap.
 *
 * Gets the #CtkSourceMap:view property, which is the view this widget is mapping.
 *
 * Returns: (transfer none) (nullable): a #CtkSourceView or %NULL.
 *
 * Since: 3.18
 */
CtkSourceView *
ctk_source_map_get_view (CtkSourceMap *map)
{
	CtkSourceMapPrivate *priv;

	g_return_val_if_fail (CTK_SOURCE_IS_MAP (map), NULL);

	priv = ctk_source_map_get_instance_private (map);

	return priv->view;
}
