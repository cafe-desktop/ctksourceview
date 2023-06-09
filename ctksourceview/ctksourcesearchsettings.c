/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8; coding: utf-8 -*- */
/*
 * This file is part of CtkSourceView
 *
 * Copyright (C) 2013 - Sébastien Wilmet <swilmet@gnome.org>
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

#include "ctksourcesearchsettings.h"

/**
 * SECTION:searchsettings
 * @Short_description: Search settings
 * @Title: CtkSourceSearchSettings
 * @See_also: #CtkSourceSearchContext
 *
 * A #CtkSourceSearchSettings object represents the settings of a search. The
 * search settings can be associated with one or several
 * #CtkSourceSearchContext<!-- -->s.
 */

enum
{
	PROP_0,
	PROP_SEARCH_TEXT,
	PROP_CASE_SENSITIVE,
	PROP_AT_WORD_BOUNDARIES,
	PROP_WRAP_AROUND,
	PROP_REGEX_ENABLED
};

struct _CtkSourceSearchSettingsPrivate
{
	gchar *search_text;
	guint case_sensitive : 1;
	guint at_word_boundaries : 1;
	guint wrap_around : 1;
	guint regex_enabled : 1;
};

G_DEFINE_TYPE_WITH_PRIVATE (CtkSourceSearchSettings, ctk_source_search_settings, G_TYPE_OBJECT)

static void
ctk_source_search_settings_finalize (GObject *object)
{
	CtkSourceSearchSettings *settings = CTK_SOURCE_SEARCH_SETTINGS (object);

	g_free (settings->priv->search_text);

	G_OBJECT_CLASS (ctk_source_search_settings_parent_class)->finalize (object);
}

