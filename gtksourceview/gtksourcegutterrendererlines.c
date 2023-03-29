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

#include "ctksourcegutterrendererlines.h"
#include "ctksourceutils-private.h"
#include "ctksourceview.h"

struct _CtkSourceGutterRendererLinesPrivate
{
	gint num_line_digits;
	gint prev_line_num;
	guint cursor_visible : 1;
};

G_DEFINE_TYPE_WITH_PRIVATE (CtkSourceGutterRendererLines, ctk_source_gutter_renderer_lines, CTK_SOURCE_TYPE_GUTTER_RENDERER_TEXT)

static CtkTextBuffer *
get_buffer (CtkSourceGutterRendererLines *renderer)
{
	CtkTextView *view;

	view = ctk_source_gutter_renderer_get_view (CTK_SOURCE_GUTTER_RENDERER (renderer));

	return view != NULL ? ctk_text_view_get_buffer (view) : NULL;
}

static inline gint
count_num_digits (gint num_lines)
{
	if (num_lines < 100)
	{
		return 2;
	}
	else if (num_lines < 1000)
	{
		return 3;
	}
	else if (num_lines < 10000)
	{
		return 4;
	}
	else if (num_lines < 100000)
	{
		return 5;
	}
	else if (num_lines < 1000000)
	{
		return 6;
	}
	else
	{
		return 10;
	}
}

static void
recalculate_size (CtkSourceGutterRendererLines *renderer)
{
	gint num_lines;
	gint num_digits = 0;
	CtkTextBuffer *buffer;

	buffer = get_buffer (renderer);

	num_lines = ctk_text_buffer_get_line_count (buffer);

	num_digits = count_num_digits (num_lines);

	if (num_digits != renderer->priv->num_line_digits)
	{
		gchar markup[24];
		gint size;

		renderer->priv->num_line_digits = num_digits;

		num_lines = MAX (num_lines, 99);

		g_snprintf (markup, sizeof markup, "<b>%d</b>", num_lines);
		ctk_source_gutter_renderer_text_measure_markup (CTK_SOURCE_GUTTER_RENDERER_TEXT (renderer),
		                                                markup,
		                                                &size,
		                                                NULL);

		ctk_source_gutter_renderer_set_size (CTK_SOURCE_GUTTER_RENDERER (renderer),
		                                     size);
	}
}

static void
on_buffer_changed (CtkSourceBuffer              *buffer,
                   CtkSourceGutterRendererLines *renderer)
{
	recalculate_size (renderer);
}

static void
gutter_renderer_change_buffer (CtkSourceGutterRenderer *renderer,
                               CtkTextBuffer           *old_buffer)
{
	CtkSourceGutterRendererLines *lines = CTK_SOURCE_GUTTER_RENDERER_LINES (renderer);
	CtkTextBuffer *buffer;

	if (old_buffer != NULL)
	{
		g_signal_handlers_disconnect_by_func (old_buffer,
						      on_buffer_changed,
						      lines);
	}

	buffer = get_buffer (lines);

	if (buffer != NULL)
	{
		g_signal_connect_object (buffer,
					 "changed",
					 G_CALLBACK (on_buffer_changed),
					 lines,
					 0);

		recalculate_size (lines);
	}

	lines->priv->prev_line_num = 0;

	if (CTK_SOURCE_GUTTER_RENDERER_CLASS (ctk_source_gutter_renderer_lines_parent_class)->change_buffer != NULL)
	{
		CTK_SOURCE_GUTTER_RENDERER_CLASS (ctk_source_gutter_renderer_lines_parent_class)->change_buffer (renderer,
														 old_buffer);
	}
}

static void
on_view_style_updated (CtkTextView                  *view,
		       CtkSourceGutterRendererLines *renderer)
{
	/* Force to recalculate the size. */
	renderer->priv->num_line_digits = -1;
	recalculate_size (renderer);
}

static void
on_view_notify_cursor_visible (CtkTextView                  *view,
                               GParamSpec                   *pspec,
                               CtkSourceGutterRendererLines *renderer)
{
	renderer->priv->cursor_visible = ctk_text_view_get_cursor_visible (view);
}

