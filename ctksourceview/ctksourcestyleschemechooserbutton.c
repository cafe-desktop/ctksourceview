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

#include "ctksourcestyleschemechooserbutton.h"
#include <glib/gi18n-lib.h>
#include "ctksourcestyleschemechooser.h"
#include "ctksourcestyleschemechooserwidget.h"
#include "ctksourcestylescheme.h"

/**
 * SECTION:styleschemechooserbutton
 * @Short_description: A button to launch a style scheme selection dialog
 * @Title: CtkSourceStyleSchemeChooserButton
 * @See_also: #CtkSourceStyleSchemeChooserWidget
 *
 * The #CtkSourceStyleSchemeChooserButton is a button which displays
 * the currently selected style scheme and allows to open a style scheme
 * selection dialog to change the style scheme.
 * It is suitable widget for selecting a style scheme in a preference dialog.
 *
 * In #CtkSourceStyleSchemeChooserButton, a #CtkSourceStyleSchemeChooserWidget
 * is used to provide a dialog for selecting style schemes.
 *
 * Since: 3.16
 */

typedef struct
{
	CtkSourceStyleScheme *scheme;

	CtkWidget *dialog;
	CtkSourceStyleSchemeChooserWidget *chooser;
} CtkSourceStyleSchemeChooserButtonPrivate;

static void ctk_source_style_scheme_chooser_button_style_scheme_chooser_interface_init (CtkSourceStyleSchemeChooserInterface *iface);

G_DEFINE_TYPE_WITH_CODE (CtkSourceStyleSchemeChooserButton,
                         ctk_source_style_scheme_chooser_button,
                         CTK_TYPE_BUTTON,
                         G_ADD_PRIVATE (CtkSourceStyleSchemeChooserButton)
                         G_IMPLEMENT_INTERFACE (CTK_SOURCE_TYPE_STYLE_SCHEME_CHOOSER,
                                                ctk_source_style_scheme_chooser_button_style_scheme_chooser_interface_init))

#define GET_PRIV(o) ctk_source_style_scheme_chooser_button_get_instance_private (o)

enum
{
	PROP_0,
	PROP_STYLE_SCHEME
};

static void
ctk_source_style_scheme_chooser_button_dispose (GObject *object)
{
	CtkSourceStyleSchemeChooserButton *button = CTK_SOURCE_STYLE_SCHEME_CHOOSER_BUTTON (object);
	CtkSourceStyleSchemeChooserButtonPrivate *priv = GET_PRIV (button);

	g_clear_object (&priv->scheme);

	G_OBJECT_CLASS (ctk_source_style_scheme_chooser_button_parent_class)->dispose (object);
}

