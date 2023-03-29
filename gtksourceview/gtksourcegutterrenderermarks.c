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

#include "ctksourcegutterrenderermarks.h"
#include "ctksourceview.h"
#include "ctksourcebuffer.h"
#include "ctksourcemarkattributes.h"
#include "ctksourcemark.h"

#define COMPOSITE_ALPHA                 225

G_DEFINE_TYPE (CtkSourceGutterRendererMarks, ctk_source_gutter_renderer_marks, CTK_SOURCE_TYPE_GUTTER_RENDERER_PIXBUF)

static gint
sort_marks_by_priority (gconstpointer m1,
			gconstpointer m2,
			gpointer data)
{
	CtkSourceMark *mark1 = CTK_SOURCE_MARK (m1);
	CtkSourceMark *mark2 = CTK_SOURCE_MARK (m2);
	CtkSourceView *view = CTK_SOURCE_VIEW (data);
	CtkTextIter iter1, iter2;
	gint line1;
	gint line2;

	ctk_text_buffer_get_iter_at_mark (ctk_text_mark_get_buffer (CTK_TEXT_MARK (mark1)),
	                                  &iter1,
	                                  CTK_TEXT_MARK (mark1));

	ctk_text_buffer_get_iter_at_mark (ctk_text_mark_get_buffer (CTK_TEXT_MARK (mark2)),
	                                  &iter2,
	                                  CTK_TEXT_MARK (mark2));

	line1 = ctk_text_iter_get_line (&iter1);
	line2 = ctk_text_iter_get_line (&iter2);

	if (line1 == line2)
	{
		gint priority1 = -1;
		gint priority2 = -1;

		ctk_source_view_get_mark_attributes (view,
		                                     ctk_source_mark_get_category (mark1),
		                                     &priority1);

		ctk_source_view_get_mark_attributes (view,
		                                     ctk_source_mark_get_category (mark2),
		                                     &priority2);

		return priority1 - priority2;
	}
	else
	{
		return line2 - line1;
	}
}

static int
measure_line_height (CtkSourceView *view)
{
	PangoLayout *layout;
	gint height = 12;

	layout = ctk_widget_create_pango_layout (CTK_WIDGET (view), "QWERTY");

	if (layout)
	{
		pango_layout_get_pixel_size (layout, NULL, &height);
		g_object_unref (layout);
	}

	return height - 2;
}

static CdkPixbuf *
composite_marks (CtkSourceView *view,
                 GSList        *marks,
                 gint           size)
{
	CdkPixbuf *composite;
	gint mark_width;
	gint mark_height;

	/* Draw the mark with higher priority */
	marks = g_slist_sort_with_data (marks, sort_marks_by_priority, view);

	composite = NULL;
	mark_width = 0;
	mark_height = 0;

	/* composite all the pixbufs for the marks present at the line */
	do
	{
		CtkSourceMark *mark;
		CtkSourceMarkAttributes *attrs;
		const CdkPixbuf *pixbuf;

		mark = marks->data;
		attrs = ctk_source_view_get_mark_attributes (view,
		                                             ctk_source_mark_get_category (mark),
		                                             NULL);

		if (attrs == NULL)
		{
			continue;
		}

		pixbuf = ctk_source_mark_attributes_render_icon (attrs,
		                                                 CTK_WIDGET (view),
		                                                 size);

		if (pixbuf != NULL)
		{
			if (composite == NULL)
			{
				composite = cdk_pixbuf_copy (pixbuf);
				mark_width = cdk_pixbuf_get_width (composite);
				mark_height = cdk_pixbuf_get_height (composite);
			}
			else
			{
				gint pixbuf_w;
				gint pixbuf_h;

				pixbuf_w = cdk_pixbuf_get_width (pixbuf);
				pixbuf_h = cdk_pixbuf_get_height (pixbuf);

				cdk_pixbuf_composite (pixbuf,
				                      composite,
				                      0, 0,
				                      mark_width, mark_height,
				                      0, 0,
				                      (gdouble) pixbuf_w / mark_width,
				                      (gdouble) pixbuf_h / mark_height,
				                      CDK_INTERP_BILINEAR,
				                      COMPOSITE_ALPHA);
			}
		}

		marks = g_slist_next (marks);
	}
	while (marks);

	return composite;
}

