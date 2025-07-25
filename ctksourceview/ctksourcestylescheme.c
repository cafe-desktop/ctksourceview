/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8; coding: utf-8 -*- */
/*
 * This file is part of CtkSourceView
 *
 * Copyright (C) 2003 - Paolo Maggi <paolo.maggi@polito.it>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
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

#include "ctksourcestylescheme.h"
#include <libxml/parser.h>
#include <string.h>
#include <glib/gi18n-lib.h>
#include "ctksourcestyleschememanager.h"
#include "ctksourcestyle.h"
#include "ctksourcestyle-private.h"
#include "ctksourceview.h"
#include "ctksourcelanguage-private.h"

/**
 * SECTION:stylescheme
 * @Short_description: Controls the appearance of CtkSourceView
 * @Title: CtkSourceStyleScheme
 * @See_also: #CtkSourceStyle, #CtkSourceStyleSchemeManager
 *
 * #CtkSourceStyleScheme contains all the text styles to be used in
 * #CtkSourceView and #CtkSourceBuffer. For instance, it contains text styles
 * for syntax highlighting, it may contain foreground and background color for
 * non-highlighted text, color for the line numbers, current line highlighting,
 * bracket matching, etc.
 *
 * Style schemes are stored in XML files. The format of a scheme file is
 * documented in the [style scheme reference][style-reference].
 *
 * The two style schemes with IDs "classic" and "tango" follow more closely the
 * CTK+ theme (for example for the background color).
 */

#define STYLE_TEXT			"text"
#define STYLE_SELECTED			"selection"
#define STYLE_SELECTED_UNFOCUSED	"selection-unfocused"
#define STYLE_BRACKET_MATCH		"bracket-match"
#define STYLE_BRACKET_MISMATCH		"bracket-mismatch"
#define STYLE_CURSOR			"cursor"
#define STYLE_SECONDARY_CURSOR		"secondary-cursor"
#define STYLE_CURRENT_LINE		"current-line"
#define STYLE_LINE_NUMBERS		"line-numbers"
#define STYLE_CURRENT_LINE_NUMBER	"current-line-number"
#define STYLE_RIGHT_MARGIN		"right-margin"
#define STYLE_DRAW_SPACES		"draw-spaces"
#define STYLE_BACKGROUND_PATTERN	"background-pattern"

#define STYLE_SCHEME_VERSION		"1.0"

#define DEFAULT_STYLE_SCHEME		"classic"

enum
{
	PROP_0,
	PROP_ID,
	PROP_NAME,
	PROP_DESCRIPTION,
	PROP_FILENAME
};

struct _CtkSourceStyleSchemePrivate
{
	gchar *id;
	gchar *name;
	GPtrArray *authors;
	gchar *description;
	gchar *filename;
	CtkSourceStyleScheme *parent;
	gchar *parent_id;
	GHashTable *defined_styles;
	GHashTable *style_cache;
	GHashTable *named_colors;

	CtkCssProvider *css_provider;
	CtkCssProvider *css_provider_cursors;
};

G_DEFINE_TYPE_WITH_PRIVATE (CtkSourceStyleScheme, ctk_source_style_scheme, G_TYPE_OBJECT)

static void
ctk_source_style_scheme_dispose (GObject *object)
{
	CtkSourceStyleScheme *scheme = CTK_SOURCE_STYLE_SCHEME (object);

	if (scheme->priv->named_colors != NULL)
	{
		g_hash_table_unref (scheme->priv->named_colors);
		scheme->priv->named_colors = NULL;
	}

	if (scheme->priv->style_cache != NULL)
	{
		g_hash_table_unref (scheme->priv->style_cache);
		scheme->priv->style_cache = NULL;
	}

	if (scheme->priv->defined_styles != NULL)
	{
		g_hash_table_unref (scheme->priv->defined_styles);
		scheme->priv->defined_styles = NULL;
	}

	g_clear_object (&scheme->priv->parent);
	g_clear_object (&scheme->priv->css_provider);
	g_clear_object (&scheme->priv->css_provider_cursors);

	G_OBJECT_CLASS (ctk_source_style_scheme_parent_class)->dispose (object);
}

static void
ctk_source_style_scheme_finalize (GObject *object)
{
	CtkSourceStyleScheme *scheme = CTK_SOURCE_STYLE_SCHEME (object);

	if (scheme->priv->authors != NULL)
	{
		g_ptr_array_free (scheme->priv->authors, TRUE);
	}

	g_free (scheme->priv->filename);
	g_free (scheme->priv->description);
	g_free (scheme->priv->id);
	g_free (scheme->priv->name);
	g_free (scheme->priv->parent_id);

	G_OBJECT_CLASS (ctk_source_style_scheme_parent_class)->finalize (object);
}

