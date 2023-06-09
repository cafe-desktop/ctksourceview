/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8; coding: utf-8 -*- */
/*
 * This file is part of CtkSourceView
 *
 * Copyright (C) 2007 -2009 Jesús Barbero Rodríguez <chuchiperriman@gmail.com>
 * Copyright (C) 2009 - Jesse van den Kieboom <jessevdk@gnome.org>
 * Copyright (C) 2012 - Sébastien Wilmet <swilmet@gnome.org>
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

/**
 * SECTION:completioninfo
 * @title: CtkSourceCompletionInfo
 * @short_description: Calltips object
 *
 * This object can be used to show a calltip or help for the
 * current completion proposal.
 *
 * The info window has always the same size as the natural size of its child
 * widget, added with ctk_container_add().  If you want a fixed size instead, a
 * possibility is to use a scrolled window, as the following example
 * demonstrates.
 *
 * <example>
 *   <title>Fixed size with a scrolled window.</title>
 *   <programlisting>
 * CtkSourceCompletionInfo *info;
 * CtkWidget *your_widget;
 * CtkWidget *scrolled_window = ctk_scrolled_window_new (NULL, NULL);
 *
 * ctk_widget_set_size_request (scrolled_window, 300, 200);
 * ctk_container_add (CTK_CONTAINER (scrolled_window), your_widget);
 * ctk_container_add (CTK_CONTAINER (info), scrolled_window);
 *   </programlisting>
 * </example>
 *
 * If the calltip is displayed on top of a certain widget, say a #CtkTextView,
 * you should attach the calltip window to the #CtkTextView with
 * ctk_window_set_attached_to().  By doing this, the calltip will be hidden when
 * the #CtkWidget::focus-out-event signal is emitted by the #CtkTextView. You
 * may also be interested by the #CtkTextBuffer:cursor-position property (when
 * its value is modified). If you use the #CtkSourceCompletionInfo through the
 * #CtkSourceCompletion machinery, you don't need to worry about this.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "ctksourcecompletioninfo.h"
#include <glib/gi18n-lib.h>

struct _CtkSourceCompletionInfoPrivate
{
	guint idle_resize;

	CtkWidget *attached_to;
	gulong focus_out_event_handler;

	gint xoffset;

	guint transient_set : 1;
};

G_DEFINE_TYPE_WITH_PRIVATE (CtkSourceCompletionInfo, ctk_source_completion_info, CTK_TYPE_WINDOW);

/* Resize the window */

static gboolean
idle_resize (CtkSourceCompletionInfo *info)
{
	CtkWidget *child = ctk_bin_get_child (CTK_BIN (info));
	CtkRequisition nat_size;
	guint border_width;
	gint window_width;
	gint window_height;
	gint cur_window_width;
	gint cur_window_height;

	info->priv->idle_resize = 0;

	if (child == NULL)
	{
		return G_SOURCE_REMOVE;
	}

	ctk_widget_get_preferred_size (child, NULL, &nat_size);

	border_width = ctk_container_get_border_width (CTK_CONTAINER (info));

	window_width = nat_size.width + 2 * border_width;
	window_height = nat_size.height + 2 * border_width;

	ctk_window_get_size (CTK_WINDOW (info), &cur_window_width, &cur_window_height);

	/* Avoid an infinite loop */
	if (cur_window_width != window_width || cur_window_height != window_height)
	{
		ctk_window_resize (CTK_WINDOW (info),
				   MAX (1, window_width),
				   MAX (1, window_height));
	}

	return G_SOURCE_REMOVE;
}

static void
queue_resize (CtkSourceCompletionInfo *info)
{
	if (info->priv->idle_resize == 0)
	{
		info->priv->idle_resize = g_idle_add ((GSourceFunc)idle_resize, info);
	}
}

static void
ctk_source_completion_info_check_resize (CtkContainer *container)
{
	CtkSourceCompletionInfo *info = CTK_SOURCE_COMPLETION_INFO (container);
	queue_resize (info);

	CTK_CONTAINER_CLASS (ctk_source_completion_info_parent_class)->check_resize (container);
}

/* Geometry management */

static CtkSizeRequestMode
ctk_source_completion_info_get_request_mode (CtkWidget *widget)
{
	return CTK_SIZE_REQUEST_CONSTANT_SIZE;
}

