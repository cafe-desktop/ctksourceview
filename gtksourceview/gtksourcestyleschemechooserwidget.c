/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8; coding: utf-8 -*-
 *
 * This file is part of CtkSourceView
 *
 * Copyright (C) 2014 - Christian Hergert
 * Copyright (C) 2014 - Ignacio Casal Quinteiro
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
 * along with CtkSourceView. If not, see <http://www.gnu.org/licenses/>.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "ctksourcestyleschemechooserwidget.h"
#include "ctksourcestyleschemechooser.h"
#include "ctksourcestylescheme.h"
#include "ctksourcestyleschememanager.h"
#include "ctksourcelanguage.h"
#include "ctksourcelanguagemanager.h"
#include "ctksourcebuffer.h"
#include "ctksourceview.h"

/**
 * SECTION:styleschemechooserwidget
 * @Short_description: A widget for choosing style schemes
 * @Title: CtkSourceStyleSchemeChooserWidget
 * @See_also: #CtkSourceStyleSchemeChooserButton
 *
 * The #CtkSourceStyleSchemeChooserWidget widget lets the user select a
 * style scheme. By default, the chooser presents a predefined list
 * of style schemes.
 *
 * To change the initially selected style scheme,
 * use ctk_source_style_scheme_chooser_set_style_scheme().
 * To get the selected style scheme
 * use ctk_source_style_scheme_chooser_get_style_scheme().
 *
 * Since: 3.16
 */

typedef struct
{
	CtkListBox *list_box;
	CtkSourceStyleScheme *scheme;
} CtkSourceStyleSchemeChooserWidgetPrivate;

static void ctk_source_style_scheme_chooser_widget_style_scheme_chooser_interface_init (CtkSourceStyleSchemeChooserInterface *iface);

G_DEFINE_TYPE_WITH_CODE (CtkSourceStyleSchemeChooserWidget,
                         ctk_source_style_scheme_chooser_widget,
                         CTK_TYPE_BIN,
                         G_ADD_PRIVATE (CtkSourceStyleSchemeChooserWidget)
                         G_IMPLEMENT_INTERFACE (CTK_SOURCE_TYPE_STYLE_SCHEME_CHOOSER,
                                                ctk_source_style_scheme_chooser_widget_style_scheme_chooser_interface_init))

#define GET_PRIV(o) ctk_source_style_scheme_chooser_widget_get_instance_private (o)

enum
{
	PROP_0,
	PROP_STYLE_SCHEME
};

static void
ctk_source_style_scheme_chooser_widget_dispose (GObject *object)
{
	CtkSourceStyleSchemeChooserWidget *widget = CTK_SOURCE_STYLE_SCHEME_CHOOSER_WIDGET (object);
	CtkSourceStyleSchemeChooserWidgetPrivate *priv = GET_PRIV (widget);

	g_clear_object (&priv->scheme);

	G_OBJECT_CLASS (ctk_source_style_scheme_chooser_widget_parent_class)->dispose (object);
}

static void
ctk_source_style_scheme_chooser_widget_get_property (GObject    *object,
                                                     guint       prop_id,
                                                     GValue     *value,
                                                     GParamSpec *pspec)
{
	switch (prop_id)
	{
		case PROP_STYLE_SCHEME:
			g_value_set_object (value,
			                    ctk_source_style_scheme_chooser_get_style_scheme (CTK_SOURCE_STYLE_SCHEME_CHOOSER (object)));
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
	}
}

static void
ctk_source_style_scheme_chooser_widget_set_property (GObject      *object,
                                                     guint         prop_id,
                                                     const GValue *value,
                                                     GParamSpec   *pspec)
{
	switch (prop_id)
	{
		case PROP_STYLE_SCHEME:
			ctk_source_style_scheme_chooser_set_style_scheme (CTK_SOURCE_STYLE_SCHEME_CHOOSER (object),
			                                                  g_value_get_object (value));
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
	}
}

static void
ctk_source_style_scheme_chooser_widget_class_init (CtkSourceStyleSchemeChooserWidgetClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->dispose = ctk_source_style_scheme_chooser_widget_dispose;
	object_class->get_property = ctk_source_style_scheme_chooser_widget_get_property;
	object_class->set_property = ctk_source_style_scheme_chooser_widget_set_property;

	g_object_class_override_property (object_class, PROP_STYLE_SCHEME, "style-scheme");
}