static void
ctk_source_style_scheme_set_property (GObject 	   *object,
				      guint         prop_id,
				      const GValue *value,
				      GParamSpec   *pspec)
{
	CtkSourceStyleScheme *scheme = CTK_SOURCE_STYLE_SCHEME (object);

	switch (prop_id)
	{
		case PROP_ID:
			g_free (scheme->priv->id);
			scheme->priv->id = g_value_dup_string (value);
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
ctk_source_style_scheme_get_property (GObject 	 *object,
				      guint 	  prop_id,
				      GValue 	 *value,
				      GParamSpec *pspec)
{
	CtkSourceStyleScheme *scheme = CTK_SOURCE_STYLE_SCHEME (object);

	switch (prop_id)
	{
		case PROP_ID:
			g_value_set_string (value, scheme->priv->id);
			break;

		case PROP_NAME:
			g_value_set_string (value, scheme->priv->name);
			break;

		case PROP_DESCRIPTION:
			g_value_set_string (value, scheme->priv->description);
			break;

		case PROP_FILENAME:
			g_value_set_string (value, scheme->priv->filename);
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
ctk_source_style_scheme_class_init (CtkSourceStyleSchemeClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->dispose = ctk_source_style_scheme_dispose;
	object_class->finalize = ctk_source_style_scheme_finalize;
	object_class->set_property = ctk_source_style_scheme_set_property;
	object_class->get_property = ctk_source_style_scheme_get_property;

	/**
	 * CtkSourceStyleScheme:id:
	 *
	 * Style scheme id, a unique string used to identify the style scheme
	 * in #CtkSourceStyleSchemeManager.
	 */
	g_object_class_install_property (object_class,
					 PROP_ID,
					 g_param_spec_string ("id",
						 	      "Style scheme id",
							      "Style scheme id",
							      NULL,
							      G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));

	/**
	 * CtkSourceStyleScheme:name:
	 *
	 * Style scheme name, a translatable string to present to the user.
	 */
	g_object_class_install_property (object_class,
					 PROP_NAME,
					 g_param_spec_string ("name",
						 	      "Style scheme name",
							      "Style scheme name",
							      NULL,
							      G_PARAM_READABLE));

	/**
	 * CtkSourceStyleScheme:description:
	 *
	 * Style scheme description, a translatable string to present to the user.
	 */
	g_object_class_install_property (object_class,
					 PROP_DESCRIPTION,
					 g_param_spec_string ("description",
						 	      "Style scheme description",
							      "Style scheme description",
							      NULL,
							      G_PARAM_READABLE));

	/**
	 * CtkSourceStyleScheme:filename:
	 *
	 * Style scheme filename or %NULL.
	 */
	g_object_class_install_property (object_class,
					 PROP_FILENAME,
					 g_param_spec_string ("filename",
						 	      "Style scheme filename",
							      "Style scheme filename",
							      NULL,
							      G_PARAM_READABLE));
}

static void
unref_if_not_null (gpointer object)
{
	if (object != NULL)
	{
		g_object_unref (object);
	}
}

static void
ctk_source_style_scheme_init (CtkSourceStyleScheme *scheme)
{
	scheme->priv = ctk_source_style_scheme_get_instance_private (scheme);

	scheme->priv->defined_styles = g_hash_table_new_full (g_str_hash, g_str_equal,
							      g_free, g_object_unref);

	scheme->priv->style_cache = g_hash_table_new_full (g_str_hash, g_str_equal,
							   g_free, unref_if_not_null);

	scheme->priv->named_colors = g_hash_table_new_full (g_str_hash, g_str_equal,
							    g_free, g_free);

	scheme->priv->css_provider = ctk_css_provider_new ();
}

/**
 * ctk_source_style_scheme_get_id:
 * @scheme: a #CtkSourceStyleScheme.
 *
 * Returns: @scheme id.
 *
 * Since: 2.0
 */
const gchar *
ctk_source_style_scheme_get_id (CtkSourceStyleScheme *scheme)
{
	g_return_val_if_fail (CTK_SOURCE_IS_STYLE_SCHEME (scheme), NULL);
	g_return_val_if_fail (scheme->priv->id != NULL, "");

	return scheme->priv->id;
}

/**
 * ctk_source_style_scheme_get_name:
 * @scheme: a #CtkSourceStyleScheme.
 *
 * Returns: @scheme name.
 *
 * Since: 2.0
 */
const gchar *
ctk_source_style_scheme_get_name (CtkSourceStyleScheme *scheme)
{
	g_return_val_if_fail (CTK_SOURCE_IS_STYLE_SCHEME (scheme), NULL);
	g_return_val_if_fail (scheme->priv->name != NULL, "");

	return scheme->priv->name;
}

/**
 * ctk_source_style_scheme_get_description:
 * @scheme: a #CtkSourceStyleScheme.
 *
 * Returns: (nullable): @scheme description (if defined), or %NULL.
 *
 * Since: 2.0
 */
const gchar *
ctk_source_style_scheme_get_description (CtkSourceStyleScheme *scheme)
{
	g_return_val_if_fail (CTK_SOURCE_IS_STYLE_SCHEME (scheme), NULL);

	return scheme->priv->description;
}

/**
 * ctk_source_style_scheme_get_authors:
 * @scheme: a #CtkSourceStyleScheme.
 *
 * Returns: (nullable) (array zero-terminated=1) (transfer none): a
 * %NULL-terminated array containing the @scheme authors or %NULL if
 * no author is specified by the style scheme.
 *
 * Since: 2.0
 */
const gchar * const *
ctk_source_style_scheme_get_authors (CtkSourceStyleScheme *scheme)
{
	g_return_val_if_fail (CTK_SOURCE_IS_STYLE_SCHEME (scheme), NULL);

	if (scheme->priv->authors == NULL)
	{
		return NULL;
	}

	return (const gchar * const *)scheme->priv->authors->pdata;
}

/**
 * ctk_source_style_scheme_get_filename:
 * @scheme: a #CtkSourceStyleScheme.
 *
 * Returns: (nullable): @scheme file name if the scheme was created
 * parsing a style scheme file or %NULL in the other cases.
 *
 * Since: 2.0
 */
const gchar *
ctk_source_style_scheme_get_filename (CtkSourceStyleScheme *scheme)
{
	g_return_val_if_fail (CTK_SOURCE_IS_STYLE_SCHEME (scheme), NULL);

	return scheme->priv->filename;
}

/*
 * Try to parse a color string.
 * If the color can be parsed, return the offset in the string
 * with the real start of the color (either the string itself, or after
 * the initial '#' character).
 */
static const gchar *
color_parse (const gchar *color,
             CdkRGBA     *rgba)
{
	if ((*color == '#') && cdk_rgba_parse (rgba, color + 1))
	{
		return color + 1;
	}

	if (cdk_rgba_parse (rgba, color))
	{
		return color;
	}

	return NULL;
}

/*
 * get_color_by_name:
 * @scheme: a #CtkSourceStyleScheme.
 * @name: color name to find.
 *
 * Returns: color which corresponds to @name in the @scheme.
 * Returned value is actual color string suitable for cdk_rgba_parse().
 * It may be @name or part of @name so copy it or something, if you need
 * it to stay around.
 *
 * Since: 2.0
 */
static const gchar *
get_color_by_name (CtkSourceStyleScheme *scheme,
		   const gchar          *name)
{
	const char *color = NULL;

	g_return_val_if_fail (name != NULL, NULL);

	if (name[0] == '#')
	{
		CdkRGBA dummy;

		color = color_parse (name, &dummy);

		if (color == NULL)
		{
			g_warning ("could not parse color '%s'", name);
		}
	}
	else
	{
		color = g_hash_table_lookup (scheme->priv->named_colors, name);

		if (color == NULL && scheme->priv->parent != NULL)
		{
			color = get_color_by_name (scheme->priv->parent, name);
		}

		if (color == NULL)
		{
			g_warning ("no color named '%s'", name);
		}
	}

	return color;
}

static CtkSourceStyle *
fix_style_colors (CtkSourceStyleScheme *scheme,
		  CtkSourceStyle       *real_style)
{
	CtkSourceStyle *style;
	guint i;

	struct {
		guint mask;
		guint offset;
	} attributes[] = {
		{ CTK_SOURCE_STYLE_USE_BACKGROUND, G_STRUCT_OFFSET (CtkSourceStyle, background) },
		{ CTK_SOURCE_STYLE_USE_FOREGROUND, G_STRUCT_OFFSET (CtkSourceStyle, foreground) },
		{ CTK_SOURCE_STYLE_USE_LINE_BACKGROUND, G_STRUCT_OFFSET (CtkSourceStyle, line_background) },
		{ CTK_SOURCE_STYLE_USE_UNDERLINE_COLOR, G_STRUCT_OFFSET (CtkSourceStyle, underline_color) }
	};

	style = ctk_source_style_copy (real_style);

	for (i = 0; i < G_N_ELEMENTS (attributes); i++)
	{
		if (style->mask & attributes[i].mask)
		{
			const gchar **attr = G_STRUCT_MEMBER_P (style, attributes[i].offset);
			const gchar *color = get_color_by_name (scheme, *attr);

			if (color == NULL)
			{
				/* warning is spit out in get_color_by_name,
				 * here we make sure style doesn't have NULL color */
				style->mask &= ~attributes[i].mask;
			}
			else
			{
				*attr = g_intern_string (color);
			}
		}
	}

	return style;
}

static CtkSourceStyle *
ctk_source_style_scheme_get_style_internal (CtkSourceStyleScheme *scheme,
					    const gchar          *style_id)
{
	CtkSourceStyle *style = NULL;
	CtkSourceStyle *real_style;

	if (g_hash_table_lookup_extended (scheme->priv->style_cache,
					  style_id,
					  NULL,
					  (gpointer)&style))
	{
		return style;
	}

	real_style = g_hash_table_lookup (scheme->priv->defined_styles, style_id);

	if (real_style == NULL)
	{
		if (scheme->priv->parent != NULL)
		{
			style = ctk_source_style_scheme_get_style (scheme->priv->parent,
								   style_id);
		}
		if (style != NULL)
		{
			g_object_ref (style);
		}
	}
	else
	{
		style = fix_style_colors (scheme, real_style);
	}

	g_hash_table_insert (scheme->priv->style_cache,
			     g_strdup (style_id),
			     style);

	return style;
}

/**
 * ctk_source_style_scheme_get_style:
 * @scheme: a #CtkSourceStyleScheme.
 * @style_id: id of the style to retrieve.
 *
 * Returns: (nullable) (transfer none): style which corresponds to @style_id in
 * the @scheme, or %NULL when no style with this name found.  It is owned by
 * @scheme and may not be unref'ed.
 *
 * Since: 2.0
 */
/*
 * It's a little weird because we have named colors: styles loaded from
 * scheme file can have "#red" or "blue", and we want to give out styles
 * which have nice colors suitable for cdk_color_parse(), so that CtkSourceStyle
 * foreground and background properties are the same as CtkTextTag's.
 * Yet we do need to preserve what we got from file in style schemes,
 * since there may be child schemes which may redefine colors or something,
 * so we can't translate colors when loading scheme.
 * So, defined_styles hash has named colors; styles returned with get_style()
 * have real colors.
 */
CtkSourceStyle *
ctk_source_style_scheme_get_style (CtkSourceStyleScheme *scheme,
				   const gchar          *style_id)
{
	CtkSourceStyle *style;

	g_return_val_if_fail (CTK_SOURCE_IS_STYLE_SCHEME (scheme), NULL);
	g_return_val_if_fail (style_id != NULL, NULL);


	style = ctk_source_style_scheme_get_style_internal (scheme, style_id);

	if (style == NULL)
	{
		/* Long ago, "underlined" was added as a style. The problem with
		 * this is that it defines how something should look rather than
		 * classifying what it is.
		 *
		 * In general, this was used for URLs.
		 *
		 * However, going forward we want to change this but do our best
		 * to not break existing style-schemes. Should "net-address" be
		 * requested, but only "underlined" existed, we will fallback to
		 * the "underlined" style.
		 *
		 * If in the future, we need to support more fallbacks, this should
		 * be changed to a GHashTable to map from src->dst style id.
		 */
		if (g_str_equal (style_id, "def:net-address"))
		{
			style = ctk_source_style_scheme_get_style_internal (scheme, "def:underlined");
		}
	}

	return style;
}

CtkSourceStyle *
_ctk_source_style_scheme_get_matching_brackets_style (CtkSourceStyleScheme *scheme)
{
	g_return_val_if_fail (CTK_SOURCE_IS_STYLE_SCHEME (scheme), NULL);

	return ctk_source_style_scheme_get_style (scheme, STYLE_BRACKET_MATCH);
}

CtkSourceStyle *
_ctk_source_style_scheme_get_right_margin_style (CtkSourceStyleScheme *scheme)
{
	g_return_val_if_fail (CTK_SOURCE_IS_STYLE_SCHEME (scheme), NULL);

	return ctk_source_style_scheme_get_style (scheme, STYLE_RIGHT_MARGIN);
}

CtkSourceStyle *
_ctk_source_style_scheme_get_draw_spaces_style (CtkSourceStyleScheme *scheme)
{
	g_return_val_if_fail (CTK_SOURCE_IS_STYLE_SCHEME (scheme), NULL);

	return ctk_source_style_scheme_get_style (scheme, STYLE_DRAW_SPACES);
}

static gboolean
get_color (CtkSourceStyle *style,
	   gboolean        foreground,
	   CdkRGBA        *dest)
{
	const gchar *color;
	guint mask;

	if (style != NULL)
	{
		if (foreground)
		{
			color = style->foreground;
			mask = CTK_SOURCE_STYLE_USE_FOREGROUND;
		}
		else
		{
			color = style->background;
			mask = CTK_SOURCE_STYLE_USE_BACKGROUND;
		}

		if (style->mask & mask)
		{
			if (color == NULL || !color_parse (color, dest))
			{
				g_warning ("%s: invalid color '%s'", G_STRLOC,
					   color != NULL ? color : "(null)");
				return FALSE;
			}

			return TRUE;
		}
	}

	return FALSE;
}

/*
 * Returns TRUE if the style for current-line is set in the scheme
 */
gboolean
_ctk_source_style_scheme_get_current_line_color (CtkSourceStyleScheme *scheme,
						 CdkRGBA              *color)
{
	CtkSourceStyle *style;

	g_return_val_if_fail (CTK_SOURCE_IS_STYLE_SCHEME (scheme), FALSE);
	g_return_val_if_fail (color != NULL, FALSE);

	style = ctk_source_style_scheme_get_style (scheme, STYLE_CURRENT_LINE);

	return get_color (style, FALSE, color);
}

/*
 * Returns TRUE if the style for background-pattern-color is set in the scheme
 */
gboolean
_ctk_source_style_scheme_get_background_pattern_color (CtkSourceStyleScheme *scheme,
                                                       CdkRGBA              *color)
{
	CtkSourceStyle *style;

	g_return_val_if_fail (CTK_SOURCE_IS_STYLE_SCHEME (scheme), FALSE);
	g_return_val_if_fail (color != NULL, FALSE);

	style = ctk_source_style_scheme_get_style (scheme, STYLE_BACKGROUND_PATTERN);

	return get_color (style, FALSE, color);
}

static gchar *
get_cursors_css_style (CtkSourceStyleScheme *scheme,
		       CtkWidget            *widget)
{
	CtkSourceStyle *primary_style;
	CtkSourceStyle *secondary_style;
	CdkRGBA primary_color = { 0 };
	CdkRGBA secondary_color = { 0 };
	gboolean primary_color_set;
	gboolean secondary_color_set;
	gchar *secondary_color_str;
	GString *css;

	primary_style = ctk_source_style_scheme_get_style (scheme, STYLE_CURSOR);
	secondary_style = ctk_source_style_scheme_get_style (scheme, STYLE_SECONDARY_CURSOR);

	primary_color_set = get_color (primary_style, TRUE, &primary_color);
	secondary_color_set = get_color (secondary_style, TRUE, &secondary_color);

	if (!primary_color_set && !secondary_color_set)
	{
		return NULL;
	}

	css = g_string_new ("textview text {\n");

	if (primary_color_set)
	{
		gchar *primary_color_str;

		primary_color_str = cdk_rgba_to_string (&primary_color);
		g_string_append_printf (css,
					"\tcaret-color: %s;\n",
					primary_color_str);
		g_free (primary_color_str);
	}

	if (!secondary_color_set)
	{
		CtkStyleContext *context;
		CdkRGBA *background_color;

		g_assert (primary_color_set);

		context = ctk_widget_get_style_context (widget);

		ctk_style_context_save (context);
		ctk_style_context_set_state (context, CTK_STATE_FLAG_NORMAL);

		ctk_style_context_get (context,
				       ctk_style_context_get_state (context),
				       "background-color", &background_color,
				       NULL);

		ctk_style_context_restore (context);

		/* Blend primary cursor color with background color. */
		secondary_color.red = (primary_color.red + background_color->red) * 0.5;
		secondary_color.green = (primary_color.green + background_color->green) * 0.5;
		secondary_color.blue = (primary_color.blue + background_color->blue) * 0.5;
		secondary_color.alpha = (primary_color.alpha + background_color->alpha) * 0.5;

		cdk_rgba_free (background_color);
	}

	secondary_color_str = cdk_rgba_to_string (&secondary_color);
	g_string_append_printf (css,
				"\t-ctk-secondary-caret-color: %s;\n",
				secondary_color_str);
	g_free (secondary_color_str);

	g_string_append_printf (css, "}\n");

	return g_string_free (css, FALSE);
}

/* The CssProvider for the cursors depends only on @scheme, but it needs a
 * @widget to shade the background color in case the secondary cursor color
 * isn't defined. The background color is normally defined by @scheme, or if
 * it's not defined it is taken from the CTK+ theme. So ideally, if the CTK+
 * theme changes at runtime, we should regenerate the CssProvider for the
 * cursors, but it isn't done.
 */
static CtkCssProvider *
get_css_provider_cursors (CtkSourceStyleScheme *scheme,
			  CtkWidget            *widget)
{
	gchar *css;
	CtkCssProvider *provider;
	GError *error = NULL;

	css = get_cursors_css_style (scheme, widget);

	if (css == NULL)
	{
		return NULL;
	}

	provider = ctk_css_provider_new ();

	ctk_css_provider_load_from_data (provider, css, -1, &error);
	g_free (css);

	if (error != NULL)
	{
		g_warning ("Error when loading CSS for cursors: %s", error->message);
		g_clear_error (&error);
		g_clear_object (&provider);
	}

	return provider;
}

/**
 * _ctk_source_style_scheme_apply:
 * @scheme:: a #CtkSourceStyleScheme.
 * @view: a #CtkSourceView to apply styles to.
 *
 * Sets style colors from @scheme to the @view.
 *
 * Since: 2.0
 */
void
_ctk_source_style_scheme_apply (CtkSourceStyleScheme *scheme,
				CtkSourceView        *view)
{
	CtkStyleContext *context;

	g_return_if_fail (CTK_SOURCE_IS_STYLE_SCHEME (scheme));
	g_return_if_fail (CTK_SOURCE_IS_VIEW (view));

	context = ctk_widget_get_style_context (CTK_WIDGET (view));
	ctk_style_context_add_provider (context,
	                                CTK_STYLE_PROVIDER (scheme->priv->css_provider),
	                                CTK_SOURCE_STYLE_PROVIDER_PRIORITY);

	/* See https://bugzilla.gnome.org/show_bug.cgi?id=708583 */
	ctk_style_context_invalidate (context);

	/* The CssProvider for the cursors needs that the first provider is
	 * applied, to get the background color.
	 */
	if (scheme->priv->css_provider_cursors == NULL)
	{
		scheme->priv->css_provider_cursors = get_css_provider_cursors (scheme,
									       CTK_WIDGET (view));
	}

	if (scheme->priv->css_provider_cursors != NULL)
	{
		ctk_style_context_add_provider (context,
						CTK_STYLE_PROVIDER (scheme->priv->css_provider_cursors),
						CTK_SOURCE_STYLE_PROVIDER_PRIORITY);

		ctk_style_context_invalidate (context);
	}
}

/**
 * _ctk_source_style_scheme_unapply:
 * @scheme: (nullable): a #CtkSourceStyleScheme or %NULL.
 * @view: a #CtkSourceView to unapply styles to.
 *
 * Removes the styles from @scheme in the @view.
 *
 * Since: 3.0
 */
void
_ctk_source_style_scheme_unapply (CtkSourceStyleScheme *scheme,
				  CtkSourceView        *view)
{
	CtkStyleContext *context;

	g_return_if_fail (CTK_SOURCE_IS_STYLE_SCHEME (scheme));
	g_return_if_fail (CTK_SOURCE_IS_VIEW (view));

	context = ctk_widget_get_style_context (CTK_WIDGET (view));
	ctk_style_context_remove_provider (context,
	                                   CTK_STYLE_PROVIDER (scheme->priv->css_provider));

	if (scheme->priv->css_provider_cursors != NULL)
	{
		ctk_style_context_remove_provider (context,
						   CTK_STYLE_PROVIDER (scheme->priv->css_provider_cursors));
	}

	/* See https://bugzilla.gnome.org/show_bug.cgi?id=708583 */
	ctk_style_context_invalidate (context);
}

/* --- PARSER ---------------------------------------------------------------- */

#define ERROR_QUARK (g_quark_from_static_string ("ctk-source-style-scheme-parser-error"))

static void
get_css_color_style (CtkSourceStyle *style,
                     gchar         **bg,
                     gchar         **text)
{
	CdkRGBA color;

	if (get_color (style, FALSE, &color))
	{
		gchar *bg_color;
		bg_color = cdk_rgba_to_string (&color);
		*bg = g_strdup_printf ("background-color: %s;\n", bg_color);
		g_free (bg_color);
	}
	else
	{
		*bg = NULL;
	}

	if (get_color (style, TRUE, &color))
	{
		gchar *text_color;
		text_color = cdk_rgba_to_string (&color);
		*text = g_strdup_printf ("color: %s;\n", text_color);
		g_free (text_color);
	}
	else
	{
		*text = NULL;
	}
}

static void
append_css_style (GString        *string,
                  CtkSourceStyle *style,
                  const gchar    *selector)
{
	gchar *bg, *text;
	const gchar css_style[] =
		"%s {\n"
		"	%s"
		"	%s"
		"}\n";

	get_css_color_style (style, &bg, &text);
	if (bg || text)
	{
		g_string_append_printf (string, css_style, selector,
		                        bg != NULL ? bg : "",
		                        text != NULL ? text : "");

		g_free (bg);
		g_free (text);
	}
}

static void
generate_css_style (CtkSourceStyleScheme *scheme)
{
	GString *final_style;
	CtkSourceStyle *style, *style2;

	final_style = g_string_new ("");

	style = ctk_source_style_scheme_get_style (scheme, STYLE_TEXT);
	append_css_style (final_style, style, "textview text");

	style = ctk_source_style_scheme_get_style (scheme, STYLE_SELECTED);
	append_css_style (final_style, style, "textview:focus text selection");

	style2 = ctk_source_style_scheme_get_style (scheme, STYLE_SELECTED_UNFOCUSED);
	append_css_style (final_style,
			  style2 != NULL ? style2 : style,
			  "textview text selection");

	/* For now we use "line numbers" colors for all the gutters */
	style = ctk_source_style_scheme_get_style (scheme, STYLE_LINE_NUMBERS);
	if (style != NULL)
	{
		append_css_style (final_style, style, "textview border");

		/* Needed for CtkSourceGutter. In the ::draw callback,
		 * ctk_style_context_add_class() is called to add e.g. the
		 * "left" class. Because as of CTK+ 3.20 we cannot do the same
		 * to add the "border" subnode.
		 */
		append_css_style (final_style, style, "textview .left");
		append_css_style (final_style, style, "textview .right");
		append_css_style (final_style, style, "textview .top");
		append_css_style (final_style, style, "textview .bottom");

		/* For the corners if the top or bottom gutter is also
		 * displayed.
		 * FIXME: this shouldn't be necessary, CTK+ should apply the
		 * border style to the corners too, see:
		 * https://bugzilla.gnome.org/show_bug.cgi?id=764239
		 */
		append_css_style (final_style, style, "textview");
	}

	style = ctk_source_style_scheme_get_style (scheme, STYLE_CURRENT_LINE_NUMBER);
	if (style != NULL)
	{
		append_css_style (final_style, style, "textview .current-line-number");
	}

	if (*final_style->str != '\0')
	{
		GError *error = NULL;

		ctk_css_provider_load_from_data (scheme->priv->css_provider,
						 final_style->str,
						 final_style->len,
						 &error);

		if (error != NULL)
		{
			g_warning ("%s", error->message);
			g_clear_error (&error);
		}
	}

	g_string_free (final_style, TRUE);
}

static gboolean
parse_bool (char *value)
{
	return (g_ascii_strcasecmp (value, "true") == 0 ||
	        g_ascii_strcasecmp (value, "yes") == 0 ||
	        g_ascii_strcasecmp (value, "1") == 0);
}

static void
get_bool (xmlNode    *node,
	  const char *propname,
	  guint      *mask,
	  guint       mask_value,
	  gboolean   *value)
{
	xmlChar *tmp = xmlGetProp (node, BAD_CAST propname);

	if (tmp != NULL)
	{
		*mask |= mask_value;
		*value = parse_bool ((char*) tmp);
	}

	xmlFree (tmp);
}

static gboolean
parse_style (CtkSourceStyleScheme *scheme,
	     xmlNode              *node,
	     gchar               **style_name_p,
	     CtkSourceStyle      **style_p,
	     GError              **error)
{
	CtkSourceStyle *use_style = NULL;
	CtkSourceStyle *result = NULL;
	xmlChar *fg = NULL;
	xmlChar *bg = NULL;
	xmlChar *line_bg = NULL;
	gchar *style_name = NULL;
	guint mask = 0;
	gboolean bold = FALSE;
	gboolean italic = FALSE;
	gboolean strikethrough = FALSE;
	xmlChar *underline = NULL;
	xmlChar *underline_color = NULL;
	xmlChar *scale = NULL;
	xmlChar *tmp;

	tmp = xmlGetProp (node, BAD_CAST "name");
	if (tmp == NULL)
	{
		g_set_error (error, ERROR_QUARK, 0, "name attribute missing");
		return FALSE;
	}
	style_name = g_strdup ((char*) tmp);
	xmlFree (tmp);

	tmp = xmlGetProp (node, BAD_CAST "use-style");
	if (tmp != NULL)
	{
		use_style = ctk_source_style_scheme_get_style (scheme, (char*) tmp);

		if (use_style == NULL)
		{
			g_set_error (error, ERROR_QUARK, 0,
				     "in style '%s': unknown style '%s'",
				     style_name, tmp);
			g_free (style_name);
			return FALSE;
		}

		g_object_ref (use_style);
	}
	xmlFree (tmp);

	fg = xmlGetProp (node, BAD_CAST "foreground");
	bg = xmlGetProp (node, BAD_CAST "background");
	line_bg = xmlGetProp (node, BAD_CAST "line-background");
	get_bool (node, "italic", &mask, CTK_SOURCE_STYLE_USE_ITALIC, &italic);
	get_bool (node, "bold", &mask, CTK_SOURCE_STYLE_USE_BOLD, &bold);
	get_bool (node, "strikethrough", &mask, CTK_SOURCE_STYLE_USE_STRIKETHROUGH, &strikethrough);
	underline = xmlGetProp (node, BAD_CAST "underline");
	underline_color = xmlGetProp (node, BAD_CAST "underline-color");
	scale = xmlGetProp (node, BAD_CAST "scale");

	if (use_style)
	{
		if (fg != NULL ||
		    bg != NULL ||
		    line_bg != NULL ||
		    mask != 0 ||
		    underline != NULL ||
		    underline_color != NULL ||
		    scale != NULL)
		{
			g_set_error (error, ERROR_QUARK, 0,
				     "in style '%s': style attributes used along with use-style",
				     style_name);
			g_object_unref (use_style);
			g_free (style_name);
			xmlFree (fg);
			xmlFree (bg);
			xmlFree (line_bg);
			xmlFree (underline);
			xmlFree (underline_color);
			xmlFree (scale);
			return FALSE;
		}

		result = use_style;
		use_style = NULL;
	}
	else
	{
		result = g_object_new (CTK_SOURCE_TYPE_STYLE, NULL);

		result->mask = mask;
		result->bold = bold;
		result->italic = italic;
		result->strikethrough = strikethrough;

		if (fg != NULL)
		{
			result->foreground = g_intern_string ((char*) fg);
			result->mask |= CTK_SOURCE_STYLE_USE_FOREGROUND;
		}

		if (bg != NULL)
		{
			result->background = g_intern_string ((char*) bg);
			result->mask |= CTK_SOURCE_STYLE_USE_BACKGROUND;
		}

		if (line_bg != NULL)
		{
			result->line_background = g_intern_string ((char*) line_bg);
			result->mask |= CTK_SOURCE_STYLE_USE_LINE_BACKGROUND;
		}

		if (underline != NULL)
		{
			/* Up until 3.16 underline was a "bool", so for backward
			 * compat we accept underline="true" and map it to "single"
			 */
			if (parse_bool ((char *) underline))
			{
				result->underline = PANGO_UNDERLINE_SINGLE;
				result->mask |= CTK_SOURCE_STYLE_USE_UNDERLINE;
			}
			else
			{
				GEnumClass *enum_class;
				GEnumValue *enum_value;
				gchar *underline_lowercase;

				enum_class = G_ENUM_CLASS (g_type_class_ref (PANGO_TYPE_UNDERLINE));

				underline_lowercase = g_ascii_strdown ((char*) underline, -1);
				enum_value = g_enum_get_value_by_nick (enum_class, underline_lowercase);
				g_free (underline_lowercase);

				if (enum_value != NULL)
				{
					result->underline = enum_value->value;
					result->mask |= CTK_SOURCE_STYLE_USE_UNDERLINE;
				}

				g_type_class_unref (enum_class);
			}
		}

		if (underline_color != NULL)
		{
			result->underline_color = g_intern_string ((char*) underline_color);
			result->mask |= CTK_SOURCE_STYLE_USE_UNDERLINE_COLOR;
		}

		if (scale != NULL)
		{
			result->scale = g_intern_string ((char*) scale);
			result->mask |= CTK_SOURCE_STYLE_USE_SCALE;
		}
	}

	*style_p = result;
	*style_name_p = style_name;

	xmlFree (fg);
	xmlFree (bg);
	xmlFree (line_bg);
	xmlFree (underline);
	xmlFree (underline_color);
	xmlFree (scale);

	return TRUE;
}

static gboolean
parse_color (CtkSourceStyleScheme *scheme,
	     xmlNode              *node,
	     GError              **error)
{
	xmlChar *name, *value;
	gboolean result = FALSE;

	name = xmlGetProp (node, BAD_CAST "name");
	value = xmlGetProp (node, BAD_CAST "value");

	if (name == NULL || name[0] == 0)
		g_set_error (error, ERROR_QUARK, 0, "name attribute missing in 'color' tag");
	else if (value == NULL)
		g_set_error (error, ERROR_QUARK, 0, "value attribute missing in 'color' tag");
	else if (value[0] != '#' || value[1] == 0)
		g_set_error (error, ERROR_QUARK, 0, "value in 'color' tag is not of the form '#RGB' or '#name'");
	else if (g_hash_table_lookup (scheme->priv->named_colors, name) != NULL)
		g_set_error (error, ERROR_QUARK, 0, "duplicated color '%s'", name);
	else
		result = TRUE;

	if (result)
		g_hash_table_insert (scheme->priv->named_colors,
				     g_strdup ((char *) name),
				     g_strdup ((char *) value));

	xmlFree (value);
	xmlFree (name);

	return result;
}

static gboolean
parse_style_scheme_child (CtkSourceStyleScheme *scheme,
			  xmlNode              *node,
			  GError              **error)
{
	if (strcmp ((char*) node->name, "style") == 0)
	{
		CtkSourceStyle *style;
		gchar *style_name;

		if (!parse_style (scheme, node, &style_name, &style, error))
			return FALSE;

		g_hash_table_insert (scheme->priv->defined_styles, style_name, style);
	}
	else if (strcmp ((char*) node->name, "color") == 0)
	{
		if (!parse_color (scheme, node, error))
			return FALSE;
	}
	else if (strcmp ((char*) node->name, "author") == 0)
	{
		xmlChar *tmp = xmlNodeGetContent (node);
		if (scheme->priv->authors == NULL)
			scheme->priv->authors = g_ptr_array_new_with_free_func (g_free);

		g_ptr_array_add (scheme->priv->authors, g_strdup ((char*) tmp));

		xmlFree (tmp);
	}
	else if (strcmp ((char*) node->name, "description") == 0)
	{
		xmlChar *tmp = xmlNodeGetContent (node);
		scheme->priv->description = g_strdup ((char*) tmp);
		xmlFree (tmp);
	}
	else if (strcmp ((char*) node->name, "_description") == 0)
	{
		xmlChar *tmp = xmlNodeGetContent (node);
		scheme->priv->description = g_strdup (_((char*) tmp));
		xmlFree (tmp);
	}
	else
	{
		g_set_error (error, ERROR_QUARK, 0, "unknown node '%s'", node->name);
		return FALSE;
	}

	return TRUE;
}

static void
parse_style_scheme_element (CtkSourceStyleScheme *scheme,
			    xmlNode              *scheme_node,
			    GError              **error)
{
	xmlChar *tmp;
	xmlNode *node;

	if (strcmp ((char*) scheme_node->name, "style-scheme") != 0)
	{
		g_set_error (error, ERROR_QUARK, 0,
			     "unexpected element '%s'",
			     (char*) scheme_node->name);
		return;
	}

	tmp = xmlGetProp (scheme_node, BAD_CAST "version");
	if (tmp == NULL)
	{
		g_set_error (error, ERROR_QUARK, 0, "missing 'version' attribute");
		return;
	}
	if (strcmp ((char*) tmp, STYLE_SCHEME_VERSION) != 0)
	{
		g_set_error (error, ERROR_QUARK, 0, "unsupported version '%s'", (char*) tmp);
		xmlFree (tmp);
		return;
	}
	xmlFree (tmp);

	tmp = xmlGetProp (scheme_node, BAD_CAST "id");
	if (tmp == NULL)
	{
		g_set_error (error, ERROR_QUARK, 0, "missing 'id' attribute");
		return;
	}
	scheme->priv->id = g_strdup ((char*) tmp);
	xmlFree (tmp);

	tmp = xmlGetProp (scheme_node, BAD_CAST "_name");
	if (tmp != NULL)
		scheme->priv->name = g_strdup (_((char*) tmp));
	else if ((tmp = xmlGetProp (scheme_node, BAD_CAST "name")) != NULL)
		scheme->priv->name = g_strdup ((char*) tmp);
	else
	{
		g_set_error (error, ERROR_QUARK, 0, "missing 'name' attribute");
		return;
	}
	xmlFree (tmp);

	tmp = xmlGetProp (scheme_node, BAD_CAST "parent-scheme");
	if (tmp != NULL)
		scheme->priv->parent_id = g_strdup ((char*) tmp);
	xmlFree (tmp);

	for (node = scheme_node->children; node != NULL; node = node->next)
		if (node->type == XML_ELEMENT_NODE)
			if (!parse_style_scheme_child (scheme, node, error))
				return;

	/* NULL-terminate the array of authors */
	if (scheme->priv->authors != NULL)
		g_ptr_array_add (scheme->priv->authors, NULL);
}

/**
 * _ctk_source_style_scheme_new_from_file:
 * @filename: file to parse.
 *
 * Returns: (nullable): new #CtkSourceStyleScheme created from file,
 * or %NULL on error.
 *
 * Since: 2.0
 */
CtkSourceStyleScheme *
_ctk_source_style_scheme_new_from_file (const gchar *filename)
{
	CtkSourceStyleScheme *scheme;
	gchar *text;
	gsize text_len;
	xmlDoc *doc;
	xmlNode *node;
	GError *error = NULL;

	g_return_val_if_fail (filename != NULL, NULL);

	if (!g_file_get_contents (filename, &text, &text_len, &error))
	{
		gchar *filename_utf8 = g_filename_display_name (filename);
		g_warning ("could not load style scheme file '%s': %s",
			   filename_utf8, error->message);
		g_free (filename_utf8);
		g_clear_error (&error);
		return NULL;
	}

	doc = xmlParseMemory (text, text_len);

	if (!doc)
	{
		gchar *filename_utf8 = g_filename_display_name (filename);
		g_warning ("could not parse scheme file '%s'", filename_utf8);
		g_free (filename_utf8);
		g_free (text);
		return NULL;
	}

	node = xmlDocGetRootElement (doc);

	if (node == NULL)
	{
		gchar *filename_utf8 = g_filename_display_name (filename);
		g_warning ("could not load scheme file '%s': empty document", filename_utf8);
		g_free (filename_utf8);
		xmlFreeDoc (doc);
		g_free (text);
		return NULL;
	}

	scheme = g_object_new (CTK_SOURCE_TYPE_STYLE_SCHEME, NULL);
	scheme->priv->filename = g_strdup (filename);

	parse_style_scheme_element (scheme, node, &error);

	if (error != NULL)
	{
		gchar *filename_utf8 = g_filename_display_name (filename);
		g_warning ("could not load style scheme file '%s': %s",
			   filename_utf8, error->message);
		g_free (filename_utf8);
		g_clear_error (&error);
		g_clear_object (&scheme);
	}
	else
	{
		/* css style part */
		generate_css_style (scheme);
	}

	xmlFreeDoc (doc);
	g_free (text);

	return scheme;
}

/**
 * _ctk_source_style_scheme_get_parent_id:
 * @scheme: a #CtkSourceStyleScheme.
 *
 * Returns: (nullable): parent style scheme id or %NULL.
 *
 * Since: 2.0
 */
const gchar *
_ctk_source_style_scheme_get_parent_id (CtkSourceStyleScheme *scheme)
{
	g_return_val_if_fail (CTK_SOURCE_IS_STYLE_SCHEME (scheme), NULL);

	return scheme->priv->parent_id;
}

/**
 * _ctk_source_style_scheme_set_parent:
 * @scheme: a #CtkSourceStyleScheme.
 * @parent_scheme: parent #CtkSourceStyleScheme for @scheme.
 *
 * Sets @parent_scheme as parent scheme for @scheme, @scheme will
 * look for styles in @parent_scheme if it doesn't have style set
 * for given name.
 *
 * Since: 2.0
 */
void
_ctk_source_style_scheme_set_parent (CtkSourceStyleScheme *scheme,
				     CtkSourceStyleScheme *parent_scheme)
{
	g_return_if_fail (CTK_SOURCE_IS_STYLE_SCHEME (scheme));
	g_return_if_fail (parent_scheme == NULL || CTK_SOURCE_IS_STYLE_SCHEME (parent_scheme));

	if (scheme->priv->parent == parent_scheme)
	{
		return;
	}

	g_clear_object (&scheme->priv->parent);

	if (parent_scheme != NULL)
	{
		g_object_ref (parent_scheme);
	}

	scheme->priv->parent = parent_scheme;

	/* Update CSS based on parent styles */
	g_hash_table_remove_all (scheme->priv->style_cache);
	generate_css_style (scheme);
}

/**
 * _ctk_source_style_scheme_get_default:
 *
 * Returns: default style scheme to be used when user didn't set
 * style scheme explicitly.
 *
 * Since: 2.0
 */
CtkSourceStyleScheme *
_ctk_source_style_scheme_get_default (void)
{
	CtkSourceStyleSchemeManager *manager;

	manager = ctk_source_style_scheme_manager_get_default ();

	return ctk_source_style_scheme_manager_get_scheme (manager,
							   DEFAULT_STYLE_SCHEME);
}