static void
ctk_source_completion_info_get_preferred_width (CtkWidget *widget,
						gint	  *min_width,
						gint	  *nat_width)
{
	CtkWidget *child = ctk_bin_get_child (CTK_BIN (widget));
	gint width = 0;

	if (child != NULL)
	{
		CtkRequisition nat_size;
		ctk_widget_get_preferred_size (child, NULL, &nat_size);
		width = nat_size.width;
	}

	if (min_width != NULL)
	{
		*min_width = width;
	}

	if (nat_width != NULL)
	{
		*nat_width = width;
	}
}

static void
ctk_source_completion_info_get_preferred_height (CtkWidget *widget,
						 gint	   *min_height,
						 gint	   *nat_height)
{
	CtkWidget *child = ctk_bin_get_child (CTK_BIN (widget));
	gint height = 0;

	if (child != NULL)
	{
		CtkRequisition nat_size;
		ctk_widget_get_preferred_size (child, NULL, &nat_size);
		height = nat_size.height;
	}

	if (min_height != NULL)
	{
		*min_height = height;
	}

	if (nat_height != NULL)
	{
		*nat_height = height;
	}
}

/* Init, dispose, finalize, ... */

static gboolean
focus_out_event_cb (CtkSourceCompletionInfo *info)
{
	ctk_widget_hide (CTK_WIDGET (info));
	return FALSE;
}

static void
set_attached_to (CtkSourceCompletionInfo *info,
		 CtkWidget               *attached_to)
{
	if (info->priv->attached_to != NULL)
	{
		g_object_remove_weak_pointer (G_OBJECT (info->priv->attached_to),
					      (gpointer *) &info->priv->attached_to);

		if (info->priv->focus_out_event_handler != 0)
		{
			g_signal_handler_disconnect (info->priv->attached_to,
						     info->priv->focus_out_event_handler);

			info->priv->focus_out_event_handler = 0;
		}
	}

	info->priv->attached_to = attached_to;

	if (attached_to == NULL)
	{
		return;
	}

	g_object_add_weak_pointer (G_OBJECT (attached_to),
				   (gpointer *) &info->priv->attached_to);

	info->priv->focus_out_event_handler =
		g_signal_connect_swapped (attached_to,
					  "focus-out-event",
					  G_CALLBACK (focus_out_event_cb),
					  info);

	info->priv->transient_set = FALSE;
}

static void
update_attached_to (CtkSourceCompletionInfo *info)
{
	set_attached_to (info, ctk_window_get_attached_to (CTK_WINDOW (info)));
}

static void
ctk_source_completion_info_init (CtkSourceCompletionInfo *info)
{
	info->priv = ctk_source_completion_info_get_instance_private (info);

	g_signal_connect (info,
			  "notify::attached-to",
			  G_CALLBACK (update_attached_to),
			  NULL);

	update_attached_to (info);

	/* Tooltip style */
	ctk_window_set_title (CTK_WINDOW (info), _("Completion Info"));
	ctk_widget_set_name (CTK_WIDGET (info), "ctk-tooltip");

	ctk_window_set_type_hint (CTK_WINDOW (info),
	                          CDK_WINDOW_TYPE_HINT_COMBO);

	ctk_container_set_border_width (CTK_CONTAINER (info), 1);
}

static void
ctk_source_completion_info_dispose (GObject *object)
{
	CtkSourceCompletionInfo *info = CTK_SOURCE_COMPLETION_INFO (object);

	if (info->priv->idle_resize != 0)
	{
		g_source_remove (info->priv->idle_resize);
		info->priv->idle_resize = 0;
	}

	set_attached_to (info, NULL);

	G_OBJECT_CLASS (ctk_source_completion_info_parent_class)->dispose (object);
}

static void
ctk_source_completion_info_show (CtkWidget *widget)
{
	CtkSourceCompletionInfo *info = CTK_SOURCE_COMPLETION_INFO (widget);

	if (info->priv->attached_to != NULL && !info->priv->transient_set)
	{
		CtkWidget *toplevel;

		toplevel = ctk_widget_get_toplevel (CTK_WIDGET (info->priv->attached_to));
		if (ctk_widget_is_toplevel (toplevel))
		{
			ctk_window_set_transient_for (CTK_WINDOW (info),
						      CTK_WINDOW (toplevel));
			info->priv->transient_set = TRUE;
		}
	}

	CTK_WIDGET_CLASS (ctk_source_completion_info_parent_class)->show (widget);
}

static gboolean
ctk_source_completion_info_draw (CtkWidget *widget,
                                 cairo_t   *cr)
{
	CTK_WIDGET_CLASS (ctk_source_completion_info_parent_class)->draw (widget, cr);

	ctk_render_frame (ctk_widget_get_style_context (widget),
	                  cr,
	                  0, 0,
	                  ctk_widget_get_allocated_width (widget),
	                  ctk_widget_get_allocated_height (widget));

	return FALSE;
}