static CtkWidget *
make_row (CtkSourceStyleScheme *scheme,
          CtkSourceLanguage    *language)
{
	CtkWidget *row;
	AtkObject *accessible;
	CtkWidget *event;
	CtkSourceBuffer *buffer;
	CtkWidget *view;
	gchar *text;

	row = ctk_list_box_row_new ();
	accessible = ctk_widget_get_accessible (row);
	atk_object_set_name (accessible,
	                     ctk_source_style_scheme_get_name (scheme));
	ctk_widget_show (row);

	g_object_set_data (G_OBJECT (row), "scheme", scheme);

	event = ctk_event_box_new ();
	ctk_event_box_set_above_child (CTK_EVENT_BOX (event), TRUE);
	ctk_widget_show (event);
	ctk_container_add (CTK_CONTAINER (row), event);

	buffer = ctk_source_buffer_new_with_language (language);
	ctk_source_buffer_set_highlight_matching_brackets (buffer, FALSE);
	ctk_source_buffer_set_style_scheme (buffer, scheme);

	text = g_strdup_printf ("/* %s */\n#include <ctksourceview/ctksource.h>",
	                        ctk_source_style_scheme_get_name (scheme));
	ctk_text_buffer_set_text (CTK_TEXT_BUFFER (buffer), text, -1);
	g_free (text);

	view = g_object_new (CTK_SOURCE_TYPE_VIEW,
	                     "buffer", buffer,
	                     "can-focus", FALSE,
	                     "cursor-visible", FALSE,
	                     "editable", FALSE,
	                     "visible", TRUE,
	                     "show-line-numbers", TRUE,
	                     "right-margin-position", 30,
	                     "show-right-margin", TRUE,
	                     "margin", 2,
	                     NULL);
	ctk_container_add (CTK_CONTAINER (event), view);

	return row;
}

static void
on_row_selected (CtkListBox                        *list_box,
                 CtkListBoxRow                     *row,
                 CtkSourceStyleSchemeChooserWidget *widget)
{
	CtkSourceStyleSchemeChooserWidgetPrivate *priv = GET_PRIV (widget);

	if (row != NULL)
	{
		CtkSourceStyleScheme *scheme;

		scheme = g_object_get_data (G_OBJECT (row), "scheme");

		if (g_set_object (&priv->scheme, scheme))
		{
			g_object_notify (G_OBJECT (widget), "style-scheme");
		}
	}
}

static void
destroy_child_cb (CtkWidget *widget,
		  gpointer   data)
{
	ctk_widget_destroy (widget);
}

static void
ctk_source_style_scheme_chooser_widget_populate (CtkSourceStyleSchemeChooserWidget *widget)
{
	CtkSourceStyleSchemeChooserWidgetPrivate *priv = GET_PRIV (widget);
	CtkSourceLanguageManager *lm;
	CtkSourceLanguage *lang;
	CtkSourceStyleSchemeManager *manager;
	const gchar * const *scheme_ids;
	guint i;
	gboolean row_selected = FALSE;

	g_signal_handlers_block_by_func (priv->list_box, on_row_selected, widget);

	ctk_container_foreach (CTK_CONTAINER (priv->list_box),
	                       destroy_child_cb,
	                       NULL);

	manager = ctk_source_style_scheme_manager_get_default ();
	scheme_ids = ctk_source_style_scheme_manager_get_scheme_ids (manager);

	lm = ctk_source_language_manager_get_default ();
	lang = ctk_source_language_manager_get_language (lm, "c");

	for (i = 0; scheme_ids [i]; i++)
	{
		CtkWidget *row;
		CtkSourceStyleScheme *scheme;

		scheme = ctk_source_style_scheme_manager_get_scheme (manager, scheme_ids [i]);
		row = make_row (scheme, lang);
		ctk_container_add (CTK_CONTAINER (priv->list_box), CTK_WIDGET (row));

		if (scheme == priv->scheme)
		{
			ctk_list_box_select_row (priv->list_box, CTK_LIST_BOX_ROW (row));

			row_selected = TRUE;
		}
	}

	g_signal_handlers_unblock_by_func (priv->list_box, on_row_selected, widget);

	/* The current scheme may have been removed so select the default one */
	if (!row_selected)
	{
		ctk_source_style_scheme_chooser_set_style_scheme (CTK_SOURCE_STYLE_SCHEME_CHOOSER (widget),
		                                                  _ctk_source_style_scheme_get_default ());
	}
}

