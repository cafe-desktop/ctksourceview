/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8; coding: utf-8 -*- */
/*
 * This file is part of CtkSourceView
 *
 * Copyright (C) 2008, 2011, 2016 - Paolo Borelli <pborelli@gnome.org>
 * Copyright (C) 2008, 2010 - Ignacio Casal Quinteiro <icq@gnome.org>
 * Copyright (C) 2010 - Garret Regier
 * Copyright (C) 2013 - Arpad Borsos <arpad.borsos@googlemail.com>
 * Copyright (C) 2015, 2016 - Sébastien Wilmet <swilmet@gnome.org>
 * Copyright (C) 2016 - Christian Hergert <christian@hergert.me>
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

#include "ctksourcespacedrawer.h"
#include "ctksourcespacedrawer-private.h"
#include "ctksourcebuffer.h"
#include "ctksourcebuffer-private.h"
#include "ctksourceiter.h"
#include "ctksourcestylescheme.h"
#include "ctksourcetag.h"
#include "ctksourceview.h"

/**
 * SECTION:spacedrawer
 * @Short_description: Represent white space characters with symbols
 * @Title: CtkSourceSpaceDrawer
 * @See_also: #CtkSourceView
 *
 * #CtkSourceSpaceDrawer provides a way to visualize white spaces, by drawing
 * symbols.
 *
 * Call ctk_source_view_get_space_drawer() to get the #CtkSourceSpaceDrawer
 * instance of a certain #CtkSourceView.
 *
 * By default, no white spaces are drawn because the
 * #CtkSourceSpaceDrawer:enable-matrix is %FALSE.
 *
 * To draw white spaces, ctk_source_space_drawer_set_types_for_locations() can
 * be called to set the #CtkSourceSpaceDrawer:matrix property (by default all
 * space types are enabled at all locations). Then call
 * ctk_source_space_drawer_set_enable_matrix().
 *
 * For a finer-grained method, there is also the CtkSourceTag's
 * #CtkSourceTag:draw-spaces property.
 *
 * # Example
 *
 * To draw non-breaking spaces everywhere and draw all types of trailing spaces
 * except newlines:
 * |[
 * ctk_source_space_drawer_set_types_for_locations (space_drawer,
 *                                                  CTK_SOURCE_SPACE_LOCATION_ALL,
 *                                                  CTK_SOURCE_SPACE_TYPE_NBSP);
 *
 * ctk_source_space_drawer_set_types_for_locations (space_drawer,
 *                                                  CTK_SOURCE_SPACE_LOCATION_TRAILING,
 *                                                  CTK_SOURCE_SPACE_TYPE_ALL &
 *                                                  ~CTK_SOURCE_SPACE_TYPE_NEWLINE);
 *
 * ctk_source_space_drawer_set_enable_matrix (space_drawer, TRUE);
 * ]|
 *
 * # Use-case: draw unwanted white spaces
 *
 * A possible use-case is to draw only unwanted white spaces. Examples:
 * - Draw all trailing spaces.
 * - If the indentation and alignment must be done with spaces, draw tabs.
 *
 * And non-breaking spaces can always be drawn, everywhere, to distinguish them
 * from normal spaces.
 */

/* A drawer specially designed for the International Space Station. It comes by
 * default with a DVD of Matrix, in case the astronauts are bored.
 */

/*
#define ENABLE_PROFILE
*/
#undef ENABLE_PROFILE

struct _CtkSourceSpaceDrawerPrivate
{
	CtkSourceSpaceTypeFlags *matrix;
	GdkRGBA *color;
	guint enable_matrix : 1;
};

enum
{
	PROP_0,
	PROP_ENABLE_MATRIX,
	PROP_MATRIX,
	N_PROPERTIES
};

static GParamSpec *properties[N_PROPERTIES];

G_DEFINE_TYPE_WITH_PRIVATE (CtkSourceSpaceDrawer, ctk_source_space_drawer, G_TYPE_OBJECT)

static gint
get_number_of_locations (void)
{
	gint num;
	gint flags;

	num = 0;
	flags = CTK_SOURCE_SPACE_LOCATION_ALL;

	while (flags != 0)
	{
		flags >>= 1;
		num++;
	}

	return num;
}

static GVariant *
get_default_matrix (void)
{
	GVariantBuilder builder;
	gint num_locations;
	gint i;

	g_variant_builder_init (&builder, G_VARIANT_TYPE ("au"));

	num_locations = get_number_of_locations ();

	for (i = 0; i < num_locations; i++)
	{
		GVariant *space_types;

		space_types = g_variant_new_uint32 (CTK_SOURCE_SPACE_TYPE_ALL);

		g_variant_builder_add_value (&builder, space_types);
	}

	return g_variant_builder_end (&builder);
}

