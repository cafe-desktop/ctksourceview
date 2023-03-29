/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8; coding: utf-8 -*- /
 *
 * This file is part of CtkSourceView
 *
 * Copyright (C) 2009 - Jesse van den Kieboom
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

#include "ctksourcegutter.h"
#include "ctksourcegutter-private.h"
#include "ctksourceview.h"
#include "ctksourcegutterrenderer.h"
#include "ctksourcegutterrenderer-private.h"

/**
 * SECTION:gutter
 * @Short_description: Gutter object for CtkSourceView
 * @Title: CtkSourceGutter
 * @See_also: #CtkSourceView, #CtkSourceMark
 *
 * The #CtkSourceGutter object represents the left or right gutter of the text
 * view. It is used by #CtkSourceView to draw the line numbers and
 * #CtkSourceMark<!-- -->s that might be present on a line. By packing
 * additional #CtkSourceGutterRenderer objects in the gutter, you can extend the
 * gutter with your own custom drawings.
 *
 * To get a #CtkSourceGutter, use the ctk_source_view_get_gutter() function.
 *
 * The gutter works very much the same way as cells rendered in a #CtkTreeView.
 * The concept is similar, with the exception that the gutter does not have an
 * underlying #CtkTreeModel. The builtin line number renderer is at position
 * #CTK_SOURCE_VIEW_GUTTER_POSITION_LINES (-30) and the marks renderer is at
 * #CTK_SOURCE_VIEW_GUTTER_POSITION_MARKS (-20). The gutter sorts the renderers
 * in ascending order, from left to right. So the marks are displayed on the
 * right of the line numbers.
 */

enum
{
	PROP_0,
	PROP_VIEW,
	PROP_WINDOW_TYPE,
};

typedef struct
{
	CtkSourceGutterRenderer *renderer;

	gint prelit;
	gint position;

	gulong queue_draw_handler;
	gulong size_changed_handler;
	gulong notify_xpad_handler;
	gulong notify_ypad_handler;
	gulong notify_visible_handler;
} Renderer;

struct _CtkSourceGutterPrivate
{
	CtkSourceView *view;
	CtkTextWindowType window_type;
	CtkOrientation orientation;

	GList *renderers;

	guint is_drawing : 1;
};

G_DEFINE_TYPE_WITH_PRIVATE (CtkSourceGutter, ctk_source_gutter, G_TYPE_OBJECT)

static gboolean on_view_motion_notify_event (CtkSourceView   *view,
                                             CdkEventMotion  *event,
                                             CtkSourceGutter *gutter);


static gboolean on_view_enter_notify_event (CtkSourceView    *view,
                                            CdkEventCrossing *event,
                                            CtkSourceGutter  *gutter);

static gboolean on_view_leave_notify_event (CtkSourceView    *view,
                                            CdkEventCrossing *event,
                                            CtkSourceGutter  *gutter);

static gboolean on_view_button_press_event (CtkSourceView    *view,
                                            CdkEventButton   *event,
                                            CtkSourceGutter  *gutter);

static gboolean on_view_query_tooltip (CtkSourceView   *view,
                                       gint             x,
                                       gint             y,
                                       gboolean         keyboard_mode,
                                       CtkTooltip      *tooltip,
                                       CtkSourceGutter *gutter);

static void on_view_style_updated (CtkSourceView    *view,
                                   CtkSourceGutter  *gutter);

static void do_redraw (CtkSourceGutter *gutter);
static void update_gutter_size (CtkSourceGutter *gutter);

static CdkWindow *
get_window (CtkSourceGutter *gutter)
{
	return ctk_text_view_get_window (CTK_TEXT_VIEW (gutter->priv->view),
	                                 gutter->priv->window_type);
}

static void
on_renderer_size_changed (CtkSourceGutterRenderer *renderer,
                          GParamSpec              *spec,
                          CtkSourceGutter         *gutter)
{
	update_gutter_size (gutter);
}

static void
on_renderer_queue_draw (CtkSourceGutterRenderer *renderer,
                        CtkSourceGutter         *gutter)
{
	do_redraw (gutter);
}

static void
on_renderer_notify_padding (CtkSourceGutterRenderer *renderer,
                            GParamSpec              *spec,
                            CtkSourceGutter         *gutter)
{
	update_gutter_size (gutter);
}

static void
on_renderer_notify_visible (CtkSourceGutterRenderer *renderer,
                            GParamSpec              *spec,
                            CtkSourceGutter         *gutter)
{
	update_gutter_size (gutter);
}

static Renderer *
renderer_new (CtkSourceGutter         *gutter,
              CtkSourceGutterRenderer *renderer,
              gint                     position)
{
	Renderer *ret = g_slice_new0 (Renderer);

	ret->renderer = g_object_ref_sink (renderer);
	ret->position = position;
	ret->prelit = -1;

	_ctk_source_gutter_renderer_set_view (renderer,
	                                      CTK_TEXT_VIEW (gutter->priv->view),
	                                      gutter->priv->window_type);

	ret->size_changed_handler =
		g_signal_connect (ret->renderer,
		                  "notify::size",
		                  G_CALLBACK (on_renderer_size_changed),
		                  gutter);

	ret->queue_draw_handler =
		g_signal_connect (ret->renderer,
		                  "queue-draw",
		                  G_CALLBACK (on_renderer_queue_draw),
		                  gutter);

	ret->notify_xpad_handler =
		g_signal_connect (ret->renderer,
		                  "notify::xpad",
		                  G_CALLBACK (on_renderer_notify_padding),
		                  gutter);

	ret->notify_ypad_handler =
		g_signal_connect (ret->renderer,
		                  "notify::ypad",
		                  G_CALLBACK (on_renderer_notify_padding),
		                  gutter);

	ret->notify_visible_handler =
		g_signal_connect (ret->renderer,
		                  "notify::visible",
		                  G_CALLBACK (on_renderer_notify_visible),
		                  gutter);

	return ret;
}