static void
on_scheme_ids_changed (CtkSourceStyleSchemeManager       *manager,
                       GParamSpec                        *pspec,
                       CtkSourceStyleSchemeChooserWidget *widget)
{
	ctk_source_style_scheme_chooser_widget_populate (widget);
}

static void
ctk_source_style_scheme_chooser_widget_init (CtkSourceStyleSchemeChooserWidget *widget)
{
	CtkSourceStyleSchemeChooserWidgetPrivate *priv = GET_PRIV (widget);
	CtkSourceStyleSchemeManager *manager;

	priv->list_box = CTK_LIST_BOX (ctk_list_box_new ());
	ctk_list_box_set_selection_mode (priv->list_box, CTK_SELECTION_BROWSE);
	ctk_widget_show (CTK_WIDGET (priv->list_box));
	ctk_container_add (CTK_CONTAINER (widget), CTK_WIDGET (priv->list_box));

	manager = ctk_source_style_scheme_manager_get_default ();
	g_signal_connect (manager,
	                  "notify::scheme-ids",
	                  G_CALLBACK (on_scheme_ids_changed),
	                  widget);

	ctk_source_style_scheme_chooser_widget_populate (widget);

	ctk_source_style_scheme_chooser_set_style_scheme (CTK_SOURCE_STYLE_SCHEME_CHOOSER (widget),
	                                                  _ctk_source_style_scheme_get_default ());

	g_signal_connect (priv->list_box,
	                  "row-selected",
	                  G_CALLBACK (on_row_selected),
	                  widget);
}

static CtkSourceStyleScheme *
ctk_source_style_scheme_chooser_widget_get_style_scheme (CtkSourceStyleSchemeChooser *chooser)
{
	CtkSourceStyleSchemeChooserWidget *widget = CTK_SOURCE_STYLE_SCHEME_CHOOSER_WIDGET (chooser);
	CtkSourceStyleSchemeChooserWidgetPrivate *priv = GET_PRIV (widget);

	return priv->scheme;
}

static void
ctk_source_style_scheme_chooser_widget_set_style_scheme (CtkSourceStyleSchemeChooser *chooser,
                                                         CtkSourceStyleScheme        *scheme)
{
	CtkSourceStyleSchemeChooserWidget *widget = CTK_SOURCE_STYLE_SCHEME_CHOOSER_WIDGET (chooser);
	CtkSourceStyleSchemeChooserWidgetPrivate *priv = GET_PRIV (widget);

	if (g_set_object (&priv->scheme, scheme))
	{
		GList *children;
		GList *l;

		children = ctk_container_get_children (CTK_CONTAINER (priv->list_box));

		for (l = children; l != NULL; l = g_list_next (l))
		{
			CtkListBoxRow *row = l->data;
			CtkSourceStyleScheme *cur;

			cur = g_object_get_data (G_OBJECT (row), "scheme");

			if (cur == scheme)
			{
				g_signal_handlers_block_by_func (priv->list_box, on_row_selected, widget);
				ctk_list_box_select_row (priv->list_box, row);
				g_signal_handlers_unblock_by_func (priv->list_box, on_row_selected, widget);
				break;
			}
		}

		g_list_free (children);

		g_object_notify (G_OBJECT (chooser), "style-scheme");
	}
}

static void
ctk_source_style_scheme_chooser_widget_style_scheme_chooser_interface_init (CtkSourceStyleSchemeChooserInterface *iface)
{
	iface->get_style_scheme = ctk_source_style_scheme_chooser_widget_get_style_scheme;
	iface->set_style_scheme = ctk_source_style_scheme_chooser_widget_set_style_scheme;
}

/**
 * ctk_source_style_scheme_chooser_widget_new:
 *
 * Creates a new #CtkSourceStyleSchemeChooserWidget.
 *
 * Returns: a new  #CtkSourceStyleSchemeChooserWidget.
 *
 * Since: 3.16
 */
CtkWidget *
ctk_source_style_scheme_chooser_widget_new (void)
{
	return g_object_new (CTK_SOURCE_TYPE_STYLE_SCHEME_CHOOSER_WIDGET, NULL);
}