static void
gutter_renderer_change_view (CtkSourceGutterRenderer *renderer,
			     CtkTextView             *old_view)
{
	CtkTextView *new_view;

	if (old_view != NULL)
	{
		g_signal_handlers_disconnect_by_func (old_view,
						      on_view_style_updated,
						      renderer);
		g_signal_handlers_disconnect_by_func (old_view,
						      on_view_notify_cursor_visible,
						      renderer);
	}

	new_view = ctk_source_gutter_renderer_get_view (renderer);

	if (new_view != NULL)
	{
		g_signal_connect_object (new_view,
					 "style-updated",
					 G_CALLBACK (on_view_style_updated),
					 renderer,
					 0);

		g_signal_connect_object (new_view,
					 "notify::cursor-visible",
					 G_CALLBACK (on_view_notify_cursor_visible),
					 renderer,
					 0);

		CTK_SOURCE_GUTTER_RENDERER_LINES (renderer)->priv->cursor_visible = ctk_text_view_get_cursor_visible (new_view);
	}

	if (CTK_SOURCE_GUTTER_RENDERER_CLASS (ctk_source_gutter_renderer_lines_parent_class)->change_view != NULL)
	{
		CTK_SOURCE_GUTTER_RENDERER_CLASS (ctk_source_gutter_renderer_lines_parent_class)->change_view (renderer,
													       old_view);
	}
}

static void
gutter_renderer_query_data (CtkSourceGutterRenderer      *renderer,
                            CtkTextIter                  *start,
                            CtkTextIter                  *end,
                            CtkSourceGutterRendererState  state)
{
	CtkSourceGutterRendererLines *lines = CTK_SOURCE_GUTTER_RENDERER_LINES (renderer);
	gchar text[24];
	const gchar *textptr = text;
	gint line;
	gint len;
	gboolean current_line;

	line = ctk_text_iter_get_line (start) + 1;

	current_line = (state & CTK_SOURCE_GUTTER_RENDERER_STATE_CURSOR) &&
	               lines->priv->cursor_visible;

	if G_LIKELY (!current_line)
	{
		len = _ctk_source_utils_int_to_string (line, &textptr);
	}
	else
	{
		len = g_snprintf (text, sizeof text, "<b>%d</b>", line);
	}

	ctk_source_gutter_renderer_text_set_markup (CTK_SOURCE_GUTTER_RENDERER_TEXT (renderer),
	                                            textptr,
	                                            len);
}

static gint
get_last_visible_line_number (CtkSourceGutterRendererLines *lines)
{
	CtkTextView *view;
	GdkRectangle visible_rect;
	CtkTextIter iter;

	view = ctk_source_gutter_renderer_get_view (CTK_SOURCE_GUTTER_RENDERER (lines));

	ctk_text_view_get_visible_rect (view, &visible_rect);

	ctk_text_view_get_line_at_y (view,
				     &iter,
				     visible_rect.y + visible_rect.height,
				     NULL);

	ctk_text_iter_forward_line (&iter);

	return ctk_text_iter_get_line (&iter);
}

static void
gutter_renderer_end (CtkSourceGutterRenderer *renderer)
{
	CtkSourceGutterRendererLines *lines = CTK_SOURCE_GUTTER_RENDERER_LINES (renderer);
	CtkTextBuffer *buffer = get_buffer (lines);

	if (buffer != NULL)
	{
		gint line_num = get_last_visible_line_number (lines);

		/* When the text is modified in a CtkTextBuffer, CtkTextView tries to
		 * redraw the smallest required region. But the information displayed in
		 * the gutter may become invalid in a bigger region.
		 * See https://bugzilla.gnome.org/show_bug.cgi?id=732418 for an example
		 * where line numbers are not updated correctly when splitting a wrapped
		 * line.
		 * The performances should not be a big problem here. Correctness is
		 * more important than performances. It just triggers a second
		 * draw.
		 * The queue_draw() is called in gutter_renderer_end(), because
		 * the first draw is anyway needed to avoid flickering (if the
		 * first draw is not done, there will be a white region in the
		 * gutter during one frame).
		 * Another solution that has better performances is to compare
		 * the total number of lines in the buffer, instead of the last
		 * visible line. But it has the drawback that the gutter is
		 * continuously redrawn during file loading.
		 *
		 * FIXME A better solution would be to add a vfunc in the
		 * GutterRenderer so that the Gutter can ask each renderer for
		 * the invalidation region, before drawing. So that only one
		 * draw is needed, and the solution would be more generic (if
		 * other renderers also need a different invalidation region
		 * than the CtkTextView). But the GutterRendererClass doesn't
		 * have padding for future expansion, so it must wait for
		 * CtkSourceView 4.
		 */
		if (lines->priv->prev_line_num != line_num)
		{
			lines->priv->prev_line_num = line_num;
			ctk_source_gutter_renderer_queue_draw (renderer);
		}
	}

	if (CTK_SOURCE_GUTTER_RENDERER_CLASS (ctk_source_gutter_renderer_lines_parent_class)->end != NULL)
	{
		CTK_SOURCE_GUTTER_RENDERER_CLASS (ctk_source_gutter_renderer_lines_parent_class)->end (renderer);
	}
}