static gboolean
is_zero_matrix (CtkSourceSpaceDrawer *drawer)
{
	gint num_locations;
	gint i;

	num_locations = get_number_of_locations ();

	for (i = 0; i < num_locations; i++)
	{
		if (drawer->priv->matrix[i] != 0)
		{
			return FALSE;
		}
	}

	return TRUE;
}

static void
set_zero_matrix (CtkSourceSpaceDrawer *drawer)
{
	gint num_locations;
	gint i;
	gboolean changed = FALSE;

	num_locations = get_number_of_locations ();

	for (i = 0; i < num_locations; i++)
	{
		if (drawer->priv->matrix[i] != 0)
		{
			drawer->priv->matrix[i] = 0;
			changed = TRUE;
		}
	}

	if (changed)
	{
		g_object_notify_by_pspec (G_OBJECT (drawer), properties[PROP_MATRIX]);
	}
}

/* AND */
static CtkSourceSpaceTypeFlags
get_types_at_all_locations (CtkSourceSpaceDrawer        *drawer,
			    CtkSourceSpaceLocationFlags  locations)
{
	CtkSourceSpaceTypeFlags ret = CTK_SOURCE_SPACE_TYPE_ALL;
	gint index;
	gint num_locations;
	gboolean found;

	index = 0;
	num_locations = get_number_of_locations ();
	found = FALSE;

	while (locations != 0 && index < num_locations)
	{
		if ((locations & 1) == 1)
		{
			ret &= drawer->priv->matrix[index];
			found = TRUE;
		}

		locations >>= 1;
		index++;
	}

	return found ? ret : CTK_SOURCE_SPACE_TYPE_NONE;
}

/* OR */
static CtkSourceSpaceTypeFlags
get_types_at_any_locations (CtkSourceSpaceDrawer        *drawer,
			    CtkSourceSpaceLocationFlags  locations)
{
	CtkSourceSpaceTypeFlags ret = CTK_SOURCE_SPACE_TYPE_NONE;
	gint index;
	gint num_locations;

	index = 0;
	num_locations = get_number_of_locations ();

	while (locations != 0 && index < num_locations)
	{
		if ((locations & 1) == 1)
		{
			ret |= drawer->priv->matrix[index];
		}

		locations >>= 1;
		index++;
	}

	return ret;
}