static void
gutter_renderer_query_data (CtkSourceGutterRenderer      *renderer,
			    CtkTextIter                  *start,
			    CtkTextIter                  *end,
			    CtkSourceGutterRendererState  state)
{
	GSList *marks;
	CdkPixbuf *pixbuf = NULL;
	gint size = 0;
	CtkSourceView *view;
	CtkSourceBuffer *buffer;

	view = CTK_SOURCE_VIEW (ctk_source_gutter_renderer_get_view (renderer));
	buffer = CTK_SOURCE_BUFFER (ctk_text_view_get_buffer (CTK_TEXT_VIEW (view)));

	marks = ctk_source_buffer_get_source_marks_at_iter (buffer,
	                                                    start,
	                                                    NULL);

	if (marks != NULL)
	{
		size = measure_line_height (view);
		pixbuf = composite_marks (view, marks, size);

		g_slist_free (marks);
	}

	g_object_set (G_OBJECT (renderer),
	              "pixbuf", pixbuf,
	              "xpad", 2,
	              "yalign", 0.5,
	              "xalign", 0.5,
	              "alignment-mode", CTK_SOURCE_GUTTER_RENDERER_ALIGNMENT_MODE_FIRST,
	              NULL);

	g_clear_object (&pixbuf);
}

static gboolean
set_tooltip_widget_from_marks (CtkSourceView *view,
                               CtkTooltip    *tooltip,
                               GSList        *marks)
{
	CtkGrid *grid = NULL;
	gint row_num = 0;
	gint icon_size;

	ctk_icon_size_lookup (CTK_ICON_SIZE_MENU, NULL, &icon_size);

	for (; marks; marks = g_slist_next (marks))
	{
		const gchar *category;
		CtkSourceMark *mark;
		CtkSourceMarkAttributes *attrs;
		gchar *text;
		gboolean ismarkup = FALSE;
		CtkWidget *label;
		const CdkPixbuf *pixbuf;

		mark = marks->data;
		category = ctk_source_mark_get_category (mark);

		attrs = ctk_source_view_get_mark_attributes (view, category, NULL);

		if (attrs == NULL)
		{
			continue;
		}

		text = ctk_source_mark_attributes_get_tooltip_markup (attrs, mark);

		if (text == NULL)
		{
			text = ctk_source_mark_attributes_get_tooltip_text (attrs, mark);
		}
		else
		{
			ismarkup = TRUE;
		}

		if (text == NULL)
		{
			continue;
		}

		if (grid == NULL)
		{
			grid = CTK_GRID (ctk_grid_new ());
			ctk_grid_set_column_spacing (grid, 4);
			ctk_widget_show (CTK_WIDGET (grid));
		}

		label = ctk_label_new (NULL);

		if (ismarkup)
		{
			ctk_label_set_markup (CTK_LABEL (label), text);
		}
		else
		{
			ctk_label_set_text (CTK_LABEL (label), text);
		}

		ctk_widget_set_halign (label, CTK_ALIGN_START);
		ctk_widget_set_valign (label, CTK_ALIGN_START);
		ctk_widget_show (label);

		pixbuf = ctk_source_mark_attributes_render_icon (attrs,
		                                                 CTK_WIDGET (view),
		                                                 icon_size);

		if (pixbuf == NULL)
		{
			ctk_grid_attach (grid, label, 0, row_num, 2, 1);
		}
		else
		{
			CtkWidget *image;
			CdkPixbuf *copy;

			/* FIXME why a copy is needed? */
			copy = cdk_pixbuf_copy (pixbuf);
			image = ctk_image_new_from_pixbuf (copy);
			g_object_unref (copy);

			ctk_widget_set_halign (image, CTK_ALIGN_START);
			ctk_widget_set_valign (image, CTK_ALIGN_START);
			ctk_widget_show (image);

			ctk_grid_attach (grid, image, 0, row_num, 1, 1);
			ctk_grid_attach (grid, label, 1, row_num, 1, 1);
		}

		row_num++;

		if (marks->next != NULL)
		{
			CtkWidget *separator;

			separator = ctk_separator_new (CTK_ORIENTATION_HORIZONTAL);

			ctk_widget_show (separator);

			ctk_grid_attach (grid, separator, 0, row_num, 2, 1);
			row_num++;
		}

		g_free (text);
	}

	if (grid == NULL)
	{
		return FALSE;
	}

	ctk_tooltip_set_custom (tooltip, CTK_WIDGET (grid));

	return TRUE;
}