static void
extend_selection_to_line (CtkSourceGutterRendererLines *renderer,
                          CtkTextIter                  *line_start)
{
	CtkTextIter start;
	CtkTextIter end;
	CtkTextIter line_end;
	CtkTextBuffer *buffer;

	buffer = get_buffer (renderer);

	ctk_text_buffer_get_selection_bounds (buffer,
	                                      &start,
	                                      &end);

	line_end = *line_start;

	if (!ctk_text_iter_ends_line (&line_end))
	{
		ctk_text_iter_forward_to_line_end (&line_end);
	}

	if (ctk_text_iter_compare (&start, line_start) < 0)
	{
		ctk_text_buffer_select_range (buffer,
		                              &start,
		                              &line_end);
	}
	else if (ctk_text_iter_compare (&end, &line_end) < 0)
	{
		/* if the selection is in this line, extend
		 * the selection to the whole line */
		ctk_text_buffer_select_range (buffer,
		                              &line_end,
		                              line_start);
	}
	else
	{
		ctk_text_buffer_select_range (buffer,
		                              &end,
		                              line_start);
	}
}

static void
select_line (CtkSourceGutterRendererLines *renderer,
             CtkTextIter                  *line_start)
{
	CtkTextIter iter;
	CtkTextBuffer *buffer;

	buffer = get_buffer (renderer);

	iter = *line_start;

	if (!ctk_text_iter_ends_line (&iter))
	{
		ctk_text_iter_forward_to_line_end (&iter);
	}

	/* Select the line, put the cursor at the end of the line */
	ctk_text_buffer_select_range (buffer, &iter, line_start);
}

static void
gutter_renderer_activate (CtkSourceGutterRenderer *renderer,
                          CtkTextIter             *iter,
                          GdkRectangle            *rect,
                          GdkEvent                *event)
{
	CtkSourceGutterRendererLines *lines;

	lines = CTK_SOURCE_GUTTER_RENDERER_LINES (renderer);

	if (event->type == CDK_BUTTON_PRESS && (event->button.button == 1))
	{
		CtkTextBuffer *buffer;

		buffer = get_buffer (lines);

		if ((event->button.state & CDK_CONTROL_MASK) != 0)
		{
			/* Single click + Ctrl -> select the line */
			select_line (lines, iter);
		}
		else if ((event->button.state & CDK_SHIFT_MASK) != 0)
		{
			/* Single click + Shift -> extended current
			   selection to include the clicked line */
			extend_selection_to_line (lines, iter);
		}
		else
		{
			ctk_text_buffer_place_cursor (buffer, iter);
		}
	}
	else if (event->type == CDK_2BUTTON_PRESS && (event->button.button == 1))
	{
		select_line (lines, iter);
	}
}

static gboolean
gutter_renderer_query_activatable (CtkSourceGutterRenderer *renderer,
                                   CtkTextIter             *iter,
                                   GdkRectangle            *area,
                                   GdkEvent                *event)
{
	return get_buffer (CTK_SOURCE_GUTTER_RENDERER_LINES (renderer)) != NULL;
}

static void
ctk_source_gutter_renderer_lines_class_init (CtkSourceGutterRendererLinesClass *klass)
{
	CtkSourceGutterRendererClass *renderer_class = CTK_SOURCE_GUTTER_RENDERER_CLASS (klass);

	renderer_class->query_data = gutter_renderer_query_data;
	renderer_class->end = gutter_renderer_end;
	renderer_class->query_activatable = gutter_renderer_query_activatable;
	renderer_class->activate = gutter_renderer_activate;
	renderer_class->change_buffer = gutter_renderer_change_buffer;
	renderer_class->change_view = gutter_renderer_change_view;
}

static void
ctk_source_gutter_renderer_lines_init (CtkSourceGutterRendererLines *self)
{
	self->priv = ctk_source_gutter_renderer_lines_get_instance_private (self);
}

CtkSourceGutterRenderer *
ctk_source_gutter_renderer_lines_new (void)
{
	return g_object_new (CTK_SOURCE_TYPE_GUTTER_RENDERER_LINES, NULL);
}
