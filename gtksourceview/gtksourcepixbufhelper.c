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

#include "ctksourcepixbufhelper.h"

typedef enum _IconType
{
	ICON_TYPE_PIXBUF,
	ICON_TYPE_GICON,
	ICON_TYPE_NAME
} IconType;

struct _CtkSourcePixbufHelper
{
	GdkPixbuf *cached_pixbuf;
	IconType type;

	GdkPixbuf *pixbuf;
	gchar *icon_name;
	GIcon *gicon;
};

CtkSourcePixbufHelper *
ctk_source_pixbuf_helper_new (void)
{
	return g_slice_new0 (CtkSourcePixbufHelper);
}

void
ctk_source_pixbuf_helper_free (CtkSourcePixbufHelper *helper)
{
	if (helper->pixbuf)
	{
		g_object_unref (helper->pixbuf);
	}

	if (helper->cached_pixbuf)
	{
		g_object_unref (helper->cached_pixbuf);
	}

	if (helper->gicon)
	{
		g_object_unref (helper->gicon);
	}

	g_free (helper->icon_name);

	g_slice_free (CtkSourcePixbufHelper, helper);
}

static void
set_cache (CtkSourcePixbufHelper *helper,
           GdkPixbuf             *pixbuf)
{
	if (helper->cached_pixbuf)
	{
		g_object_unref (helper->cached_pixbuf);
		helper->cached_pixbuf = NULL;
	}

	if (pixbuf)
	{
		helper->cached_pixbuf = pixbuf;
	}
}

static void
clear_cache (CtkSourcePixbufHelper *helper)
{
	set_cache (helper, NULL);
}

void
ctk_source_pixbuf_helper_set_pixbuf (CtkSourcePixbufHelper *helper,
                                     const GdkPixbuf       *pixbuf)
{
	helper->type = ICON_TYPE_PIXBUF;

	if (helper->pixbuf)
	{
		g_object_unref (helper->pixbuf);
		helper->pixbuf = NULL;
	}

	if (pixbuf)
	{
		helper->pixbuf = gdk_pixbuf_copy (pixbuf);
	}

	clear_cache (helper);
}

GdkPixbuf *
ctk_source_pixbuf_helper_get_pixbuf (CtkSourcePixbufHelper *helper)
{
	return helper->pixbuf;
}

void
ctk_source_pixbuf_helper_set_icon_name (CtkSourcePixbufHelper *helper,
                                        const gchar           *icon_name)
{
	helper->type = ICON_TYPE_NAME;

	if (helper->icon_name)
	{
		g_free (helper->icon_name);
	}

	helper->icon_name = g_strdup (icon_name);

	clear_cache (helper);
}

const gchar *
ctk_source_pixbuf_helper_get_icon_name (CtkSourcePixbufHelper *helper)
{
	return helper->icon_name;
}

void
ctk_source_pixbuf_helper_set_gicon (CtkSourcePixbufHelper *helper,
                                    GIcon                 *gicon)
{
	helper->type = ICON_TYPE_GICON;

	if (helper->gicon)
	{
		g_object_unref (helper->gicon);
		helper->gicon = NULL;
	}

	if (gicon)
	{
		helper->gicon = g_object_ref (gicon);
	}

	clear_cache (helper);
}

GIcon *
ctk_source_pixbuf_helper_get_gicon (CtkSourcePixbufHelper *helper)
{
	return helper->gicon;
}

static void
from_pixbuf (CtkSourcePixbufHelper *helper,
             CtkWidget             *widget,
             gint                   size)
{
	if (helper->pixbuf == NULL)
	{
		return;
	}

	if (gdk_pixbuf_get_width (helper->pixbuf) <= size)
	{
		if (!helper->cached_pixbuf)
		{
			set_cache (helper, gdk_pixbuf_copy (helper->pixbuf));
		}

		return;
	}

	/* Make smaller */
	set_cache (helper, gdk_pixbuf_scale_simple (helper->pixbuf,
	                                            size,
	                                            size,
	                                            GDK_INTERP_BILINEAR));
}

static void
from_gicon (CtkSourcePixbufHelper *helper,
            CtkWidget             *widget,
            gint                   size)
{
	CdkScreen *screen;
	CtkIconTheme *icon_theme;
	CtkIconInfo *info;
	CtkIconLookupFlags flags;

	screen = ctk_widget_get_screen (widget);
	icon_theme = ctk_icon_theme_get_for_screen (screen);

	flags = CTK_ICON_LOOKUP_USE_BUILTIN;

	info = ctk_icon_theme_lookup_by_gicon (icon_theme,
	                                       helper->gicon,
	                                       size,
	                                       flags);

	if (info)
	{
		set_cache (helper, ctk_icon_info_load_icon (info, NULL));
	}
}

static void
from_name (CtkSourcePixbufHelper *helper,
           CtkWidget             *widget,
           gint                   size)
{
	CdkScreen *screen;
	CtkIconTheme *icon_theme;
	CtkIconInfo *info;
	CtkIconLookupFlags flags;
	gint scale;

	screen = ctk_widget_get_screen (widget);
	icon_theme = ctk_icon_theme_get_for_screen (screen);

	flags = CTK_ICON_LOOKUP_USE_BUILTIN;
        scale = ctk_widget_get_scale_factor (widget);

	info = ctk_icon_theme_lookup_icon_for_scale (icon_theme,
	                                             helper->icon_name,
	                                             size,
	                                             scale,
	                                             flags);

	if (info)
	{
		GdkPixbuf *pixbuf;

		if (ctk_icon_info_is_symbolic (info))
		{
			CtkStyleContext *context;

			context = ctk_widget_get_style_context (widget);
			pixbuf = ctk_icon_info_load_symbolic_for_context (info, context, NULL, NULL);
		}
		else
		{
			pixbuf = ctk_icon_info_load_icon (info, NULL);
		}

		set_cache (helper, pixbuf);
	}
}

GdkPixbuf *
ctk_source_pixbuf_helper_render (CtkSourcePixbufHelper *helper,
                                 CtkWidget             *widget,
                                 gint                   size)
{
	if (helper->cached_pixbuf &&
	    gdk_pixbuf_get_width (helper->cached_pixbuf) == size)
	{
		return helper->cached_pixbuf;
	}

	switch (helper->type)
	{
		case ICON_TYPE_PIXBUF:
			from_pixbuf (helper, widget, size);
			break;
		case ICON_TYPE_GICON:
			from_gicon (helper, widget, size);
			break;
		case ICON_TYPE_NAME:
			from_name (helper, widget, size);
			break;
		default:
			g_assert_not_reached ();
	}

	return helper->cached_pixbuf;
}