static void
renderer_free (Renderer *renderer)
{
	g_signal_handler_disconnect (renderer->renderer,
	                             renderer->queue_draw_handler);

	g_signal_handler_disconnect (renderer->renderer,
	                             renderer->size_changed_handler);

	g_signal_handler_disconnect (renderer->renderer,
	                             renderer->notify_xpad_handler);

	g_signal_handler_disconnect (renderer->renderer,
	                             renderer->notify_ypad_handler);

	g_signal_handler_disconnect (renderer->renderer,
	                             renderer->notify_visible_handler);

	_ctk_source_gutter_renderer_set_view (renderer->renderer,
	                                      NULL,
	                                      CTK_TEXT_WINDOW_PRIVATE);

	g_object_unref (renderer->renderer);
	g_slice_free (Renderer, renderer);
}

static void
ctk_source_gutter_dispose (GObject *object)
{
	CtkSourceGutter *gutter = CTK_SOURCE_GUTTER (object);

	g_list_free_full (gutter->priv->renderers, (GDestroyNotify)renderer_free);
	gutter->priv->renderers = NULL;

	gutter->priv->view = NULL;

	G_OBJECT_CLASS (ctk_source_gutter_parent_class)->dispose (object);
}

static void
ctk_source_gutter_get_property (GObject    *object,
                                guint       prop_id,
                                GValue     *value,
                                GParamSpec *pspec)
{
	CtkSourceGutter *self = CTK_SOURCE_GUTTER (object);

	switch (prop_id)
	{
		case PROP_VIEW:
			g_value_set_object (value, self->priv->view);
			break;
		case PROP_WINDOW_TYPE:
			g_value_set_enum (value, self->priv->window_type);
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
on_view_realize (CtkSourceView   *view,
                 CtkSourceGutter *gutter)
{
	update_gutter_size (gutter);
}

static void
set_view (CtkSourceGutter *gutter,
          CtkSourceView   *view)
{
	gutter->priv->view = view;

	g_signal_connect_object (view,
				 "motion-notify-event",
				 G_CALLBACK (on_view_motion_notify_event),
				 gutter,
				 0);

	g_signal_connect_object (view,
				 "enter-notify-event",
				 G_CALLBACK (on_view_enter_notify_event),
				 gutter,
				 0);

	g_signal_connect_object (view,
				 "leave-notify-event",
				 G_CALLBACK (on_view_leave_notify_event),
				 gutter,
				 0);

	g_signal_connect_object (view,
				 "button-press-event",
				 G_CALLBACK (on_view_button_press_event),
				 gutter,
				 0);

	g_signal_connect_object (view,
				 "query-tooltip",
				 G_CALLBACK (on_view_query_tooltip),
				 gutter,
				 0);

	g_signal_connect_object (view,
				 "realize",
				 G_CALLBACK (on_view_realize),
				 gutter,
				 0);

	g_signal_connect_object (view,
				 "style-updated",
				 G_CALLBACK (on_view_style_updated),
				 gutter,
				 0);
}

static void
do_redraw (CtkSourceGutter *gutter)
{
	CdkWindow *window;

	window = ctk_text_view_get_window (CTK_TEXT_VIEW (gutter->priv->view),
	                                   gutter->priv->window_type);

	if (window && !gutter->priv->is_drawing)
	{
		cdk_window_invalidate_rect (window, NULL, FALSE);
	}
}

static gint
calculate_gutter_size (CtkSourceGutter *gutter,
		       GArray          *sizes)
{
	GList *item;
	gint total_width = 0;

	/* Calculate size */
	for (item = gutter->priv->renderers; item; item = g_list_next (item))
	{
		Renderer *renderer = item->data;
		gint width;

		if (!ctk_source_gutter_renderer_get_visible (renderer->renderer))
		{
			width = 0;
		}
		else
		{
			gint xpad;
			gint size;

			size = ctk_source_gutter_renderer_get_size (renderer->renderer);

			ctk_source_gutter_renderer_get_padding (renderer->renderer,
			                                        &xpad,
			                                        NULL);

			width = size + 2 * xpad;
		}

		if (sizes)
		{
			g_array_append_val (sizes, width);
		}

		total_width += width;
	}

	return total_width;
}

static void
update_gutter_size (CtkSourceGutter *gutter)
{
	gint width = calculate_gutter_size (gutter, NULL);

	ctk_text_view_set_border_window_size (CTK_TEXT_VIEW (gutter->priv->view),
	                                      gutter->priv->window_type,
	                                      width);
}

static void
ctk_source_gutter_set_property (GObject       *object,
                                guint          prop_id,
                                const GValue  *value,
                                GParamSpec    *pspec)
{
	CtkSourceGutter *self = CTK_SOURCE_GUTTER (object);

	switch (prop_id)
	{
		case PROP_VIEW:
			set_view (self, CTK_SOURCE_VIEW (g_value_get_object (value)));
			break;
		case PROP_WINDOW_TYPE:
			self->priv->window_type = g_value_get_enum (value);
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
ctk_source_gutter_constructed (GObject *object)
{
	CtkSourceGutter *gutter;

	gutter = CTK_SOURCE_GUTTER (object);

	if (gutter->priv->window_type == CTK_TEXT_WINDOW_LEFT ||
	    gutter->priv->window_type == CTK_TEXT_WINDOW_RIGHT)
	{
		gutter->priv->orientation = CTK_ORIENTATION_HORIZONTAL;
	}
	else
	{
		gutter->priv->orientation = CTK_ORIENTATION_VERTICAL;
	}

	G_OBJECT_CLASS (ctk_source_gutter_parent_class)->constructed (object);
}

static void
ctk_source_gutter_class_init (CtkSourceGutterClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->set_property = ctk_source_gutter_set_property;
	object_class->get_property = ctk_source_gutter_get_property;

	object_class->dispose = ctk_source_gutter_dispose;
	object_class->constructed = ctk_source_gutter_constructed;

	/**
	 * CtkSourceGutter:view:
	 *
	 * The #CtkSourceView of the gutter.
	 */
	g_object_class_install_property (object_class,
	                                 PROP_VIEW,
	                                 g_param_spec_object ("view",
	                                                      "View",
	                                                      "",
	                                                      CTK_SOURCE_TYPE_VIEW,
	                                                      G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));

	/**
	 * CtkSourceGutter:window-type:
	 *
	 * The text window type on which the window is placed.
	 */
	g_object_class_install_property (object_class,
	                                 PROP_WINDOW_TYPE,
	                                 g_param_spec_enum ("window_type",
	                                                    "Window Type",
	                                                    "The gutters' text window type",
	                                                    CTK_TYPE_TEXT_WINDOW_TYPE,
	                                                    0,
	                                                    G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));
}

static void
ctk_source_gutter_init (CtkSourceGutter *self)
{
	self->priv = ctk_source_gutter_get_instance_private (self);
}

static gint
sort_by_position (Renderer *r1,
                  Renderer *r2,
                  gpointer  data)
{
	if (r1->position < r2->position)
	{
		return -1;
	}
	else if (r1->position > r2->position)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

static void
append_renderer (CtkSourceGutter *gutter,
                 Renderer        *renderer)
{
	gutter->priv->renderers =
		g_list_insert_sorted_with_data (gutter->priv->renderers,
		                                renderer,
		                                (GCompareDataFunc)sort_by_position,
		                                NULL);

	update_gutter_size (gutter);
}

CtkSourceGutter *
_ctk_source_gutter_new (CtkSourceView     *view,
			CtkTextWindowType  type)
{
	return g_object_new (CTK_SOURCE_TYPE_GUTTER,
	                     "view", view,
	                     "window_type", type,
	                     NULL);
}

/* Public API */

/**
 * ctk_source_gutter_get_view:
 * @gutter: a #CtkSourceGutter.
 *
 * Returns: (transfer none): the associated #CtkSourceView.
 * Since: 3.24
 */
CtkSourceView *
ctk_source_gutter_get_view (CtkSourceGutter *gutter)
{
	g_return_val_if_fail (CTK_SOURCE_IS_GUTTER (gutter), NULL);

	return gutter->priv->view;
}

/**
 * ctk_source_gutter_get_window_type:
 * @gutter: a #CtkSourceGutter.
 *
 * Returns: the #CtkTextWindowType of @gutter.
 * Since: 3.24
 */
CtkTextWindowType
ctk_source_gutter_get_window_type (CtkSourceGutter *gutter)
{
	g_return_val_if_fail (CTK_SOURCE_IS_GUTTER (gutter), CTK_TEXT_WINDOW_PRIVATE);

	return gutter->priv->window_type;
}

/**
 * ctk_source_gutter_insert:
 * @gutter: a #CtkSourceGutter.
 * @renderer: a gutter renderer (must inherit from #CtkSourceGutterRenderer).
 * @position: the renderer position.
 *
 * Insert @renderer into the gutter. If @renderer is yet unowned then gutter
 * claims its ownership. Otherwise just increases renderer's reference count.
 * @renderer cannot be already inserted to another gutter.
 *
 * Returns: %TRUE if operation succeeded. Otherwise %FALSE.
 *
 * Since: 3.0
 *
 **/
gboolean
ctk_source_gutter_insert (CtkSourceGutter         *gutter,
                          CtkSourceGutterRenderer *renderer,
                          gint                     position)
{
	Renderer* internal_renderer;

	g_return_val_if_fail (CTK_SOURCE_IS_GUTTER (gutter), FALSE);
	g_return_val_if_fail (CTK_SOURCE_IS_GUTTER_RENDERER (renderer), FALSE);
	g_return_val_if_fail (ctk_source_gutter_renderer_get_view (renderer) == NULL, FALSE);
	g_return_val_if_fail (ctk_source_gutter_renderer_get_window_type (renderer) == CTK_TEXT_WINDOW_PRIVATE, FALSE);

	internal_renderer = renderer_new (gutter, renderer, position);
	append_renderer (gutter, internal_renderer);

	return TRUE;
}

static gboolean
renderer_find (CtkSourceGutter          *gutter,
               CtkSourceGutterRenderer  *renderer,
               Renderer                **ret,
               GList                   **retlist)
{
	GList *list;

	for (list = gutter->priv->renderers; list; list = g_list_next (list))
	{
		*ret = list->data;

		if ((*ret)->renderer == renderer)
		{
			if (retlist)
			{
				*retlist = list;
			}

			return TRUE;
		}
	}

	return FALSE;
}

/**
 * ctk_source_gutter_reorder:
 * @gutter: a #CtkSourceGutterRenderer.
 * @renderer: a #CtkCellRenderer.
 * @position: the new renderer position.
 *
 * Reorders @renderer in @gutter to new @position.
 *
 * Since: 2.8
 */
void
ctk_source_gutter_reorder (CtkSourceGutter         *gutter,
                           CtkSourceGutterRenderer *renderer,
                           gint                     position)
{
	Renderer *ret;
	GList *retlist;

	g_return_if_fail (CTK_SOURCE_IS_GUTTER (gutter));
	g_return_if_fail (CTK_SOURCE_IS_GUTTER_RENDERER (renderer));

	if (renderer_find (gutter, renderer, &ret, &retlist))
	{
		gutter->priv->renderers =
			g_list_delete_link (gutter->priv->renderers,
			                    retlist);

		ret->position = position;
		append_renderer (gutter, ret);
	}
}

/**
 * ctk_source_gutter_remove:
 * @gutter: a #CtkSourceGutter.
 * @renderer: a #CtkSourceGutterRenderer.
 *
 * Removes @renderer from @gutter.
 *
 * Since: 2.8
 */
void
ctk_source_gutter_remove (CtkSourceGutter         *gutter,
                          CtkSourceGutterRenderer *renderer)
{
	Renderer *ret;
	GList *retlist;

	g_return_if_fail (CTK_SOURCE_IS_GUTTER (gutter));
	g_return_if_fail (CTK_SOURCE_IS_GUTTER_RENDERER (renderer));

	if (renderer_find (gutter, renderer, &ret, &retlist))
	{
		gutter->priv->renderers =
			g_list_delete_link (gutter->priv->renderers,
			                    retlist);

		update_gutter_size (gutter);
		renderer_free (ret);
	}
}

/**
 * ctk_source_gutter_queue_draw:
 * @gutter: a #CtkSourceGutter.
 *
 * Invalidates the drawable area of the gutter. You can use this to force a
 * redraw of the gutter if something has changed and needs to be redrawn.
 *
 * Since: 2.8
 */
void
ctk_source_gutter_queue_draw (CtkSourceGutter *gutter)
{
	g_return_if_fail (CTK_SOURCE_IS_GUTTER (gutter));

	do_redraw (gutter);
}

typedef struct _LinesInfo LinesInfo;

struct _LinesInfo
{
	gint total_height;
	gint lines_count;
	GArray *buffer_coords;
	GArray *line_heights;
	GArray *line_numbers;
	CtkTextIter start;
	CtkTextIter end;
};

static LinesInfo *
lines_info_new (void)
{
	LinesInfo *info;

	info = g_slice_new0 (LinesInfo);

	info->buffer_coords = g_array_new (FALSE, FALSE, sizeof (gint));
	info->line_heights = g_array_new (FALSE, FALSE, sizeof (gint));
	info->line_numbers = g_array_new (FALSE, FALSE, sizeof (gint));

	return info;
}

static void
lines_info_free (LinesInfo *info)
{
	if (info != NULL)
	{
		g_array_free (info->buffer_coords, TRUE);
		g_array_free (info->line_heights, TRUE);
		g_array_free (info->line_numbers, TRUE);

		g_slice_free (LinesInfo, info);
	}
}

/* This function is taken and adapted from ctk+/tests/testtext.c */
static LinesInfo *
get_lines_info (CtkTextView *text_view,
		gint         first_y_buffer_coord,
		gint         last_y_buffer_coord)
{
	LinesInfo *info;
	CtkTextIter iter;
	gint last_line_num = -1;

	info = lines_info_new ();

	/* Get iter at first y */
	ctk_text_view_get_line_at_y (text_view, &iter, first_y_buffer_coord, NULL);

	info->start = iter;

	/* For each iter, get its location and add it to the arrays.
	 * Stop when we pass last_y_buffer_coord.
	 */
	while (!ctk_text_iter_is_end (&iter))
	{
		gint y;
		gint height;
		gint line_num;

		ctk_text_view_get_line_yrange (text_view, &iter, &y, &height);

		g_array_append_val (info->buffer_coords, y);
		g_array_append_val (info->line_heights, height);

		info->total_height += height;

		line_num = ctk_text_iter_get_line (&iter);
		g_array_append_val (info->line_numbers, line_num);

		last_line_num = line_num;

		info->lines_count++;

		if (last_y_buffer_coord <= (y + height))
		{
			break;
		}

		ctk_text_iter_forward_line (&iter);
	}

	if (ctk_text_iter_is_end (&iter))
	{
		gint y;
		gint height;
		gint line_num;

		ctk_text_view_get_line_yrange (text_view, &iter, &y, &height);

		line_num = ctk_text_iter_get_line (&iter);

		if (line_num != last_line_num)
		{
			g_array_append_val (info->buffer_coords, y);
			g_array_append_val (info->line_heights, height);

			info->total_height += height;

			g_array_append_val (info->line_numbers, line_num);
			info->lines_count++;
		}
	}

	if (info->lines_count == 0)
	{
		gint y = 0;
		gint n = 0;
		gint height;

		info->lines_count = 1;

		g_array_append_val (info->buffer_coords, y);
		g_array_append_val (info->line_numbers, n);

		ctk_text_view_get_line_yrange (text_view, &iter, &y, &height);
		g_array_append_val (info->line_heights, height);

		info->total_height += height;
	}

	info->end = iter;

	return info;
}

/* Returns %TRUE if @clip is set. @clip contains the area that should be drawn. */
static gboolean
get_clip_rectangle (CtkSourceGutter *gutter,
		    CtkSourceView   *view,
		    cairo_t         *cr,
		    CdkRectangle    *clip)
{
	CdkWindow *window = get_window (gutter);

	if (window == NULL || !ctk_cairo_should_draw_window (cr, window))
	{
		return FALSE;
	}

	ctk_cairo_transform_to_window (cr, CTK_WIDGET (view), window);

	return cdk_cairo_get_clip_rectangle (cr, clip);
}

static void
apply_style (CtkSourceGutter *gutter,
	     CtkSourceView   *view,
	     CtkStyleContext *style_context,
	     cairo_t         *cr)
{
	const gchar *class;
	CdkRGBA fg_color;

	switch (gutter->priv->window_type)
	{
		case CTK_TEXT_WINDOW_TOP:
			class = CTK_STYLE_CLASS_TOP;
			break;

		case CTK_TEXT_WINDOW_RIGHT:
			class = CTK_STYLE_CLASS_RIGHT;
			break;

		case CTK_TEXT_WINDOW_BOTTOM:
			class = CTK_STYLE_CLASS_BOTTOM;
			break;

		case CTK_TEXT_WINDOW_LEFT:
			class = CTK_STYLE_CLASS_LEFT;
			break;

		case CTK_TEXT_WINDOW_PRIVATE:
		case CTK_TEXT_WINDOW_WIDGET:
		case CTK_TEXT_WINDOW_TEXT:
		default:
			g_return_if_reached ();
	}

	/* Apply classes ourselves, since we are in connect_after and so they
	 * are not set by ctk.
	 */
	ctk_style_context_add_class (style_context, class);
	ctk_style_context_get_color (style_context,
	                             ctk_style_context_get_state (style_context),
	                             &fg_color);

	cdk_cairo_set_source_rgba (cr, &fg_color);
}

/* Call ctk_source_gutter_renderer_begin() on each renderer. */
static void
begin_draw (CtkSourceGutter *gutter,
	    CtkTextView     *view,
	    GArray          *renderer_widths,
	    LinesInfo       *info,
	    cairo_t         *cr)
{
	CdkRectangle background_area = { 0 };
	CdkRectangle cell_area;
	GList *l;
	gint renderer_num;

	background_area.x = 0;
	background_area.height = info->total_height;

	ctk_text_view_buffer_to_window_coords (view,
	                                       gutter->priv->window_type,
	                                       0,
	                                       g_array_index (info->buffer_coords, gint, 0),
	                                       NULL,
	                                       &background_area.y);

	cell_area = background_area;

	for (l = gutter->priv->renderers, renderer_num = 0;
	     l != NULL;
	     l = l->next, renderer_num++)
	{
		Renderer *renderer = l->data;
		gint width;
		gint xpad;

		width = g_array_index (renderer_widths, gint, renderer_num);

		if (!ctk_source_gutter_renderer_get_visible (renderer->renderer))
		{
			g_assert_cmpint (width, ==, 0);
			continue;
		}

		ctk_source_gutter_renderer_get_padding (renderer->renderer,
							&xpad,
							NULL);

		background_area.width = width;

		cell_area.width = background_area.width - 2 * xpad;
		cell_area.x = background_area.x + xpad;

		cairo_save (cr);

		cdk_cairo_rectangle (cr, &background_area);
		cairo_clip (cr);

		ctk_source_gutter_renderer_begin (renderer->renderer,
						  cr,
						  &background_area,
						  &cell_area,
						  &info->start,
						  &info->end);

		cairo_restore (cr);

		background_area.x += background_area.width;
	}
}

static void
draw_cells (CtkSourceGutter *gutter,
	    CtkTextView     *view,
	    GArray          *renderer_widths,
	    LinesInfo       *info,
	    cairo_t         *cr)
{
	CtkTextBuffer *buffer;
	CtkTextIter insert_iter;
	gint cur_line;
	CtkTextIter selection_start;
	CtkTextIter selection_end;
	gint selection_start_line = 0;
	gint selection_end_line = 0;
	gboolean has_selection;
	CtkTextIter start;
	gint i;

	buffer = ctk_text_view_get_buffer (view);

	ctk_text_buffer_get_iter_at_mark (buffer,
	                                  &insert_iter,
	                                  ctk_text_buffer_get_insert (buffer));

	cur_line = ctk_text_iter_get_line (&insert_iter);

	has_selection = ctk_text_buffer_get_selection_bounds (buffer,
	                                                      &selection_start,
	                                                      &selection_end);

	if (has_selection)
	{
		selection_start_line = ctk_text_iter_get_line (&selection_start);
		selection_end_line = ctk_text_iter_get_line (&selection_end);
	}

	start = info->start;
	i = 0;

	while (i < info->lines_count)
	{
		CtkTextIter end;
		CdkRectangle background_area;
		CtkSourceGutterRendererState state;
		gint pos;
		gint line_to_paint;
		gint renderer_num;
		GList *l;

		end = start;

		if (!ctk_text_iter_ends_line (&end))
		{
			/*
			 * It turns out that ctk_text_iter_forward_to_line_end
			 * is slower than jumping to the next line in the
			 * btree index and then moving backwards a character.
			 * We don't really care that we might be after the
			 * newline breaking characters, since those are part
			 * of the same line (rather than the next line).
			 */
			if (ctk_text_iter_forward_line (&end))
			{
				ctk_text_iter_backward_char (&end);
			}
		}

		/* Possible improvement: if buffer and window coords have the
		 * same unit, there are probably some possible performance
		 * improvements by avoiding some buffer <-> window coords
		 * conversions.
		 */
		ctk_text_view_buffer_to_window_coords (view,
		                                       gutter->priv->window_type,
		                                       0,
		                                       g_array_index (info->buffer_coords, gint, i),
		                                       NULL,
		                                       &pos);

		line_to_paint = g_array_index (info->line_numbers, gint, i);

		background_area.y = pos;
		background_area.height = g_array_index (info->line_heights, gint, i);
		background_area.x = 0;

		state = CTK_SOURCE_GUTTER_RENDERER_STATE_NORMAL;

		if (line_to_paint == cur_line)
		{
			state |= CTK_SOURCE_GUTTER_RENDERER_STATE_CURSOR;
		}

		if (has_selection &&
		    selection_start_line <= line_to_paint && line_to_paint <= selection_end_line)
		{
			state |= CTK_SOURCE_GUTTER_RENDERER_STATE_SELECTED;
		}

		for (l = gutter->priv->renderers, renderer_num = 0;
		     l != NULL;
		     l = l->next, renderer_num++)
		{
			Renderer *renderer;
			CdkRectangle cell_area;
			gint width;
			gint xpad;
			gint ypad;

			renderer = l->data;
			width = g_array_index (renderer_widths, gint, renderer_num);

			if (!ctk_source_gutter_renderer_get_visible (renderer->renderer))
			{
				g_assert_cmpint (width, ==, 0);
				continue;
			}

			ctk_source_gutter_renderer_get_padding (renderer->renderer,
			                                        &xpad,
			                                        &ypad);

			background_area.width = width;

			cell_area.y = background_area.y + ypad;
			cell_area.height = background_area.height - 2 * ypad;

			cell_area.x = background_area.x + xpad;
			cell_area.width = background_area.width - 2 * xpad;

			if (renderer->prelit >= 0 &&
			    cell_area.y <= renderer->prelit && renderer->prelit <= cell_area.y + cell_area.height)
			{
				state |= CTK_SOURCE_GUTTER_RENDERER_STATE_PRELIT;
			}

			ctk_source_gutter_renderer_query_data (renderer->renderer,
			                                       &start,
			                                       &end,
			                                       state);

			cairo_save (cr);

			cdk_cairo_rectangle (cr, &background_area);

			cairo_clip (cr);

			/* Call render with correct area */
			ctk_source_gutter_renderer_draw (renderer->renderer,
			                                 cr,
			                                 &background_area,
			                                 &cell_area,
			                                 &start,
			                                 &end,
			                                 state);

			cairo_restore (cr);

			background_area.x += background_area.width;
			state &= ~CTK_SOURCE_GUTTER_RENDERER_STATE_PRELIT;
		}

		i++;
		ctk_text_iter_forward_line (&start);
	}
}

static void
end_draw (CtkSourceGutter *gutter)
{
	GList *l;

	for (l = gutter->priv->renderers; l != NULL; l = l->next)
	{
		Renderer *renderer = l->data;

		if (ctk_source_gutter_renderer_get_visible (renderer->renderer))
		{
			ctk_source_gutter_renderer_end (renderer->renderer);
		}
	}
}

void
_ctk_source_gutter_draw (CtkSourceGutter *gutter,
			 CtkSourceView   *view,
			 cairo_t         *cr)
{
	CdkRectangle clip;
	CtkTextView *text_view;
	gint first_y_window_coord;
	gint last_y_window_coord;
	gint first_y_buffer_coord;
	gint last_y_buffer_coord;
	GArray *renderer_widths;
	LinesInfo *info;
	CtkStyleContext *style_context;

	if (!get_clip_rectangle (gutter, view, cr, &clip))
	{
		return;
	}

	gutter->priv->is_drawing = TRUE;

	renderer_widths = g_array_new (FALSE, FALSE, sizeof (gint));
	calculate_gutter_size (gutter, renderer_widths);

	text_view = CTK_TEXT_VIEW (view);

	first_y_window_coord = clip.y;
	last_y_window_coord = first_y_window_coord + clip.height;

	/* get the extents of the line printing */
	ctk_text_view_window_to_buffer_coords (text_view,
	                                       gutter->priv->window_type,
	                                       0,
	                                       first_y_window_coord,
	                                       NULL,
	                                       &first_y_buffer_coord);

	ctk_text_view_window_to_buffer_coords (text_view,
	                                       gutter->priv->window_type,
	                                       0,
	                                       last_y_window_coord,
	                                       NULL,
	                                       &last_y_buffer_coord);

	info = get_lines_info (text_view,
			       first_y_buffer_coord,
			       last_y_buffer_coord);

	style_context = ctk_widget_get_style_context (CTK_WIDGET (view));
	ctk_style_context_save (style_context);
	apply_style (gutter, view, style_context, cr);

	begin_draw (gutter,
		    text_view,
		    renderer_widths,
		    info,
		    cr);

	draw_cells (gutter,
		    text_view,
		    renderer_widths,
		    info,
		    cr);

	/* Allow to call queue_redraw() in ::end. */
	gutter->priv->is_drawing = FALSE;

	end_draw (gutter);

	ctk_style_context_restore (style_context);

	g_array_free (renderer_widths, TRUE);
	lines_info_free (info);
}

static Renderer *
renderer_at_x (CtkSourceGutter *gutter,
               gint             x,
               gint            *start,
               gint            *width)
{
	GList *item;
	gint s;
	gint w;

	update_gutter_size (gutter);

	s = 0;

	for (item = gutter->priv->renderers; item; item = g_list_next (item))
	{
		Renderer *renderer = item->data;
		gint xpad;

		if (!ctk_source_gutter_renderer_get_visible (renderer->renderer))
		{
			continue;
		}

		w = ctk_source_gutter_renderer_get_size (renderer->renderer);

		ctk_source_gutter_renderer_get_padding (renderer->renderer,
		                                        &xpad,
		                                        NULL);

		s += xpad;

		if (w > 0 && x >= s && x < s + w)
		{
			if (width)
			{
				*width = w;
			}

			if (start)
			{
				*start = s;
			}

			return renderer;
		}

		s += w + xpad;
	}

	return NULL;
}

static void
get_renderer_rect (CtkSourceGutter *gutter,
                   Renderer        *renderer,
                   CtkTextIter     *iter,
                   gint             line,
                   CdkRectangle    *rectangle,
                   gint             start)
{
	gint y;
	gint ypad;

	rectangle->x = start;

	ctk_text_view_get_line_yrange (CTK_TEXT_VIEW (gutter->priv->view),
	                               iter,
	                               &y,
	                               &rectangle->height);

	rectangle->width = ctk_source_gutter_renderer_get_size (renderer->renderer);

	ctk_text_view_buffer_to_window_coords (CTK_TEXT_VIEW (gutter->priv->view),
	                                       gutter->priv->window_type,
	                                       0,
	                                       y,
	                                       NULL,
	                                       &rectangle->y);

	ctk_source_gutter_renderer_get_padding (renderer->renderer,
	                                        NULL,
	                                        &ypad);

	rectangle->y += ypad;
	rectangle->height -= 2 * ypad;
}

static gboolean
renderer_query_activatable (CtkSourceGutter *gutter,
                            Renderer        *renderer,
                            CdkEvent        *event,
                            gint             x,
                            gint             y,
                            CtkTextIter     *line_iter,
                            CdkRectangle    *rect,
                            gint             start)
{
	gint y_buf;
	gint yline;
	CtkTextIter iter;
	CdkRectangle r;

	if (!renderer)
	{
		return FALSE;
	}

	ctk_text_view_window_to_buffer_coords (CTK_TEXT_VIEW (gutter->priv->view),
	                                       gutter->priv->window_type,
	                                       x,
	                                       y,
	                                       NULL,
	                                       &y_buf);

	ctk_text_view_get_line_at_y (CTK_TEXT_VIEW (gutter->priv->view),
	                             &iter,
	                             y_buf,
	                             &yline);

	if (yline > y_buf)
	{
		return FALSE;
	}

	get_renderer_rect (gutter, renderer, &iter, yline, &r, start);

	if (line_iter)
	{
		*line_iter = iter;
	}

	if (rect)
	{
		*rect = r;
	}

	if (y < r.y || y > r.y + r.height)
	{
		return FALSE;
	}

	return ctk_source_gutter_renderer_query_activatable (renderer->renderer,
	                                                     &iter,
	                                                     &r,
	                                                     event);
}

static gboolean
redraw_for_window (CtkSourceGutter *gutter,
		   CdkEvent        *event,
		   gboolean         act_on_window,
		   gint             x,
		   gint             y)
{
	Renderer *at_x = NULL;
	gint start = 0;
	GList *item;
	gboolean redraw;

	if (event->any.window != get_window (gutter) && act_on_window)
	{
		return FALSE;
	}

	if (act_on_window)
	{
		at_x = renderer_at_x (gutter, x, &start, NULL);
	}

	redraw = FALSE;

	for (item = gutter->priv->renderers; item; item = g_list_next (item))
	{
		Renderer *renderer = item->data;
		gint prelit = renderer->prelit;

		if (!ctk_source_gutter_renderer_get_visible (renderer->renderer))
		{
			renderer->prelit = -1;
		}
		else
		{
			if (renderer != at_x || !act_on_window)
			{
				renderer->prelit = -1;
			}
			else if (renderer_query_activatable (gutter,
			                                     renderer,
			                                     event,
			                                     x,
			                                     y,
			                                     NULL,
			                                     NULL,
			                                     start))
			{
				renderer->prelit = y;
			}
			else
			{
				renderer->prelit = -1;
			}
		}

		redraw |= (renderer->prelit != prelit);
	}

	if (redraw)
	{
		do_redraw (gutter);
	}

	return FALSE;
}

static gboolean
on_view_motion_notify_event (CtkSourceView    *view,
                             CdkEventMotion   *event,
                             CtkSourceGutter  *gutter)
{
	return redraw_for_window (gutter,
	                          (CdkEvent *)event,
	                          TRUE,
	                          (gint)event->x,
	                          (gint)event->y);
}

static gboolean
on_view_enter_notify_event (CtkSourceView     *view,
                            CdkEventCrossing  *event,
                            CtkSourceGutter   *gutter)
{
	return redraw_for_window (gutter,
	                          (CdkEvent *)event,
	                          TRUE,
	                          (gint)event->x,
	                          (gint)event->y);
}

static gboolean
on_view_leave_notify_event (CtkSourceView     *view,
                            CdkEventCrossing  *event,
                            CtkSourceGutter   *gutter)
{
	return redraw_for_window (gutter,
	                          (CdkEvent *)event,
	                          FALSE,
	                          (gint)event->x,
	                          (gint)event->y);
}

static gboolean
on_view_button_press_event (CtkSourceView    *view,
                            CdkEventButton   *event,
                            CtkSourceGutter  *gutter)
{
	Renderer *renderer;
	CtkTextIter line_iter;
	gint start = -1;
	CdkRectangle rect;

	if (event->window != get_window (gutter))
	{
		return FALSE;
	}

	if (event->type != CDK_BUTTON_PRESS)
	{
		return FALSE;
	}

	/* Check cell renderer */
	renderer = renderer_at_x (gutter, event->x, &start, NULL);

	if (renderer_query_activatable (gutter,
	                                renderer,
	                                (CdkEvent *)event,
	                                (gint)event->x,
	                                (gint)event->y,
	                                &line_iter,
	                                &rect,
	                                start))
	{
		ctk_source_gutter_renderer_activate (renderer->renderer,
		                                     &line_iter,
		                                     &rect,
		                                     (CdkEvent *)event);

		do_redraw (gutter);

		return TRUE;
	}

	return FALSE;
}

static gboolean
on_view_query_tooltip (CtkSourceView   *view,
                       gint             x,
                       gint             y,
                       gboolean         keyboard_mode,
                       CtkTooltip      *tooltip,
                       CtkSourceGutter *gutter)
{
	CtkTextView *text_view = CTK_TEXT_VIEW (view);
	Renderer *renderer;
	gint start = 0;
	gint width = 0;
	gint y_buf;
	gint yline;
	CtkTextIter line_iter;
	CdkRectangle rect;

	if (keyboard_mode)
	{
		return FALSE;
	}

	/* Check cell renderer */
	renderer = renderer_at_x (gutter, x, &start, &width);

	if (!renderer)
	{
		return FALSE;
	}

	ctk_text_view_window_to_buffer_coords (text_view,
	                                       gutter->priv->window_type,
	                                       x, y,
	                                       NULL, &y_buf);

	ctk_text_view_get_line_at_y (CTK_TEXT_VIEW (view),
	                             &line_iter,
	                             y_buf,
	                             &yline);

	if (yline > y_buf)
	{
		return FALSE;
	}

	get_renderer_rect (gutter,
	                   renderer,
	                   &line_iter,
	                   yline,
	                   &rect,
	                   start);

	return ctk_source_gutter_renderer_query_tooltip (renderer->renderer,
	                                                 &line_iter,
	                                                 &rect,
	                                                 x,
	                                                 y,
	                                                 tooltip);
}

static void
on_view_style_updated (CtkSourceView   *view,
                       CtkSourceGutter *gutter)
{
	ctk_source_gutter_queue_draw (gutter);
}

/**
 * ctk_source_gutter_get_renderer_at_pos:
 * @gutter: A #CtkSourceGutter.
 * @x: The x position to get identified.
 * @y: The y position to get identified.
 *
 * Finds the #CtkSourceGutterRenderer at (x, y).
 *
 * Returns: (nullable) (transfer none): the renderer at (x, y) or %NULL.
 */
/* TODO: better document this function. The (x,y) position is different from
 * the position passed to ctk_source_gutter_insert() and
 * ctk_source_gutter_reorder(). The (x,y) coordinate can come from a click
 * event, for example? Is the (x,y) a coordinate of the Gutter's CdkWindow?
 * Where is the (0,0)? And so on.
 * Also, this function doesn't seem to be used.
 */
CtkSourceGutterRenderer *
ctk_source_gutter_get_renderer_at_pos (CtkSourceGutter *gutter,
                                       gint             x,
                                       gint             y)
{
	Renderer *renderer;

	g_return_val_if_fail (CTK_SOURCE_IS_GUTTER (gutter), NULL);

	renderer = renderer_at_x (gutter, x, NULL, NULL);

	if (renderer == NULL)
	{
		return NULL;
	}

	return renderer->renderer;
}