static void
ctk_source_completion_info_class_init (CtkSourceCompletionInfoClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	CtkWidgetClass *widget_class = CTK_WIDGET_CLASS (klass);
	CtkContainerClass *container_class = CTK_CONTAINER_CLASS (klass);

	object_class->dispose = ctk_source_completion_info_dispose;

	widget_class->show = ctk_source_completion_info_show;
	widget_class->draw = ctk_source_completion_info_draw;
	widget_class->get_request_mode = ctk_source_completion_info_get_request_mode;
	widget_class->get_preferred_width = ctk_source_completion_info_get_preferred_width;
	widget_class->get_preferred_height = ctk_source_completion_info_get_preferred_height;

	container_class->check_resize = ctk_source_completion_info_check_resize;
}

void
_ctk_source_completion_info_set_xoffset (CtkSourceCompletionInfo *window,
					 gint                     xoffset)
{
	g_return_if_fail (CTK_SOURCE_IS_COMPLETION_INFO (window));

	window->priv->xoffset = xoffset;
}

/* Move to iter */

static void
get_iter_pos (CtkTextView *text_view,
              CtkTextIter *iter,
              gint        *x,
              gint        *y,
              gint        *height)
{
	CdkRectangle location;

	ctk_text_view_get_iter_location (text_view, iter, &location);

	ctk_text_view_buffer_to_window_coords (text_view,
					       CTK_TEXT_WINDOW_WIDGET,
					       location.x,
					       location.y,
					       x,
					       y);

	*height = location.height;
}

static void
move_to_iter (CtkSourceCompletionInfo *window,
              CtkTextView             *view,
              CtkTextIter             *iter)
{
	CdkRectangle position;
	CdkWindow *cdk_window;
	gint x, y;
	gint line_height;

	cdk_window = ctk_widget_get_window (CTK_WIDGET (window));

	if (cdk_window == NULL)
	{
		return;
	}

	get_iter_pos (view, iter, &x, &y, &line_height);

	ctk_widget_translate_coordinates (CTK_WIDGET (view),
	                                  ctk_widget_get_toplevel (CTK_WIDGET (view)),
					  x, y, &x, &y);

	position.x = x;
	position.y = y;
	position.height = line_height;
	position.width = 0;

	cdk_window_move_to_rect (cdk_window,
				 &position,
				 CDK_GRAVITY_SOUTH_WEST,
				 CDK_GRAVITY_NORTH_WEST,
				 CDK_ANCHOR_SLIDE | CDK_ANCHOR_FLIP_Y | CDK_ANCHOR_RESIZE,
				 window->priv->xoffset, 0);
}

static void
move_to_cursor (CtkSourceCompletionInfo *window,
		CtkTextView             *view)
{
	CtkTextBuffer *buffer;
	CtkTextIter insert;

	buffer = ctk_text_view_get_buffer (view);
	ctk_text_buffer_get_iter_at_mark (buffer, &insert, ctk_text_buffer_get_insert (buffer));

	move_to_iter (window, view, &insert);
}

/* Public functions */

/**
 * ctk_source_completion_info_new:
 *
 * Returns: a new CtkSourceCompletionInfo.
 */
CtkSourceCompletionInfo *
ctk_source_completion_info_new (void)
{
	return g_object_new (CTK_SOURCE_TYPE_COMPLETION_INFO,
	                     "type", CTK_WINDOW_POPUP,
			     "border-width", 3,
	                     NULL);
}

/**
 * ctk_source_completion_info_move_to_iter:
 * @info: a #CtkSourceCompletionInfo.
 * @view: a #CtkTextView on which the info window should be positioned.
 * @iter: (nullable): a #CtkTextIter.
 *
 * Moves the #CtkSourceCompletionInfo to @iter. If @iter is %NULL @info is
 * moved to the cursor position. Moving will respect the #CdkGravity setting
 * of the info window and will ensure the line at @iter is not occluded by
 * the window.
 */
void
ctk_source_completion_info_move_to_iter (CtkSourceCompletionInfo *info,
                                         CtkTextView             *view,
                                         CtkTextIter             *iter)
{
	g_return_if_fail (CTK_SOURCE_IS_COMPLETION_INFO (info));
	g_return_if_fail (CTK_IS_TEXT_VIEW (view));

	if (iter == NULL)
	{
		move_to_cursor (info, view);
	}
	else
	{
		move_to_iter (info, view, iter);
	}
}
