/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8; coding: utf-8 -*-
 *
 * This file is part of CtkSourceView
 *
 * Copyright (C) 2009 - Jesse van den Kieboom <jessevdk@gnome.org>
 * Copyright (C) 2016 - Sébastien Wilmet <swilmet@gnome.org>
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
 * SECTION:completionitem
 * @title: CtkSourceCompletionItem
 * @short_description: Simple implementation of CtkSourceCompletionProposal
 *
 * The #CtkSourceCompletionItem class is a simple implementation of the
 * #CtkSourceCompletionProposal interface.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "ctksourcecompletionitem.h"
#include "ctksourcecompletionproposal.h"

struct _CtkSourceCompletionItemPrivate
{
	gchar *label;
	gchar *markup;
	gchar *text;
	GdkPixbuf *icon;
	gchar *icon_name;
	GIcon *gicon;
	gchar *info;
};

enum
{
	PROP_0,
	PROP_LABEL,
	PROP_MARKUP,
	PROP_TEXT,
	PROP_ICON,
	PROP_ICON_NAME,
	PROP_GICON,
	PROP_INFO
};

static void ctk_source_completion_proposal_iface_init (gpointer g_iface, gpointer iface_data);

G_DEFINE_TYPE_WITH_CODE (CtkSourceCompletionItem,
			 ctk_source_completion_item,
			 G_TYPE_OBJECT,
			 G_ADD_PRIVATE (CtkSourceCompletionItem)
			 G_IMPLEMENT_INTERFACE (CTK_SOURCE_TYPE_COMPLETION_PROPOSAL,
			 			ctk_source_completion_proposal_iface_init))

static gchar *
ctk_source_completion_proposal_get_label_impl (CtkSourceCompletionProposal *proposal)
{
	return g_strdup (CTK_SOURCE_COMPLETION_ITEM (proposal)->priv->label);
}

static gchar *
ctk_source_completion_proposal_get_markup_impl (CtkSourceCompletionProposal *proposal)
{
	return g_strdup (CTK_SOURCE_COMPLETION_ITEM (proposal)->priv->markup);
}

static gchar *
ctk_source_completion_proposal_get_text_impl (CtkSourceCompletionProposal *proposal)
{
	return g_strdup (CTK_SOURCE_COMPLETION_ITEM (proposal)->priv->text);
}

static GdkPixbuf *
ctk_source_completion_proposal_get_icon_impl (CtkSourceCompletionProposal *proposal)
{
	return CTK_SOURCE_COMPLETION_ITEM (proposal)->priv->icon;
}

static const gchar *
ctk_source_completion_proposal_get_icon_name_impl (CtkSourceCompletionProposal *proposal)
{
	return CTK_SOURCE_COMPLETION_ITEM (proposal)->priv->icon_name;
}

static GIcon *
ctk_source_completion_proposal_get_gicon_impl (CtkSourceCompletionProposal *proposal)
{
	return CTK_SOURCE_COMPLETION_ITEM (proposal)->priv->gicon;
}

static gchar *
ctk_source_completion_proposal_get_info_impl (CtkSourceCompletionProposal *proposal)
{
	return g_strdup (CTK_SOURCE_COMPLETION_ITEM (proposal)->priv->info);
}

static void
ctk_source_completion_proposal_iface_init (gpointer g_iface,
					   gpointer iface_data)
{
	CtkSourceCompletionProposalIface *iface = g_iface;

	/* Interface data getter implementations */
	iface->get_label = ctk_source_completion_proposal_get_label_impl;
	iface->get_markup = ctk_source_completion_proposal_get_markup_impl;
	iface->get_text = ctk_source_completion_proposal_get_text_impl;
	iface->get_icon = ctk_source_completion_proposal_get_icon_impl;
	iface->get_icon_name = ctk_source_completion_proposal_get_icon_name_impl;
	iface->get_gicon = ctk_source_completion_proposal_get_gicon_impl;
	iface->get_info = ctk_source_completion_proposal_get_info_impl;
}