static void
ctk_source_space_drawer_get_property (GObject    *object,
				      guint       prop_id,
				      GValue     *value,
				      GParamSpec *pspec)
{
	CtkSourceSpaceDrawer *drawer = CTK_SOURCE_SPACE_DRAWER (object);

	switch (prop_id)
	{
		case PROP_ENABLE_MATRIX:
			g_value_set_boolean (value, ctk_source_space_drawer_get_enable_matrix (drawer));
			break;

		case PROP_MATRIX:
			g_value_set_variant (value, ctk_source_space_drawer_get_matrix (drawer));
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
ctk_source_space_drawer_set_property (GObject      *object,
				      guint         prop_id,
				      const GValue *value,
				      GParamSpec   *pspec)
{
	CtkSourceSpaceDrawer *drawer = CTK_SOURCE_SPACE_DRAWER (object);

	switch (prop_id)
	{
		case PROP_ENABLE_MATRIX:
			ctk_source_space_drawer_set_enable_matrix (drawer, g_value_get_boolean (value));
			break;

		case PROP_MATRIX:
			ctk_source_space_drawer_set_matrix (drawer, g_value_get_variant (value));
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
ctk_source_space_drawer_finalize (GObject *object)
{
	CtkSourceSpaceDrawer *drawer = CTK_SOURCE_SPACE_DRAWER (object);

	g_free (drawer->priv->matrix);

	if (drawer->priv->color != NULL)
	{
		cdk_rgba_free (drawer->priv->color);
	}

	G_OBJECT_CLASS (ctk_source_space_drawer_parent_class)->finalize (object);
}

static void
ctk_source_space_drawer_class_init (CtkSourceSpaceDrawerClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->get_property = ctk_source_space_drawer_get_property;
	object_class->set_property = ctk_source_space_drawer_set_property;
	object_class->finalize = ctk_source_space_drawer_finalize;

	/**
	 * CtkSourceSpaceDrawer:enable-matrix:
	 *
	 * Whether the #CtkSourceSpaceDrawer:matrix property is enabled.
	 *
	 * Since: 3.24
	 */
	properties[PROP_ENABLE_MATRIX] =
		g_param_spec_boolean ("enable-matrix",
				      "Enable Matrix",
				      "",
				      FALSE,
				      G_PARAM_READWRITE |
				      G_PARAM_CONSTRUCT |
				      G_PARAM_STATIC_STRINGS);

	/**
	 * CtkSourceSpaceDrawer:matrix:
	 *
	 * The :matrix property is a #GVariant property to specify where and
	 * what kind of white spaces to draw.
	 *
	 * The #GVariant is of type `"au"`, an array of unsigned integers. Each
	 * integer is a combination of #CtkSourceSpaceTypeFlags. There is one
	 * integer for each #CtkSourceSpaceLocationFlags, in the same order as
	 * they are defined in the enum (%CTK_SOURCE_SPACE_LOCATION_NONE and
	 * %CTK_SOURCE_SPACE_LOCATION_ALL are not taken into account).
	 *
	 * If the array is shorter than the number of locations, then the value
	 * for the missing locations will be %CTK_SOURCE_SPACE_TYPE_NONE.
	 *
	 * By default, %CTK_SOURCE_SPACE_TYPE_ALL is set for all locations.
	 *
	 * Since: 3.24
	 */
	properties[PROP_MATRIX] =
		g_param_spec_variant ("matrix",
				      "Matrix",
				      "",
				      G_VARIANT_TYPE ("au"),
				      get_default_matrix (),
				      G_PARAM_READWRITE |
				      G_PARAM_CONSTRUCT |
				      G_PARAM_STATIC_STRINGS);

	g_object_class_install_properties (object_class, N_PROPERTIES, properties);
}

static void
ctk_source_space_drawer_init (CtkSourceSpaceDrawer *drawer)
{
	drawer->priv = ctk_source_space_drawer_get_instance_private (drawer);

	drawer->priv->matrix = g_new0 (CtkSourceSpaceTypeFlags, get_number_of_locations ());
}

/**
 * ctk_source_space_drawer_new:
 *
 * Creates a new #CtkSourceSpaceDrawer object. Useful for storing space drawing
 * settings independently of a #CtkSourceView.
 *
 * Returns: a new #CtkSourceSpaceDrawer.
 * Since: 3.24
 */
CtkSourceSpaceDrawer *
ctk_source_space_drawer_new (void)
{
	return g_object_new (CTK_SOURCE_TYPE_SPACE_DRAWER, NULL);
}

/**
 * ctk_source_space_drawer_get_types_for_locations:
 * @drawer: a #CtkSourceSpaceDrawer.
 * @locations: one or several #CtkSourceSpaceLocationFlags.
 *
 * If only one location is specified, this function returns what kind of
 * white spaces are drawn at that location. The value is retrieved from the
 * #CtkSourceSpaceDrawer:matrix property.
 *
 * If several locations are specified, this function returns the logical AND for
 * those locations. Which means that if a certain kind of white space is present
 * in the return value, then that kind of white space is drawn at all the
 * specified @locations.
 *
 * Returns: a combination of #CtkSourceSpaceTypeFlags.
 * Since: 3.24
 */
CtkSourceSpaceTypeFlags
ctk_source_space_drawer_get_types_for_locations (CtkSourceSpaceDrawer        *drawer,
						 CtkSourceSpaceLocationFlags  locations)
{
	g_return_val_if_fail (CTK_SOURCE_IS_SPACE_DRAWER (drawer), CTK_SOURCE_SPACE_TYPE_NONE);

	return get_types_at_all_locations (drawer, locations);
}

/**
 * ctk_source_space_drawer_set_types_for_locations:
 * @drawer: a #CtkSourceSpaceDrawer.
 * @locations: one or several #CtkSourceSpaceLocationFlags.
 * @types: a combination of #CtkSourceSpaceTypeFlags.
 *
 * Modifies the #CtkSourceSpaceDrawer:matrix property at the specified
 * @locations.
 *
 * Since: 3.24
 */
void
ctk_source_space_drawer_set_types_for_locations (CtkSourceSpaceDrawer        *drawer,
						 CtkSourceSpaceLocationFlags  locations,
						 CtkSourceSpaceTypeFlags      types)
{
	gint index;
	gint num_locations;
	gboolean changed = FALSE;

	g_return_if_fail (CTK_SOURCE_IS_SPACE_DRAWER (drawer));

	index = 0;
	num_locations = get_number_of_locations ();

	while (locations != 0 && index < num_locations)
	{
		if ((locations & 1) == 1 &&
		    drawer->priv->matrix[index] != types)
		{
			drawer->priv->matrix[index] = types;
			changed = TRUE;
		}

		locations >>= 1;
		index++;
	}

	if (changed)
	{
		g_object_notify_by_pspec (G_OBJECT (drawer), properties[PROP_MATRIX]);
	}
}

/**
 * ctk_source_space_drawer_get_matrix:
 * @drawer: a #CtkSourceSpaceDrawer.
 *
 * Gets the value of the #CtkSourceSpaceDrawer:matrix property, as a #GVariant.
 * An empty array can be returned in case the matrix is a zero matrix.
 *
 * The ctk_source_space_drawer_get_types_for_locations() function may be more
 * convenient to use.
 *
 * Returns: the #CtkSourceSpaceDrawer:matrix value as a new floating #GVariant
 *   instance.
 * Since: 3.24
 */
GVariant *
ctk_source_space_drawer_get_matrix (CtkSourceSpaceDrawer *drawer)
{
	GVariantBuilder builder;
	gint num_locations;
	gint i;

	g_return_val_if_fail (CTK_SOURCE_IS_SPACE_DRAWER (drawer), NULL);

	if (is_zero_matrix (drawer))
	{
		return g_variant_new ("au", NULL);
	}

	g_variant_builder_init (&builder, G_VARIANT_TYPE ("au"));

	num_locations = get_number_of_locations ();

	for (i = 0; i < num_locations; i++)
	{
		GVariant *space_types;

		space_types = g_variant_new_uint32 (drawer->priv->matrix[i]);

		g_variant_builder_add_value (&builder, space_types);
	}

	return g_variant_builder_end (&builder);
}

/**
 * ctk_source_space_drawer_set_matrix:
 * @drawer: a #CtkSourceSpaceDrawer.
 * @matrix: (transfer floating) (nullable): the new matrix value, or %NULL.
 *
 * Sets a new value to the #CtkSourceSpaceDrawer:matrix property, as a
 * #GVariant. If @matrix is %NULL, then an empty array is set.
 *
 * If @matrix is floating, it is consumed.
 *
 * The ctk_source_space_drawer_set_types_for_locations() function may be more
 * convenient to use.
 *
 * Since: 3.24
 */
void
ctk_source_space_drawer_set_matrix (CtkSourceSpaceDrawer *drawer,
				    GVariant             *matrix)
{
	gint num_locations;
	gint index;
	GVariantIter iter;
	gboolean changed = FALSE;

	g_return_if_fail (CTK_SOURCE_IS_SPACE_DRAWER (drawer));

	if (matrix == NULL)
	{
		set_zero_matrix (drawer);
		return;
	}

	g_return_if_fail (g_variant_is_of_type (matrix, G_VARIANT_TYPE ("au")));

	g_variant_iter_init (&iter, matrix);

	num_locations = get_number_of_locations ();
	index = 0;
	while (index < num_locations)
	{
		GVariant *child;
		guint32 space_types;

		child = g_variant_iter_next_value (&iter);
		if (child == NULL)
		{
			break;
		}

		space_types = g_variant_get_uint32 (child);

		if (drawer->priv->matrix[index] != space_types)
		{
			drawer->priv->matrix[index] = space_types;
			changed = TRUE;
		}

		g_variant_unref (child);
		index++;
	}

	while (index < num_locations)
	{
		if (drawer->priv->matrix[index] != 0)
		{
			drawer->priv->matrix[index] = 0;
			changed = TRUE;
		}

		index++;
	}

	if (changed)
	{
		g_object_notify_by_pspec (G_OBJECT (drawer), properties[PROP_MATRIX]);
	}

	if (g_variant_is_floating (matrix))
	{
		g_variant_ref_sink (matrix);
		g_variant_unref (matrix);
	}
}

/**
 * ctk_source_space_drawer_get_enable_matrix:
 * @drawer: a #CtkSourceSpaceDrawer.
 *
 * Returns: whether the #CtkSourceSpaceDrawer:matrix property is enabled.
 * Since: 3.24
 */
gboolean
ctk_source_space_drawer_get_enable_matrix (CtkSourceSpaceDrawer *drawer)
{
	g_return_val_if_fail (CTK_SOURCE_IS_SPACE_DRAWER (drawer), FALSE);

	return drawer->priv->enable_matrix;
}

/**
 * ctk_source_space_drawer_set_enable_matrix:
 * @drawer: a #CtkSourceSpaceDrawer.
 * @enable_matrix: the new value.
 *
 * Sets whether the #CtkSourceSpaceDrawer:matrix property is enabled.
 *
 * Since: 3.24
 */
void
ctk_source_space_drawer_set_enable_matrix (CtkSourceSpaceDrawer *drawer,
					   gboolean              enable_matrix)
{
	g_return_if_fail (CTK_SOURCE_IS_SPACE_DRAWER (drawer));

	enable_matrix = enable_matrix != FALSE;

	if (drawer->priv->enable_matrix != enable_matrix)
	{
		drawer->priv->enable_matrix = enable_matrix;
		g_object_notify_by_pspec (G_OBJECT (drawer), properties[PROP_ENABLE_MATRIX]);
	}
}

static gboolean
matrix_get_mapping (GValue   *value,
		    GVariant *variant,
		    gpointer  user_data)
{
	g_value_set_variant (value, variant);
	return TRUE;
}

static GVariant *
matrix_set_mapping (const GValue       *value,
		    const GVariantType *expected_type,
		    gpointer            user_data)
{
	return g_value_dup_variant (value);
}

/**
 * ctk_source_space_drawer_bind_matrix_setting:
 * @drawer: a #CtkSourceSpaceDrawer object.
 * @settings: a #GSettings object.
 * @key: the @settings key to bind.
 * @flags: flags for the binding.
 *
 * Binds the #CtkSourceSpaceDrawer:matrix property to a #GSettings key.
 *
 * The #GSettings key must be of the same type as the
 * #CtkSourceSpaceDrawer:matrix property, that is, `"au"`.
 *
 * The g_settings_bind() function cannot be used, because the default GIO
 * mapping functions don't support #GVariant properties (maybe it will be
 * supported by a future GIO version, in which case this function can be
 * deprecated).
 *
 * Since: 3.24
 */
void
ctk_source_space_drawer_bind_matrix_setting (CtkSourceSpaceDrawer *drawer,
					     GSettings            *settings,
					     const gchar          *key,
					     GSettingsBindFlags    flags)
{
	GVariant *value;

	g_return_if_fail (CTK_SOURCE_IS_SPACE_DRAWER (drawer));
	g_return_if_fail (G_IS_SETTINGS (settings));
	g_return_if_fail (key != NULL);
	g_return_if_fail ((flags & G_SETTINGS_BIND_INVERT_BOOLEAN) == 0);

	value = g_settings_get_value (settings, key);
	if (!g_variant_is_of_type (value, G_VARIANT_TYPE ("au")))
	{
		g_warning ("%s(): the GSettings key must be of type \"au\".", G_STRFUNC);
		g_variant_unref (value);
		return;
	}
	g_variant_unref (value);

	g_settings_bind_with_mapping (settings, key,
				      drawer, "matrix",
				      flags,
				      matrix_get_mapping,
				      matrix_set_mapping,
				      NULL, NULL);
}

void
_ctk_source_space_drawer_update_color (CtkSourceSpaceDrawer *drawer,
				       CtkSourceView        *view)
{
	CtkSourceBuffer *buffer;
	CtkSourceStyleScheme *style_scheme;

	g_return_if_fail (CTK_SOURCE_IS_SPACE_DRAWER (drawer));
	g_return_if_fail (CTK_SOURCE_IS_VIEW (view));

	if (drawer->priv->color != NULL)
	{
		cdk_rgba_free (drawer->priv->color);
		drawer->priv->color = NULL;
	}

	buffer = CTK_SOURCE_BUFFER (ctk_text_view_get_buffer (CTK_TEXT_VIEW (view)));
	style_scheme = ctk_source_buffer_get_style_scheme (buffer);

	if (style_scheme != NULL)
	{
		CtkSourceStyle *style;

		style = _ctk_source_style_scheme_get_draw_spaces_style (style_scheme);

		if (style != NULL)
		{
			gchar *color_str = NULL;
			gboolean color_set;
			GdkRGBA color;

			g_object_get (style,
				      "foreground", &color_str,
				      "foreground-set", &color_set,
				      NULL);

			if (color_set &&
			    color_str != NULL &&
			    cdk_rgba_parse (&color, color_str))
			{
				drawer->priv->color = cdk_rgba_copy (&color);
			}

			g_free (color_str);
		}
	}

	if (drawer->priv->color == NULL)
	{
		CtkStyleContext *context;
		GdkRGBA color;

		context = ctk_widget_get_style_context (CTK_WIDGET (view));
		ctk_style_context_save (context);
		ctk_style_context_set_state (context, CTK_STATE_FLAG_INSENSITIVE);
		ctk_style_context_get_color (context,
					     ctk_style_context_get_state (context),
					     &color);
		ctk_style_context_restore (context);

		drawer->priv->color = cdk_rgba_copy (&color);
	}
}

static inline gboolean
is_tab (gunichar ch)
{
	return ch == '\t';
}

static inline gboolean
is_nbsp (gunichar ch)
{
	return g_unichar_break_type (ch) == G_UNICODE_BREAK_NON_BREAKING_GLUE;
}

static inline gboolean
is_narrowed_nbsp (gunichar ch)
{
	return ch == 0x202F;
}

static inline gboolean
is_space (gunichar ch)
{
	return g_unichar_type (ch) == G_UNICODE_SPACE_SEPARATOR;
}

static gboolean
is_newline (const CtkTextIter *iter)
{
	if (ctk_text_iter_is_end (iter))
	{
		CtkSourceBuffer *buffer;

		buffer = CTK_SOURCE_BUFFER (ctk_text_iter_get_buffer (iter));

		return ctk_source_buffer_get_implicit_trailing_newline (buffer);
	}

	return ctk_text_iter_ends_line (iter);
}

static inline gboolean
is_whitespace (gunichar ch)
{
	return (g_unichar_isspace (ch) || is_nbsp (ch) || is_space (ch));
}

static void
draw_space_at_pos (cairo_t      *cr,
		   GdkRectangle  rect)
{
	gint x, y;
	gdouble w;

	x = rect.x;
	y = rect.y + rect.height * 2 / 3;

	w = rect.width;

	cairo_save (cr);
	cairo_move_to (cr, x + w * 0.5, y);
	cairo_arc (cr, x + w * 0.5, y, 0.8, 0, 2 * G_PI);
	cairo_stroke (cr);
	cairo_restore (cr);
}

static void
draw_tab_at_pos (cairo_t      *cr,
		 GdkRectangle  rect)
{
	gint x, y;
	gdouble w, h;

	x = rect.x;
	y = rect.y + rect.height * 2 / 3;

	w = rect.width;
	h = rect.height;

	cairo_save (cr);
	cairo_move_to (cr, x + h * 1 / 6, y);
	cairo_rel_line_to (cr, w - h * 2 / 6, 0);
	cairo_rel_line_to (cr, -h * 1 / 4, -h * 1 / 4);
	cairo_rel_move_to (cr, +h * 1 / 4, +h * 1 / 4);
	cairo_rel_line_to (cr, -h * 1 / 4, +h * 1 / 4);
	cairo_stroke (cr);
	cairo_restore (cr);
}

static void
draw_newline_at_pos (cairo_t      *cr,
		     GdkRectangle  rect)
{
	gint x, y;
	gdouble w, h;

	x = rect.x;
	y = rect.y + rect.height / 3;

	w = 2 * rect.width;
	h = rect.height;

	cairo_save (cr);

	if (ctk_widget_get_default_direction () == CTK_TEXT_DIR_LTR)
	{
		cairo_move_to (cr, x + w * 7 / 8, y);
		cairo_rel_line_to (cr, 0, h * 1 / 3);
		cairo_rel_line_to (cr, -w * 6 / 8, 0);
		cairo_rel_line_to (cr, +h * 1 / 4, -h * 1 / 4);
		cairo_rel_move_to (cr, -h * 1 / 4, +h * 1 / 4);
		cairo_rel_line_to (cr, +h * 1 / 4, +h * 1 / 4);
	}
	else
	{
		cairo_move_to (cr, x + w * 1 / 8, y);
		cairo_rel_line_to (cr, 0, h * 1 / 3);
		cairo_rel_line_to (cr, w * 6 / 8, 0);
		cairo_rel_line_to (cr, -h * 1 / 4, -h * 1 / 4);
		cairo_rel_move_to (cr, +h * 1 / 4, +h * 1 / 4);
		cairo_rel_line_to (cr, -h * 1 / 4, -h * 1 / 4);
	}

	cairo_stroke (cr);
	cairo_restore (cr);
}

static void
draw_nbsp_at_pos (cairo_t      *cr,
		  GdkRectangle  rect,
		  gboolean      narrowed)
{
	gint x, y;
	gdouble w, h;

	x = rect.x;
	y = rect.y + rect.height / 2;

	w = rect.width;
	h = rect.height;

	cairo_save (cr);
	cairo_move_to (cr, x + w * 1 / 6, y);
	cairo_rel_line_to (cr, w * 4 / 6, 0);
	cairo_rel_line_to (cr, -w * 2 / 6, +h * 1 / 4);
	cairo_rel_line_to (cr, -w * 2 / 6, -h * 1 / 4);

	if (narrowed)
	{
		cairo_fill (cr);
	}
	else
	{
		cairo_stroke (cr);
	}

	cairo_restore (cr);
}

static void
draw_whitespace_at_iter (CtkTextView *text_view,
			 CtkTextIter *iter,
			 cairo_t     *cr)
{
	gunichar ch;
	GdkRectangle rect;

	ctk_text_view_get_iter_location (text_view, iter, &rect);

	/* If the space is at a line-wrap position, or if the character is a
	 * newline, we get 0 width so we fallback to the height.
	 */
	if (rect.width == 0)
	{
		rect.width = rect.height;
	}

	ch = ctk_text_iter_get_char (iter);

	if (is_tab (ch))
	{
		draw_tab_at_pos (cr, rect);
	}
	else if (is_nbsp (ch))
	{
		draw_nbsp_at_pos (cr, rect, is_narrowed_nbsp (ch));
	}
	else if (is_space (ch))
	{
		draw_space_at_pos (cr, rect);
	}
	else if (is_newline (iter))
	{
		draw_newline_at_pos (cr, rect);
	}
}

static void
space_needs_drawing_according_to_tag (const CtkTextIter *iter,
				      gboolean          *has_tag,
				      gboolean          *needs_drawing)
{
	GSList *tags;
	GSList *l;

	*has_tag = FALSE;
	*needs_drawing = FALSE;

	tags = ctk_text_iter_get_tags (iter);
	tags = g_slist_reverse (tags);

	for (l = tags; l != NULL; l = l->next)
	{
		CtkTextTag *tag = l->data;

		if (CTK_SOURCE_IS_TAG (tag))
		{
			gboolean draw_spaces_set;
			gboolean draw_spaces;

			g_object_get (tag,
				      "draw-spaces-set", &draw_spaces_set,
				      "draw-spaces", &draw_spaces,
				      NULL);

			if (draw_spaces_set)
			{
				*has_tag = TRUE;
				*needs_drawing = draw_spaces;
				break;
			}
		}
	}

	g_slist_free (tags);
}

static CtkSourceSpaceLocationFlags
get_iter_locations (const CtkTextIter *iter,
		    const CtkTextIter *leading_end,
		    const CtkTextIter *trailing_start)
{
	CtkSourceSpaceLocationFlags iter_locations = CTK_SOURCE_SPACE_LOCATION_NONE;

	if (ctk_text_iter_compare (iter, leading_end) < 0)
	{
		iter_locations |= CTK_SOURCE_SPACE_LOCATION_LEADING;
	}

	if (ctk_text_iter_compare (trailing_start, iter) <= 0)
	{
		iter_locations |= CTK_SOURCE_SPACE_LOCATION_TRAILING;
	}

	/* Neither leading nor trailing, must be in text. */
	if (iter_locations == CTK_SOURCE_SPACE_LOCATION_NONE)
	{
		iter_locations = CTK_SOURCE_SPACE_LOCATION_INSIDE_TEXT;
	}

	return iter_locations;
}

static CtkSourceSpaceTypeFlags
get_iter_space_type (const CtkTextIter *iter)
{
	gunichar ch;

	ch = ctk_text_iter_get_char (iter);

	if (is_tab (ch))
	{
		return CTK_SOURCE_SPACE_TYPE_TAB;
	}
	else if (is_nbsp (ch))
	{
		return CTK_SOURCE_SPACE_TYPE_NBSP;
	}
	else if (is_space (ch))
	{
		return CTK_SOURCE_SPACE_TYPE_SPACE;
	}
	else if (is_newline (iter))
	{
		return CTK_SOURCE_SPACE_TYPE_NEWLINE;
	}

	return CTK_SOURCE_SPACE_TYPE_NONE;
}

static gboolean
space_needs_drawing_according_to_matrix (CtkSourceSpaceDrawer *drawer,
					 const CtkTextIter    *iter,
					 const CtkTextIter    *leading_end,
					 const CtkTextIter    *trailing_start)
{
	CtkSourceSpaceLocationFlags iter_locations;
	CtkSourceSpaceTypeFlags iter_space_type;
	CtkSourceSpaceTypeFlags allowed_space_types;

	iter_locations = get_iter_locations (iter, leading_end, trailing_start);
	iter_space_type = get_iter_space_type (iter);
	allowed_space_types = get_types_at_any_locations (drawer, iter_locations);

	return (iter_space_type & allowed_space_types) != 0;
}

static gboolean
space_needs_drawing (CtkSourceSpaceDrawer *drawer,
		     const CtkTextIter    *iter,
		     const CtkTextIter    *leading_end,
		     const CtkTextIter    *trailing_start)
{
	gboolean has_tag;
	gboolean needs_drawing;

	/* Check the CtkSourceTag:draw-spaces property (higher priority) */
	space_needs_drawing_according_to_tag (iter, &has_tag, &needs_drawing);
	if (has_tag)
	{
		return needs_drawing;
	}

	/* Check the matrix */
	return (drawer->priv->enable_matrix &&
		space_needs_drawing_according_to_matrix (drawer, iter, leading_end, trailing_start));
}

static void
get_line_end (CtkTextView       *text_view,
	      const CtkTextIter *start_iter,
	      CtkTextIter       *line_end,
	      gint               max_x,
	      gint               max_y,
	      gboolean           is_wrapping)
{
	gint min;
	gint max;
	GdkRectangle rect;

	*line_end = *start_iter;
	if (!ctk_text_iter_ends_line (line_end))
	{
		ctk_text_iter_forward_to_line_end (line_end);
	}

	/* Check if line_end is inside the bounding box anyway. */
	ctk_text_view_get_iter_location (text_view, line_end, &rect);
	if (( is_wrapping && rect.y < max_y) ||
	    (!is_wrapping && rect.x < max_x))
	{
		return;
	}

	min = ctk_text_iter_get_line_offset (start_iter);
	max = ctk_text_iter_get_line_offset (line_end);

	while (max >= min)
	{
		gint i;

		i = (min + max) >> 1;
		ctk_text_iter_set_line_offset (line_end, i);
		ctk_text_view_get_iter_location (text_view, line_end, &rect);

		if (( is_wrapping && rect.y < max_y) ||
		    (!is_wrapping && rect.x < max_x))
		{
			min = i + 1;
		}
		else if (( is_wrapping && rect.y > max_y) ||
			 (!is_wrapping && rect.x > max_x))
		{
			max = i - 1;
		}
		else
		{
			break;
		}
	}
}

void
_ctk_source_space_drawer_draw (CtkSourceSpaceDrawer *drawer,
			       CtkSourceView        *view,
			       cairo_t              *cr)
{
	CtkTextView *text_view;
	CtkTextBuffer *buffer;
	GdkRectangle clip;
	gint min_x;
	gint min_y;
	gint max_x;
	gint max_y;
	CtkTextIter start;
	CtkTextIter end;
	CtkTextIter iter;
	CtkTextIter leading_end;
	CtkTextIter trailing_start;
	CtkTextIter line_end;
	gboolean is_wrapping;

#ifdef ENABLE_PROFILE
	static GTimer *timer = NULL;
	if (timer == NULL)
	{
		timer = g_timer_new ();
	}

	g_timer_start (timer);
#endif

	g_return_if_fail (CTK_SOURCE_IS_SPACE_DRAWER (drawer));
	g_return_if_fail (CTK_SOURCE_IS_VIEW (view));
	g_return_if_fail (cr != NULL);

	if (drawer->priv->color == NULL)
	{
		g_warning ("CtkSourceSpaceDrawer: color not set.");
		return;
	}

	text_view = CTK_TEXT_VIEW (view);
	buffer = ctk_text_view_get_buffer (text_view);

	if ((!drawer->priv->enable_matrix || is_zero_matrix (drawer)) &&
	    !_ctk_source_buffer_has_spaces_tag (CTK_SOURCE_BUFFER (buffer)))
	{
		return;
	}

	if (!cdk_cairo_get_clip_rectangle (cr, &clip))
	{
		return;
	}

	is_wrapping = ctk_text_view_get_wrap_mode (text_view) != CTK_WRAP_NONE;

	min_x = clip.x;
	min_y = clip.y;
	max_x = min_x + clip.width;
	max_y = min_y + clip.height;

	ctk_text_view_get_iter_at_location (text_view, &start, min_x, min_y);
	ctk_text_view_get_iter_at_location (text_view, &end, max_x, max_y);

	cairo_save (cr);
	cdk_cairo_set_source_rgba (cr, drawer->priv->color);
	cairo_set_line_width (cr, 0.8);
	cairo_translate (cr, -0.5, -0.5);

	iter = start;
	_ctk_source_iter_get_leading_spaces_end_boundary (&iter, &leading_end);
	_ctk_source_iter_get_trailing_spaces_start_boundary (&iter, &trailing_start);
	get_line_end (text_view, &iter, &line_end, max_x, max_y, is_wrapping);

	while (TRUE)
	{
		gunichar ch = ctk_text_iter_get_char (&iter);
		gint ly;

		/* Allow end iter, to draw implicit trailing newline. */
		if ((is_whitespace (ch) || ctk_text_iter_is_end (&iter)) &&
		    space_needs_drawing (drawer, &iter, &leading_end, &trailing_start))
		{
			draw_whitespace_at_iter (text_view, &iter, cr);
		}

		if (ctk_text_iter_is_end (&iter) ||
		    ctk_text_iter_compare (&iter, &end) >= 0)
		{
			break;
		}

		ctk_text_iter_forward_char (&iter);

		if (ctk_text_iter_compare (&iter, &line_end) > 0)
		{
			CtkTextIter next_iter = iter;

			/* Move to the first iter in the exposed area of the
			 * next line.
			 */
			if (!ctk_text_iter_starts_line (&next_iter))
			{
				/* We're trying to move forward on the last
				 * line of the buffer, so we can stop now.
				 */
				if (!ctk_text_iter_forward_line (&next_iter))
				{
					break;
				}
			}

			ctk_text_view_get_line_yrange (text_view, &next_iter, &ly, NULL);
			ctk_text_view_get_iter_at_location (text_view, &next_iter, min_x, ly);

			/* Move back one char otherwise tabs may not be redrawn. */
			if (!ctk_text_iter_starts_line (&next_iter))
			{
				ctk_text_iter_backward_char (&next_iter);
			}

			/* Ensure that we have actually advanced, since the
			 * above backward_char() is dangerous and can lead to
			 * infinite loops.
			 */
			if (ctk_text_iter_compare (&next_iter, &iter) > 0)
			{
				iter = next_iter;
			}

			_ctk_source_iter_get_leading_spaces_end_boundary (&iter, &leading_end);
			_ctk_source_iter_get_trailing_spaces_start_boundary (&iter, &trailing_start);
			get_line_end (text_view, &iter, &line_end, max_x, max_y, is_wrapping);
		}
	};

	cairo_restore (cr);

#ifdef ENABLE_PROFILE
	g_timer_stop (timer);

	/* Same indentation as similar features in ctksourceview.c. */
	g_print ("    %s time: %g (sec * 1000)\n",
		 G_STRFUNC,
		 g_timer_elapsed (timer, NULL) * 1000);
#endif
}