static void
ctk_source_search_settings_get_property (GObject    *object,
					 guint       prop_id,
					 GValue     *value,
					 GParamSpec *pspec)
{
	CtkSourceSearchSettings *settings;

	g_return_if_fail (CTK_SOURCE_IS_SEARCH_SETTINGS (object));

	settings = CTK_SOURCE_SEARCH_SETTINGS (object);

	switch (prop_id)
	{
		case PROP_SEARCH_TEXT:
			g_value_set_string (value, settings->priv->search_text);
			break;

		case PROP_CASE_SENSITIVE:
			g_value_set_boolean (value, settings->priv->case_sensitive);
			break;

		case PROP_AT_WORD_BOUNDARIES:
			g_value_set_boolean (value, settings->priv->at_word_boundaries);
			break;

		case PROP_WRAP_AROUND:
			g_value_set_boolean (value, settings->priv->wrap_around);
			break;

		case PROP_REGEX_ENABLED:
			g_value_set_boolean (value, settings->priv->regex_enabled);
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
ctk_source_search_settings_set_property (GObject      *object,
					 guint         prop_id,
					 const GValue *value,
					 GParamSpec   *pspec)
{
	CtkSourceSearchSettings *settings;

	g_return_if_fail (CTK_SOURCE_IS_SEARCH_SETTINGS (object));

	settings = CTK_SOURCE_SEARCH_SETTINGS (object);

	switch (prop_id)
	{
		case PROP_SEARCH_TEXT:
			ctk_source_search_settings_set_search_text (settings, g_value_get_string (value));
			break;

		case PROP_CASE_SENSITIVE:
			settings->priv->case_sensitive = g_value_get_boolean (value);
			break;

		case PROP_AT_WORD_BOUNDARIES:
			settings->priv->at_word_boundaries = g_value_get_boolean (value);
			break;

		case PROP_WRAP_AROUND:
			settings->priv->wrap_around = g_value_get_boolean (value);
			break;

		case PROP_REGEX_ENABLED:
			settings->priv->regex_enabled = g_value_get_boolean (value);
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
ctk_source_search_settings_class_init (CtkSourceSearchSettingsClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->finalize = ctk_source_search_settings_finalize;
	object_class->get_property = ctk_source_search_settings_get_property;
	object_class->set_property = ctk_source_search_settings_set_property;

	/**
	 * CtkSourceSearchSettings:search-text:
	 *
	 * A search string, or %NULL if the search is disabled. If the regular
	 * expression search is enabled, #CtkSourceSearchSettings:search-text is
	 * the pattern.
	 *
	 * Since: 3.10
	 */
	g_object_class_install_property (object_class,
					 PROP_SEARCH_TEXT,
					 g_param_spec_string ("search-text",
							      "Search text",
							      "The text to search",
							      NULL,
							      G_PARAM_READWRITE |
							      G_PARAM_CONSTRUCT |
							      G_PARAM_STATIC_STRINGS));

	/**
	 * CtkSourceSearchSettings:case-sensitive:
	 *
	 * Whether the search is case sensitive.
	 *
	 * Since: 3.10
	 */
	g_object_class_install_property (object_class,
					 PROP_CASE_SENSITIVE,
					 g_param_spec_boolean ("case-sensitive",
							       "Case sensitive",
							       "Case sensitive",
							       FALSE,
							       G_PARAM_READWRITE |
							       G_PARAM_CONSTRUCT |
							       G_PARAM_STATIC_STRINGS));

	/**
	 * CtkSourceSearchSettings:at-word-boundaries:
	 *
	 * If %TRUE, a search match must start and end a word. The match can
	 * span multiple words.
	 *
	 * Since: 3.10
	 */
	g_object_class_install_property (object_class,
					 PROP_AT_WORD_BOUNDARIES,
					 g_param_spec_boolean ("at-word-boundaries",
							       "At word boundaries",
							       "Search at word boundaries",
							       FALSE,
							       G_PARAM_READWRITE |
							       G_PARAM_CONSTRUCT |
							       G_PARAM_STATIC_STRINGS));

	/**
	 * CtkSourceSearchSettings:wrap-around:
	 *
	 * For a forward search, continue at the beginning of the buffer if no
	 * search occurrence is found. For a backward search, continue at the
	 * end of the buffer.
	 *
	 * Since: 3.10
	 */
	g_object_class_install_property (object_class,
					 PROP_WRAP_AROUND,
					 g_param_spec_boolean ("wrap-around",
							       "Wrap around",
							       "Wrap around",
							       FALSE,
							       G_PARAM_READWRITE |
							       G_PARAM_CONSTRUCT |
							       G_PARAM_STATIC_STRINGS));

	/**
	 * CtkSourceSearchSettings:regex-enabled:
	 *
	 * Search by regular expressions with
	 * #CtkSourceSearchSettings:search-text as the pattern.
	 *
	 * Since: 3.10
	 */
	g_object_class_install_property (object_class,
					 PROP_REGEX_ENABLED,
					 g_param_spec_boolean ("regex-enabled",
							       "Regex enabled",
							       "Whether to search by regular expression",
							       FALSE,
							       G_PARAM_READWRITE |
							       G_PARAM_CONSTRUCT |
							       G_PARAM_STATIC_STRINGS));
}

static void
ctk_source_search_settings_init (CtkSourceSearchSettings *self)
{
	self->priv = ctk_source_search_settings_get_instance_private (self);
}

/**
 * ctk_source_search_settings_new:
 *
 * Creates a new search settings object.
 *
 * Returns: a new search settings object.
 * Since: 3.10
 */
CtkSourceSearchSettings *
ctk_source_search_settings_new (void)
{
	return g_object_new (CTK_SOURCE_TYPE_SEARCH_SETTINGS, NULL);
}

/**
 * ctk_source_search_settings_set_search_text:
 * @settings: a #CtkSourceSearchSettings.
 * @search_text: (nullable): the nul-terminated text to search, or %NULL to disable the search.
 *
 * Sets the text to search. If @search_text is %NULL or is empty, the search
 * will be disabled. A copy of @search_text will be made, so you can safely free
 * @search_text after a call to this function.
 *
 * You may be interested to call ctk_source_utils_unescape_search_text() before
 * this function.
 *
 * Since: 3.10
 */
void
ctk_source_search_settings_set_search_text (CtkSourceSearchSettings *settings,
					    const gchar             *search_text)
{
	g_return_if_fail (CTK_SOURCE_IS_SEARCH_SETTINGS (settings));
	g_return_if_fail (search_text == NULL || g_utf8_validate (search_text, -1, NULL));

	if ((settings->priv->search_text == NULL &&
	     (search_text == NULL || search_text[0] == '\0')) ||
	    g_strcmp0 (settings->priv->search_text, search_text) == 0)
	{
		return;
	}

	g_free (settings->priv->search_text);

	if (search_text == NULL || search_text[0] == '\0')
	{
		settings->priv->search_text = NULL;
	}
	else
	{
		settings->priv->search_text = g_strdup (search_text);
	}

	g_object_notify (G_OBJECT (settings), "search-text");
}

/**
 * ctk_source_search_settings_get_search_text:
 * @settings: a #CtkSourceSearchSettings.
 *
 * Gets the text to search. The return value must not be freed.
 *
 * You may be interested to call ctk_source_utils_escape_search_text() after
 * this function.
 *
 * Returns: (nullable): the text to search, or %NULL if the search is disabled.
 * Since: 3.10
 */
const gchar *
ctk_source_search_settings_get_search_text (CtkSourceSearchSettings *settings)
{
	g_return_val_if_fail (CTK_SOURCE_IS_SEARCH_SETTINGS (settings), NULL);

	return settings->priv->search_text;
}

/**
 * ctk_source_search_settings_set_case_sensitive:
 * @settings: a #CtkSourceSearchSettings.
 * @case_sensitive: the setting.
 *
 * Enables or disables the case sensitivity for the search.
 *
 * Since: 3.10
 */
void
ctk_source_search_settings_set_case_sensitive (CtkSourceSearchSettings *settings,
					       gboolean                 case_sensitive)
{
	g_return_if_fail (CTK_SOURCE_IS_SEARCH_SETTINGS (settings));

	case_sensitive = case_sensitive != FALSE;

	if (settings->priv->case_sensitive != case_sensitive)
	{
		settings->priv->case_sensitive = case_sensitive;
		g_object_notify (G_OBJECT (settings), "case-sensitive");
	}
}

/**
 * ctk_source_search_settings_get_case_sensitive:
 * @settings: a #CtkSourceSearchSettings.
 *
 * Returns: whether the search is case sensitive.
 * Since: 3.10
 */
gboolean
ctk_source_search_settings_get_case_sensitive (CtkSourceSearchSettings *settings)
{
	g_return_val_if_fail (CTK_SOURCE_IS_SEARCH_SETTINGS (settings), FALSE);

	return settings->priv->case_sensitive;
}

/**
 * ctk_source_search_settings_set_at_word_boundaries:
 * @settings: a #CtkSourceSearchSettings.
 * @at_word_boundaries: the setting.
 *
 * Change whether the search is done at word boundaries. If @at_word_boundaries
 * is %TRUE, a search match must start and end a word. The match can span
 * multiple words. See also ctk_text_iter_starts_word() and
 * ctk_text_iter_ends_word().
 *
 * Since: 3.10
 */
void
ctk_source_search_settings_set_at_word_boundaries (CtkSourceSearchSettings *settings,
						   gboolean                 at_word_boundaries)
{
	g_return_if_fail (CTK_SOURCE_IS_SEARCH_SETTINGS (settings));

	at_word_boundaries = at_word_boundaries != FALSE;

	if (settings->priv->at_word_boundaries != at_word_boundaries)
	{
		settings->priv->at_word_boundaries = at_word_boundaries;
		g_object_notify (G_OBJECT (settings), "at-word-boundaries");
	}
}

/**
 * ctk_source_search_settings_get_at_word_boundaries:
 * @settings: a #CtkSourceSearchSettings.
 *
 * Returns: whether to search at word boundaries.
 * Since: 3.10
 */
gboolean
ctk_source_search_settings_get_at_word_boundaries (CtkSourceSearchSettings *settings)
{
	g_return_val_if_fail (CTK_SOURCE_IS_SEARCH_SETTINGS (settings), FALSE);

	return settings->priv->at_word_boundaries;
}

/**
 * ctk_source_search_settings_set_wrap_around:
 * @settings: a #CtkSourceSearchSettings.
 * @wrap_around: the setting.
 *
 * Enables or disables the wrap around search. If @wrap_around is %TRUE, the
 * forward search continues at the beginning of the buffer if no search
 * occurrences are found. Similarly, the backward search continues to search at
 * the end of the buffer.
 *
 * Since: 3.10
 */
void
ctk_source_search_settings_set_wrap_around (CtkSourceSearchSettings *settings,
					    gboolean                 wrap_around)
{
	g_return_if_fail (CTK_SOURCE_IS_SEARCH_SETTINGS (settings));

	wrap_around = wrap_around != FALSE;

	if (settings->priv->wrap_around != wrap_around)
	{
		settings->priv->wrap_around = wrap_around;
		g_object_notify (G_OBJECT (settings), "wrap-around");
	}
}

/**
 * ctk_source_search_settings_get_wrap_around:
 * @settings: a #CtkSourceSearchSettings.
 *
 * Returns: whether to wrap around the search.
 * Since: 3.10
 */
gboolean
ctk_source_search_settings_get_wrap_around (CtkSourceSearchSettings *settings)
{
	g_return_val_if_fail (CTK_SOURCE_IS_SEARCH_SETTINGS (settings), FALSE);

	return settings->priv->wrap_around;
}

/**
 * ctk_source_search_settings_set_regex_enabled:
 * @settings: a #CtkSourceSearchSettings.
 * @regex_enabled: the setting.
 *
 * Enables or disables whether to search by regular expressions.
 * If enabled, the #CtkSourceSearchSettings:search-text property contains the
 * pattern of the regular expression.
 *
 * #CtkSourceSearchContext uses #GRegex when regex search is enabled. See the
 * [Regular expression syntax](https://developer.gnome.org/glib/stable/glib-regex-syntax.html)
 * page in the GLib reference manual.
 *
 * Since: 3.10
 */
void
ctk_source_search_settings_set_regex_enabled (CtkSourceSearchSettings *settings,
					      gboolean                 regex_enabled)
{
	g_return_if_fail (CTK_SOURCE_IS_SEARCH_SETTINGS (settings));

	regex_enabled = regex_enabled != FALSE;

	if (settings->priv->regex_enabled != regex_enabled)
	{
		settings->priv->regex_enabled = regex_enabled;
		g_object_notify (G_OBJECT (settings), "regex-enabled");
	}
}

/**
 * ctk_source_search_settings_get_regex_enabled:
 * @settings: a #CtkSourceSearchSettings.
 *
 * Returns: whether to search by regular expressions.
 * Since: 3.10
 */
gboolean
ctk_source_search_settings_get_regex_enabled (CtkSourceSearchSettings *settings)
{
	g_return_val_if_fail (CTK_SOURCE_IS_SEARCH_SETTINGS (settings), FALSE);

	return settings->priv->regex_enabled;
}