static void
ctk_source_completion_item_dispose (GObject *object)
{
	CtkSourceCompletionItem *item = CTK_SOURCE_COMPLETION_ITEM (object);

	g_clear_object (&item->priv->icon);
	g_clear_object (&item->priv->gicon);

	G_OBJECT_CLASS (ctk_source_completion_item_parent_class)->dispose (object);
}

static void
ctk_source_completion_item_finalize (GObject *object)
{
	CtkSourceCompletionItem *item = CTK_SOURCE_COMPLETION_ITEM (object);

	g_free (item->priv->label);
	g_free (item->priv->markup);
	g_free (item->priv->text);
	g_free (item->priv->icon_name);
	g_free (item->priv->info);

	G_OBJECT_CLASS (ctk_source_completion_item_parent_class)->finalize (object);
}

static void
ctk_source_completion_item_get_property (GObject    *object,
					 guint       prop_id,
					 GValue     *value,
					 GParamSpec *pspec)
{
	CtkSourceCompletionItem *item;

	g_return_if_fail (CTK_SOURCE_IS_COMPLETION_ITEM (object));

	item = CTK_SOURCE_COMPLETION_ITEM (object);

	switch (prop_id)
	{
		case PROP_LABEL:
			g_value_set_string (value, item->priv->label);
			break;

		case PROP_MARKUP:
			g_value_set_string (value, item->priv->markup);
			break;

		case PROP_TEXT:
			g_value_set_string (value, item->priv->text);
			break;

		case PROP_ICON:
			g_value_set_object (value, item->priv->icon);
			break;

		case PROP_ICON_NAME:
			g_value_set_string (value, item->priv->icon_name);
			break;

		case PROP_GICON:
			g_value_set_object (value, item->priv->gicon);
			break;

		case PROP_INFO:
			g_value_set_string (value, item->priv->info);
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
emit_changed (CtkSourceCompletionItem *item)
{
	ctk_source_completion_proposal_changed (CTK_SOURCE_COMPLETION_PROPOSAL (item));
}

static void
ctk_source_completion_item_set_property (GObject      *object,
					 guint         prop_id,
					 const GValue *value,
					 GParamSpec   *pspec)
{
	CtkSourceCompletionItem *item;

	g_return_if_fail (CTK_SOURCE_IS_COMPLETION_ITEM (object));

	item = CTK_SOURCE_COMPLETION_ITEM (object);

	switch (prop_id)
	{
		case PROP_LABEL:
			ctk_source_completion_item_set_label (item, g_value_get_string (value));
			break;

		case PROP_MARKUP:
			ctk_source_completion_item_set_markup (item, g_value_get_string (value));
			break;

		case PROP_TEXT:
			ctk_source_completion_item_set_text (item, g_value_get_string (value));
			break;

		case PROP_ICON:
			ctk_source_completion_item_set_icon (item, g_value_get_object (value));
			break;

		case PROP_ICON_NAME:
			ctk_source_completion_item_set_icon_name (item, g_value_get_string (value));
			break;

		case PROP_GICON:
			ctk_source_completion_item_set_gicon (item, g_value_get_object (value));
			break;

		case PROP_INFO:
			ctk_source_completion_item_set_info (item, g_value_get_string (value));
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
ctk_source_completion_item_class_init (CtkSourceCompletionItemClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->dispose = ctk_source_completion_item_dispose;
	object_class->finalize = ctk_source_completion_item_finalize;
	object_class->get_property = ctk_source_completion_item_get_property;
	object_class->set_property = ctk_source_completion_item_set_property;

	/**
	 * CtkSourceCompletionItem:label:
	 *
	 * Label to be shown for this proposal.
	 */
	g_object_class_install_property (object_class,
					 PROP_LABEL,
					 g_param_spec_string ("label",
							      "Label",
							      "",
							      NULL,
							      G_PARAM_READWRITE |
							      G_PARAM_STATIC_STRINGS));

	/**
	 * CtkSourceCompletionItem:markup:
	 *
	 * Label with markup to be shown for this proposal.
	 */
	g_object_class_install_property (object_class,
					 PROP_MARKUP,
					 g_param_spec_string ("markup",
							      "Markup",
							      "",
							      NULL,
							      G_PARAM_READWRITE |
							      G_PARAM_STATIC_STRINGS));

	/**
	 * CtkSourceCompletionItem:text:
	 *
	 * Proposal text.
	 */
	g_object_class_install_property (object_class,
					 PROP_TEXT,
					 g_param_spec_string ("text",
							      "Text",
							      "",
							      NULL,
							      G_PARAM_READWRITE |
							      G_PARAM_STATIC_STRINGS));

	/**
	 * CtkSourceCompletionItem:icon:
	 *
	 * The #GdkPixbuf for the icon to be shown for this proposal.
	 */
	g_object_class_install_property (object_class,
					 PROP_ICON,
					 g_param_spec_object ("icon",
							      "Icon",
							      "",
							      CDK_TYPE_PIXBUF,
							      G_PARAM_READWRITE |
							      G_PARAM_STATIC_STRINGS));

	/**
	 * CtkSourceCompletionItem:icon-name:
	 *
	 * The icon name for the icon to be shown for this proposal.
	 *
	 * Since: 3.18
	 */
	g_object_class_install_property (object_class,
					 PROP_ICON_NAME,
					 g_param_spec_string ("icon-name",
							      "Icon Name",
							      "",
							      NULL,
							      G_PARAM_READWRITE |
							      G_PARAM_STATIC_STRINGS));

	/**
	 * CtkSourceCompletionItem:gicon:
	 *
	 * The #GIcon for the icon to be shown for this proposal.
	 *
	 * Since: 3.18
	 */
	g_object_class_install_property (object_class,
					 PROP_GICON,
					 g_param_spec_object ("gicon",
							      "GIcon",
							      "",
							      G_TYPE_ICON,
							      G_PARAM_READWRITE |
							      G_PARAM_STATIC_STRINGS));

	/**
	 * CtkSourceCompletionItem:info:
	 *
	 * Optional extra information to be shown for this proposal.
	 */
	g_object_class_install_property (object_class,
					 PROP_INFO,
					 g_param_spec_string ("info",
							      "Info",
							      "",
							      NULL,
							      G_PARAM_READWRITE |
							      G_PARAM_STATIC_STRINGS));
}

static void
ctk_source_completion_item_init (CtkSourceCompletionItem *item)
{
	item->priv = ctk_source_completion_item_get_instance_private (item);
}

/**
 * ctk_source_completion_item_new:
 *
 * Creates a new #CtkSourceCompletionItem. The desired properties need to be set
 * afterwards.
 *
 * Returns: (transfer full): a new #CtkSourceCompletionItem.
 * Since: 4.0
 */
CtkSourceCompletionItem *
ctk_source_completion_item_new (void)
{
	return g_object_new (CTK_SOURCE_TYPE_COMPLETION_ITEM, NULL);
}

/**
 * ctk_source_completion_item_set_label:
 * @item: a #CtkSourceCompletionItem.
 * @label: (nullable): the label, or %NULL.
 *
 * Since: 3.24
 */
void
ctk_source_completion_item_set_label (CtkSourceCompletionItem *item,
				      const gchar             *label)
{
	g_return_if_fail (CTK_SOURCE_IS_COMPLETION_ITEM (item));

	if (g_strcmp0 (item->priv->label, label) != 0)
	{
		g_free (item->priv->label);
		item->priv->label = g_strdup (label);

		emit_changed (item);
		g_object_notify (G_OBJECT (item), "label");
	}
}

/**
 * ctk_source_completion_item_set_markup:
 * @item: a #CtkSourceCompletionItem.
 * @markup: (nullable): the markup, or %NULL.
 *
 * Since: 3.24
 */
void
ctk_source_completion_item_set_markup (CtkSourceCompletionItem *item,
				       const gchar             *markup)
{
	g_return_if_fail (CTK_SOURCE_IS_COMPLETION_ITEM (item));

	if (g_strcmp0 (item->priv->markup, markup) != 0)
	{
		g_free (item->priv->markup);
		item->priv->markup = g_strdup (markup);

		emit_changed (item);
		g_object_notify (G_OBJECT (item), "markup");
	}
}

/**
 * ctk_source_completion_item_set_text:
 * @item: a #CtkSourceCompletionItem.
 * @text: (nullable): the text, or %NULL.
 *
 * Since: 3.24
 */
void
ctk_source_completion_item_set_text (CtkSourceCompletionItem *item,
				     const gchar             *text)
{
	g_return_if_fail (CTK_SOURCE_IS_COMPLETION_ITEM (item));

	if (g_strcmp0 (item->priv->text, text) != 0)
	{
		g_free (item->priv->text);
		item->priv->text = g_strdup (text);

		emit_changed (item);
		g_object_notify (G_OBJECT (item), "text");
	}
}

/**
 * ctk_source_completion_item_set_icon:
 * @item: a #CtkSourceCompletionItem.
 * @icon: (nullable): the #GdkPixbuf, or %NULL.
 *
 * Since: 3.24
 */
void
ctk_source_completion_item_set_icon (CtkSourceCompletionItem *item,
				     GdkPixbuf               *icon)
{
	g_return_if_fail (CTK_SOURCE_IS_COMPLETION_ITEM (item));
	g_return_if_fail (icon == NULL || CDK_IS_PIXBUF (icon));

	if (g_set_object (&item->priv->icon, icon))
	{
		emit_changed (item);
		g_object_notify (G_OBJECT (item), "icon");
	}
}

/**
 * ctk_source_completion_item_set_icon_name:
 * @item: a #CtkSourceCompletionItem.
 * @icon_name: (nullable): the icon name, or %NULL.
 *
 * Since: 3.24
 */
void
ctk_source_completion_item_set_icon_name (CtkSourceCompletionItem *item,
					  const gchar             *icon_name)
{
	g_return_if_fail (CTK_SOURCE_IS_COMPLETION_ITEM (item));

	if (g_strcmp0 (item->priv->icon_name, icon_name) != 0)
	{
		g_free (item->priv->icon_name);
		item->priv->icon_name = g_strdup (icon_name);

		emit_changed (item);
		g_object_notify (G_OBJECT (item), "icon-name");
	}
}

/**
 * ctk_source_completion_item_set_gicon:
 * @item: a #CtkSourceCompletionItem.
 * @gicon: (nullable): the #GIcon, or %NULL.
 *
 * Since: 3.24
 */
void
ctk_source_completion_item_set_gicon (CtkSourceCompletionItem *item,
				      GIcon                   *gicon)
{
	g_return_if_fail (CTK_SOURCE_IS_COMPLETION_ITEM (item));
	g_return_if_fail (gicon == NULL || G_IS_ICON (gicon));

	if (g_set_object (&item->priv->gicon, gicon))
	{
		emit_changed (item);
		g_object_notify (G_OBJECT (item), "gicon");
	}
}

/**
 * ctk_source_completion_item_set_info:
 * @item: a #CtkSourceCompletionItem.
 * @info: (nullable): the info, or %NULL.
 *
 * Since: 3.24
 */
void
ctk_source_completion_item_set_info (CtkSourceCompletionItem *item,
				     const gchar             *info)
{
	g_return_if_fail (CTK_SOURCE_IS_COMPLETION_ITEM (item));

	if (g_strcmp0 (item->priv->info, info) != 0)
	{
		g_free (item->priv->info);
		item->priv->info = g_strdup (info);

		emit_changed (item);
		g_object_notify (G_OBJECT (item), "info");
	}
}