static gboolean
gutter_renderer_query_tooltip (CtkSourceGutterRenderer *renderer,
                               CtkTextIter             *iter,
                               CdkRectangle            *area,
                               gint                     x,
                               gint                     y,
                               CtkTooltip              *tooltip)
{
	GSList *marks;
	CtkSourceView *view;
	CtkSourceBuffer *buffer;
	gboolean ret;

	view = CTK_SOURCE_VIEW (ctk_source_gutter_renderer_get_view (renderer));
	buffer = CTK_SOURCE_BUFFER (ctk_text_view_get_buffer (CTK_TEXT_VIEW (view)));

	marks = ctk_source_buffer_get_source_marks_at_iter (buffer,
	                                                    iter,
	                                                    NULL);

	if (marks != NULL)
	{
		marks = g_slist_sort_with_data (marks,
		                                sort_marks_by_priority,
		                                view);

		marks = g_slist_reverse (marks);

		ret = set_tooltip_widget_from_marks (view, tooltip, marks);

		g_slist_free (marks);

		return ret;
	}

	return FALSE;
}

static gboolean
gutter_renderer_query_activatable (CtkSourceGutterRenderer *renderer,
                                   CtkTextIter             *iter,
                                   CdkRectangle            *area,
                                   CdkEvent                *event)
{
	return TRUE;
}

static void
gutter_renderer_change_view (CtkSourceGutterRenderer *renderer,
                             CtkTextView             *old_view)
{
	CtkSourceView *view;

	view = CTK_SOURCE_VIEW (ctk_source_gutter_renderer_get_view (renderer));

	if (view != NULL)
	{
		ctk_source_gutter_renderer_set_size (renderer,
		                                     measure_line_height (view));
	}

	if (CTK_SOURCE_GUTTER_RENDERER_CLASS (ctk_source_gutter_renderer_marks_parent_class)->change_view != NULL)
	{
		CTK_SOURCE_GUTTER_RENDERER_CLASS (ctk_source_gutter_renderer_marks_parent_class)->change_view (renderer,
													       old_view);
	}
}

static void
ctk_source_gutter_renderer_marks_class_init (CtkSourceGutterRendererMarksClass *klass)
{
	CtkSourceGutterRendererClass *renderer_class = CTK_SOURCE_GUTTER_RENDERER_CLASS (klass);

	renderer_class->query_data = gutter_renderer_query_data;
	renderer_class->query_tooltip = gutter_renderer_query_tooltip;
	renderer_class->query_activatable = gutter_renderer_query_activatable;
	renderer_class->change_view = gutter_renderer_change_view;
}

static void
ctk_source_gutter_renderer_marks_init (CtkSourceGutterRendererMarks *self)
{
}

CtkSourceGutterRenderer *
ctk_source_gutter_renderer_marks_new (void)
{
	return g_object_new (CTK_SOURCE_TYPE_GUTTER_RENDERER_MARKS, NULL);
}
