/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8; coding: utf-8 -*- */
/*
 * This file is part of CtkSourceView
 *
 * Copyright (C) 2013, 2014, 2016 - Sébastien Wilmet <swilmet@gnome.org>
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

/*
 * Custom sizing of the scrolled window containing the CtkTreeView containing
 * the completion proposals. If the CtkTreeView is small enough, the scrolled
 * window returns the natural size of the CtkTreeView. If it exceeds a certain
 * size, the scrolled window returns a smaller size, with the height at a row
 * boundary of the CtkTreeView.
 *
 * The purpose is to have a compact completion window, with a certain size
 * limit.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "ctksourcecompletioncontainer.h"

#define UNREALIZED_WIDTH  350
#define MAX_HEIGHT        180

G_DEFINE_TYPE (CtkSourceCompletionContainer,
	       _ctk_source_completion_container,
	       CTK_TYPE_SCROLLED_WINDOW);

static gint
get_max_width (CtkSourceCompletionContainer *container)
{
	if (ctk_widget_get_realized (CTK_WIDGET (container)))
	{
		CdkDisplay *display;
		CdkMonitor *monitor;
		CtkWidget *toplevel;
		CdkWindow *window;
		CdkRectangle geom;
		gint max_width;
		gint xorigin;

		toplevel = ctk_widget_get_toplevel (CTK_WIDGET (container));
		window = ctk_widget_get_window (toplevel);
		display = ctk_widget_get_display (toplevel);
		monitor = cdk_display_get_monitor_at_window (display, window);
		cdk_monitor_get_geometry (monitor, &geom);

		cdk_window_get_origin (window, &xorigin, NULL);

                /* Just in case xorigin is not the global coordinate limit the max
                 * size to the monitor width.
                 */
		max_width = MIN (geom.width + geom.x - xorigin, geom.width);

		return MAX (max_width, UNREALIZED_WIDTH);
	}

	return UNREALIZED_WIDTH;
}

static void
_ctk_source_completion_container_get_preferred_width (CtkWidget *widget,
						      gint      *min_width,
						      gint      *nat_width)
{
	CtkSourceCompletionContainer *container = CTK_SOURCE_COMPLETION_CONTAINER (widget);
	CtkWidget *child;
	CtkRequisition nat_size;
	gint width;

	child = ctk_bin_get_child (CTK_BIN (container));
	ctk_widget_get_preferred_size (child, NULL, &nat_size);

	width = MIN (nat_size.width, get_max_width (container));

	if (CTK_WIDGET_CLASS (_ctk_source_completion_container_parent_class)->get_preferred_width != NULL)
	{
		gint min_width_parent = 0;

		CTK_WIDGET_CLASS (_ctk_source_completion_container_parent_class)->get_preferred_width (widget,
												       &min_width_parent,
												       NULL);

		width = MAX (width, min_width_parent);
	}

	if (min_width != NULL)
	{
		*min_width = width;
	}

	if (nat_width != NULL)
	{
		*nat_width = width;
	}

	g_return_if_fail (width >= 0);
}

static gint
get_row_height (CtkSourceCompletionContainer *container,
		gint                          tree_view_height)
{
	CtkWidget *tree_view;
	CtkTreeModel *model;
	gint nb_rows;

	/* For another possible implementation, see ctkentrycompletion.c in the
	 * CTK+ source code (the _ctk_entry_completion_resize_popup() function).
	 * It uses ctk_tree_view_column_cell_get_size() for retrieving the
	 * height, plus ctk_widget_style_get() to retrieve the
	 * "vertical-separator" height (note that the vertical separator must
	 * probably be counted one less time than the number or rows).
	 * But using that technique is buggy, it returns a smaller height (it's
	 * maybe a bug in CtkTreeView, or there are other missing parameters).
	 *
	 * Note that the following implementation doesn't take into account
	 * "vertical-separator". If there are some sizing bugs, it's maybe the
	 * source of the problem. (note that on my system the separator size was
	 * 0).
	 */

	tree_view = ctk_bin_get_child (CTK_BIN (container));
	model = ctk_tree_view_get_model (CTK_TREE_VIEW (tree_view));

	if (model == NULL)
	{
		return 0;
	}

	nb_rows = ctk_tree_model_iter_n_children (model, NULL);

	if (nb_rows == 0)
	{
		return 0;
	}

	return tree_view_height / nb_rows;
}

/* Return a height at a row boundary of the CtkTreeView. */
static void
_ctk_source_completion_container_get_preferred_height (CtkWidget *widget,
						       gint	 *min_height,
						       gint	 *nat_height)
{
	CtkSourceCompletionContainer *container = CTK_SOURCE_COMPLETION_CONTAINER (widget);
	CtkWidget *child;
	CtkRequisition nat_size;
	gint height;

	child = ctk_bin_get_child (CTK_BIN (container));
	ctk_widget_get_preferred_size (child, NULL, &nat_size);

	if (nat_size.height <= MAX_HEIGHT)
	{
		height = nat_size.height;
	}
	else
	{
		gint row_height = get_row_height (container, nat_size.height);
		gint n_rows_allowed = row_height != 0 ? MAX_HEIGHT / row_height : 0;

		height = n_rows_allowed * row_height;
	}

	if (CTK_WIDGET_CLASS (_ctk_source_completion_container_parent_class)->get_preferred_height != NULL)
	{
		gint min_height_parent = 0;

		CTK_WIDGET_CLASS (_ctk_source_completion_container_parent_class)->get_preferred_height (widget,
													&min_height_parent,
													NULL);

		height = MAX (height, min_height_parent);
	}

	if (min_height != NULL)
	{
		*min_height = height;
	}

	if (nat_height != NULL)
	{
		*nat_height = height;
	}

	g_return_if_fail (height >= 0);
}

static void
_ctk_source_completion_container_class_init (CtkSourceCompletionContainerClass *klass)
{
	CtkWidgetClass *widget_class = CTK_WIDGET_CLASS (klass);

	widget_class->get_preferred_width = _ctk_source_completion_container_get_preferred_width;
	widget_class->get_preferred_height = _ctk_source_completion_container_get_preferred_height;
}

static void
_ctk_source_completion_container_init (CtkSourceCompletionContainer *container)
{
}

CtkSourceCompletionContainer *
_ctk_source_completion_container_new (void)
{
	return g_object_new (CTK_SOURCE_TYPE_COMPLETION_CONTAINER, NULL);
}