static void
ctk_source_style_scheme_chooser_button_get_property (GObject    *object,
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
ctk_source_style_scheme_chooser_button_set_property (GObject      *object,
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
ctk_source_style_scheme_chooser_button_constructed (GObject *object)
{
	CtkSourceStyleSchemeChooserButton *button = CTK_SOURCE_STYLE_SCHEME_CHOOSER_BUTTON (object);

	G_OBJECT_CLASS (ctk_source_style_scheme_chooser_button_parent_class)->constructed (object);

	ctk_source_style_scheme_chooser_set_style_scheme (CTK_SOURCE_STYLE_SCHEME_CHOOSER (button),
	                                                  _ctk_source_style_scheme_get_default ());
}

static gboolean
dialog_destroy (CtkWidget *widget,
                gpointer   data)
{
	CtkSourceStyleSchemeChooserButton *button = CTK_SOURCE_STYLE_SCHEME_CHOOSER_BUTTON (data);
	CtkSourceStyleSchemeChooserButtonPrivate *priv = GET_PRIV (button);

	priv->dialog = NULL;
	priv->chooser = NULL;

	return FALSE;
}

static void
dialog_response (CtkDialog *dialog,
                 gint       response,
                 gpointer   data)
{
	if (response == CTK_RESPONSE_CANCEL)
	{
		ctk_widget_hide (CTK_WIDGET (dialog));
	}
	else if (response == CTK_RESPONSE_OK)
	{
		CtkSourceStyleSchemeChooserButton *button = CTK_SOURCE_STYLE_SCHEME_CHOOSER_BUTTON (data);
		CtkSourceStyleSchemeChooserButtonPrivate *priv = GET_PRIV (button);
		CtkSourceStyleScheme *scheme;

		scheme = ctk_source_style_scheme_chooser_get_style_scheme (CTK_SOURCE_STYLE_SCHEME_CHOOSER (priv->chooser));

		ctk_widget_hide (CTK_WIDGET (dialog));

		ctk_source_style_scheme_chooser_set_style_scheme (CTK_SOURCE_STYLE_SCHEME_CHOOSER (button),
		                                                  scheme);
	}
}

/* Create the dialog and connects its buttons */
static void
ensure_dialog (CtkSourceStyleSchemeChooserButton *button)
{
	CtkSourceStyleSchemeChooserButtonPrivate *priv = GET_PRIV (button);
	CtkWidget *parent, *dialog, *scrolled_window;
	CtkWidget *content_area;

	if (priv->dialog != NULL)
	{
		return;
	}

	parent = ctk_widget_get_toplevel (CTK_WIDGET (button));

	/* TODO: have a ChooserDialog? */
	priv->dialog = dialog = ctk_dialog_new_with_buttons (_("Select a Style"),
	                                                     CTK_WINDOW (parent),
	                                                     CTK_DIALOG_DESTROY_WITH_PARENT |
	                                                     CTK_DIALOG_USE_HEADER_BAR,
	                                                     _("_Cancel"), CTK_RESPONSE_CANCEL,
	                                                     _("_Select"), CTK_RESPONSE_OK,
	                                                     NULL);
	ctk_dialog_set_default_response (CTK_DIALOG (dialog), CTK_RESPONSE_OK);

	scrolled_window = ctk_scrolled_window_new (NULL, NULL);
	ctk_widget_set_size_request (scrolled_window, 325, 350);
	ctk_widget_show (scrolled_window);
	ctk_widget_set_hexpand (scrolled_window, TRUE);
	ctk_widget_set_vexpand (scrolled_window, TRUE);
	content_area = ctk_dialog_get_content_area (CTK_DIALOG (dialog));
	ctk_container_add (CTK_CONTAINER (content_area), scrolled_window);

	priv->chooser = CTK_SOURCE_STYLE_SCHEME_CHOOSER_WIDGET (ctk_source_style_scheme_chooser_widget_new ());
	ctk_widget_show (CTK_WIDGET (priv->chooser));
	ctk_source_style_scheme_chooser_set_style_scheme (CTK_SOURCE_STYLE_SCHEME_CHOOSER (priv->chooser),
	                                                  priv->scheme);

	ctk_container_add (CTK_CONTAINER (scrolled_window), CTK_WIDGET (priv->chooser));

	if (ctk_widget_is_toplevel (parent) && CTK_IS_WINDOW (parent))
	{
		if (CTK_WINDOW (parent) != ctk_window_get_transient_for (CTK_WINDOW (dialog)))
		{
			ctk_window_set_transient_for (CTK_WINDOW (dialog), CTK_WINDOW (parent));
		}

		ctk_window_set_modal (CTK_WINDOW (dialog),
		                      ctk_window_get_modal (CTK_WINDOW (parent)));
	}

	g_signal_connect (dialog, "response",
	                  G_CALLBACK (dialog_response), button);
	g_signal_connect (dialog, "destroy",
	                  G_CALLBACK (dialog_destroy), button);
}

static void
ctk_source_style_scheme_chooser_button_clicked (CtkButton *button)
{
	CtkSourceStyleSchemeChooserButton *cbutton = CTK_SOURCE_STYLE_SCHEME_CHOOSER_BUTTON (button);
	CtkSourceStyleSchemeChooserButtonPrivate *priv = GET_PRIV (cbutton);

	ensure_dialog (cbutton);

	ctk_source_style_scheme_chooser_set_style_scheme (CTK_SOURCE_STYLE_SCHEME_CHOOSER (priv->chooser),
	                                                  priv->scheme);

	ctk_window_present (CTK_WINDOW (priv->dialog));
}

static void
ctk_source_style_scheme_chooser_button_class_init (CtkSourceStyleSchemeChooserButtonClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	CtkButtonClass *button_class = CTK_BUTTON_CLASS (klass);

	object_class->dispose = ctk_source_style_scheme_chooser_button_dispose;
	object_class->get_property = ctk_source_style_scheme_chooser_button_get_property;
	object_class->set_property = ctk_source_style_scheme_chooser_button_set_property;
	object_class->constructed = ctk_source_style_scheme_chooser_button_constructed;

	button_class->clicked = ctk_source_style_scheme_chooser_button_clicked;

	g_object_class_override_property (object_class, PROP_STYLE_SCHEME, "style-scheme");
}

static void
ctk_source_style_scheme_chooser_button_init (CtkSourceStyleSchemeChooserButton *button)
{
}

static CtkSourceStyleScheme *
ctk_source_style_scheme_chooser_button_get_style_scheme (CtkSourceStyleSchemeChooser *chooser)
{
	CtkSourceStyleSchemeChooserButton *button = CTK_SOURCE_STYLE_SCHEME_CHOOSER_BUTTON (chooser);
	CtkSourceStyleSchemeChooserButtonPrivate *priv = GET_PRIV (button);

	return priv->scheme;
}

static void
ctk_source_style_scheme_chooser_button_update_label (CtkSourceStyleSchemeChooserButton *button)
{
	CtkSourceStyleSchemeChooserButtonPrivate *priv = GET_PRIV (button);
	const gchar *label;

	label = priv->scheme != NULL ? ctk_source_style_scheme_get_name (priv->scheme) : NULL;
	ctk_button_set_label (CTK_BUTTON (button), label);
}

static void
ctk_source_style_scheme_chooser_button_set_style_scheme (CtkSourceStyleSchemeChooser *chooser,
                                                         CtkSourceStyleScheme        *scheme)
{
	CtkSourceStyleSchemeChooserButton *button = CTK_SOURCE_STYLE_SCHEME_CHOOSER_BUTTON (chooser);
	CtkSourceStyleSchemeChooserButtonPrivate *priv = GET_PRIV (button);

	if (g_set_object (&priv->scheme, scheme))
	{
		ctk_source_style_scheme_chooser_button_update_label (button);

		g_object_notify (G_OBJECT (button), "style-scheme");
	}
}

static void
ctk_source_style_scheme_chooser_button_style_scheme_chooser_interface_init (CtkSourceStyleSchemeChooserInterface *iface)
{
	iface->get_style_scheme = ctk_source_style_scheme_chooser_button_get_style_scheme;
	iface->set_style_scheme = ctk_source_style_scheme_chooser_button_set_style_scheme;
}

/**
 * ctk_source_style_scheme_chooser_button_new:
 *
 * Creates a new #CtkSourceStyleSchemeChooserButton.
 *
 * Returns: a new #CtkSourceStyleSchemeChooserButton.
 *
 * Since: 3.16
 */
CtkWidget *
ctk_source_style_scheme_chooser_button_new (void)
{
	return g_object_new (CTK_SOURCE_TYPE_STYLE_SCHEME_CHOOSER_BUTTON, NULL);
}
