/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * ctksourceregion.c - CtkTextMark-based region utility
 * This file is part of CtkSourceView
 *
 * Copyright (C) 2002 Gustavo Giráldez <gustavo.giraldez@gmx.net>
 * Copyright (C) 2016 Sébastien Wilmet <swilmet@gnome.org>
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

#include "ctksourceregion.h"

/**
 * SECTION:region
 * @Short_description: Region utility
 * @Title: CtkSourceRegion
 * @See_also: #CtkTextBuffer
 *
 * A #CtkSourceRegion permits to store a group of subregions of a
 * #CtkTextBuffer. #CtkSourceRegion stores the subregions with pairs of
 * #CtkTextMark's, so the region is still valid after insertions and deletions
 * in the #CtkTextBuffer.
 *
 * The #CtkTextMark for the start of a subregion has a left gravity, while the
 * #CtkTextMark for the end of a subregion has a right gravity.
 *
 * The typical use-case of #CtkSourceRegion is to scan a #CtkTextBuffer chunk by
 * chunk, not the whole buffer at once to not block the user interface. The
 * #CtkSourceRegion represents in that case the remaining region to scan. You
 * can listen to the #CtkTextBuffer::insert-text and
 * #CtkTextBuffer::delete-range signals to update the #CtkSourceRegion
 * accordingly.
 *
 * To iterate through the subregions, you need to use a #CtkSourceRegionIter,
 * for example:
 * |[
 * CtkSourceRegion *region;
 * CtkSourceRegionIter region_iter;
 *
 * ctk_source_region_get_start_region_iter (region, &region_iter);
 *
 * while (!ctk_source_region_iter_is_end (&region_iter))
 * {
 *         CtkTextIter subregion_start;
 *         CtkTextIter subregion_end;
 *
 *         if (!ctk_source_region_iter_get_subregion (&region_iter,
 *                                                    &subregion_start,
 *                                                    &subregion_end))
 *         {
 *                 break;
 *         }
 *
 *         // Do something useful with the subregion.
 *
 *         ctk_source_region_iter_next (&region_iter);
 * }
 * ]|
 */

/* With the gravities of the CtkTextMarks, it is possible for subregions to
 * become interlaced:
 * Buffer content:
 *   "hello world"
 * Add two subregions:
 *   "[hello] [world]"
 * Delete the space:
 *   "[hello][world]"
 * Undo:
 *   "[hello[ ]world]"
 *
 * FIXME: when iterating through the subregions, it should simplify them first.
 * I don't know if it's done (swilmet).
 */

#undef ENABLE_DEBUG
/*
#define ENABLE_DEBUG
*/

#ifdef ENABLE_DEBUG
#define DEBUG(x) (x)
#else
#define DEBUG(x)
#endif

typedef struct _CtkSourceRegionPrivate CtkSourceRegionPrivate;
typedef struct _Subregion Subregion;
typedef struct _CtkSourceRegionIterReal CtkSourceRegionIterReal;

struct _CtkSourceRegionPrivate
{
	/* Weak pointer to the buffer. */
	CtkTextBuffer *buffer;

	/* List of sorted 'Subregion*' */
	GList *subregions;

	guint32 timestamp;
};

struct _Subregion
{
	CtkTextMark *start;
	CtkTextMark *end;
};

struct _CtkSourceRegionIterReal
{
	CtkSourceRegion *region;
	guint32 region_timestamp;
	GList *subregions;
};

enum
{
	PROP_0,
	PROP_BUFFER,
	N_PROPERTIES
};

static GParamSpec *properties[N_PROPERTIES];

G_DEFINE_TYPE_WITH_PRIVATE (CtkSourceRegion, ctk_source_region, G_TYPE_OBJECT)

#ifdef ENABLE_DEBUG
static void
print_region (CtkSourceRegion *region)
{
	gchar *str;

	str = ctk_source_region_to_string (region);
	g_print ("%s\n", str);
	g_free (str);
}
#endif

/* Find and return a subregion node which contains the given text
 * iter.  If left_side is TRUE, return the subregion which contains
 * the text iter or which is the leftmost; else return the rightmost
 * subregion.
 */
static GList *
find_nearest_subregion (CtkSourceRegion   *region,
			const CtkTextIter *iter,
			GList             *begin,
			gboolean           leftmost,
			gboolean           include_edges)
{
	CtkSourceRegionPrivate *priv = ctk_source_region_get_instance_private (region);
	GList *retval;
	GList *l;

	g_assert (iter != NULL);

	if (begin == NULL)
	{
		begin = priv->subregions;
	}

	if (begin != NULL)
	{
		retval = begin->prev;
	}
	else
	{
		retval = NULL;
	}

	for (l = begin; l != NULL; l = l->next)
	{
		CtkTextIter sr_iter;
		Subregion *sr = l->data;
		gint cmp;

		if (!leftmost)
		{
			ctk_text_buffer_get_iter_at_mark (priv->buffer, &sr_iter, sr->end);
			cmp = ctk_text_iter_compare (iter, &sr_iter);
			if (cmp < 0 || (cmp == 0 && include_edges))
			{
				retval = l;
				break;
			}

		}
		else
		{
			ctk_text_buffer_get_iter_at_mark (priv->buffer, &sr_iter, sr->start);
			cmp = ctk_text_iter_compare (iter, &sr_iter);
			if (cmp > 0 || (cmp == 0 && include_edges))
			{
				retval = l;
			}
			else
			{
				break;
			}
		}
	}

	return retval;
}

static void
ctk_source_region_get_property (GObject    *object,
				guint       prop_id,
				GValue     *value,
				GParamSpec *pspec)
{
	CtkSourceRegion *region = CTK_SOURCE_REGION (object);

	switch (prop_id)
	{
		case PROP_BUFFER:
			g_value_set_object (value, ctk_source_region_get_buffer (region));
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
ctk_source_region_set_property (GObject      *object,
				guint         prop_id,
				const GValue *value,
				GParamSpec   *pspec)
{
	CtkSourceRegionPrivate *priv = ctk_source_region_get_instance_private (CTK_SOURCE_REGION (object));

	switch (prop_id)
	{
		case PROP_BUFFER:
			g_assert (priv->buffer == NULL);
			priv->buffer = g_value_get_object (value);
			g_object_add_weak_pointer (G_OBJECT (priv->buffer),
						   (gpointer *) &priv->buffer);
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
ctk_source_region_dispose (GObject *object)
{
	CtkSourceRegionPrivate *priv = ctk_source_region_get_instance_private (CTK_SOURCE_REGION (object));

	while (priv->subregions != NULL)
	{
		Subregion *sr = priv->subregions->data;

		if (priv->buffer != NULL)
		{
			ctk_text_buffer_delete_mark (priv->buffer, sr->start);
			ctk_text_buffer_delete_mark (priv->buffer, sr->end);
		}

		g_slice_free (Subregion, sr);
		priv->subregions = g_list_delete_link (priv->subregions, priv->subregions);
	}

	if (priv->buffer != NULL)
	{
		g_object_remove_weak_pointer (G_OBJECT (priv->buffer),
					      (gpointer *) &priv->buffer);

		priv->buffer = NULL;
	}

	G_OBJECT_CLASS (ctk_source_region_parent_class)->dispose (object);
}

static void
ctk_source_region_class_init (CtkSourceRegionClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->get_property = ctk_source_region_get_property;
	object_class->set_property = ctk_source_region_set_property;
	object_class->dispose = ctk_source_region_dispose;

	/**
	 * CtkSourceRegion:buffer:
	 *
	 * The #CtkTextBuffer. The #CtkSourceRegion has a weak reference to the
	 * buffer.
	 *
	 * Since: 3.22
	 */
	properties[PROP_BUFFER] =
		g_param_spec_object ("buffer",
				     "Buffer",
				     "",
				     CTK_TYPE_TEXT_BUFFER,
				     G_PARAM_READWRITE |
				     G_PARAM_CONSTRUCT_ONLY |
				     G_PARAM_STATIC_STRINGS);

	g_object_class_install_properties (object_class, N_PROPERTIES, properties);
}

static void
ctk_source_region_init (CtkSourceRegion *region)
{
}

/**
 * ctk_source_region_new:
 * @buffer: a #CtkTextBuffer.
 *
 * Returns: a new #CtkSourceRegion object for @buffer.
 * Since: 3.22
 */
CtkSourceRegion *
ctk_source_region_new (CtkTextBuffer *buffer)
{
	g_return_val_if_fail (CTK_IS_TEXT_BUFFER (buffer), NULL);

	return g_object_new (CTK_SOURCE_TYPE_REGION,
			     "buffer", buffer,
			     NULL);
}

/**
 * ctk_source_region_get_buffer:
 * @region: a #CtkSourceRegion.
 *
 * Returns: (transfer none) (nullable): the #CtkTextBuffer.
 * Since: 3.22
 */
CtkTextBuffer *
ctk_source_region_get_buffer (CtkSourceRegion *region)
{
	CtkSourceRegionPrivate *priv;

	g_return_val_if_fail (CTK_SOURCE_IS_REGION (region), NULL);

	priv = ctk_source_region_get_instance_private (region);
	return priv->buffer;
}

static void
ctk_source_region_clear_zero_length_subregions (CtkSourceRegion *region)
{
	CtkSourceRegionPrivate *priv = ctk_source_region_get_instance_private (region);
	GList *node;

	node = priv->subregions;
	while (node != NULL)
	{
		Subregion *sr = node->data;
		CtkTextIter start;
		CtkTextIter end;

		ctk_text_buffer_get_iter_at_mark (priv->buffer, &start, sr->start);
		ctk_text_buffer_get_iter_at_mark (priv->buffer, &end, sr->end);

		if (ctk_text_iter_equal (&start, &end))
		{
			ctk_text_buffer_delete_mark (priv->buffer, sr->start);
			ctk_text_buffer_delete_mark (priv->buffer, sr->end);
			g_slice_free (Subregion, sr);

			if (node == priv->subregions)
			{
				priv->subregions = node = g_list_delete_link (node, node);
			}
			else
			{
				node = g_list_delete_link (node, node);
			}

			priv->timestamp++;
		}
		else
		{
			node = node->next;
		}
	}
}

/**
 * ctk_source_region_add_subregion:
 * @region: a #CtkSourceRegion.
 * @_start: the start of the subregion.
 * @_end: the end of the subregion.
 *
 * Adds the subregion delimited by @_start and @_end to @region.
 *
 * Since: 3.22
 */
void
ctk_source_region_add_subregion (CtkSourceRegion   *region,
				 const CtkTextIter *_start,
				 const CtkTextIter *_end)
{
	CtkSourceRegionPrivate *priv;
	GList *start_node;
	GList *end_node;
	CtkTextIter start;
	CtkTextIter end;

	g_return_if_fail (CTK_SOURCE_IS_REGION (region));
	g_return_if_fail (_start != NULL);
	g_return_if_fail (_end != NULL);

	priv = ctk_source_region_get_instance_private (region);

	if (priv->buffer == NULL)
	{
		return;
	}

	start = *_start;
	end = *_end;

	DEBUG (g_print ("---\n"));
	DEBUG (print_region (region));
	DEBUG (g_message ("region_add (%d, %d)",
			  ctk_text_iter_get_offset (&start),
			  ctk_text_iter_get_offset (&end)));

	ctk_text_iter_order (&start, &end);

	/* Don't add zero-length regions. */
	if (ctk_text_iter_equal (&start, &end))
	{
		return;
	}

	/* Find bounding subregions. */
	start_node = find_nearest_subregion (region, &start, NULL, FALSE, TRUE);
	end_node = find_nearest_subregion (region, &end, start_node, TRUE, TRUE);

	if (start_node == NULL || end_node == NULL || end_node == start_node->prev)
	{
		/* Create the new subregion. */
		Subregion *sr = g_slice_new0 (Subregion);
		sr->start = ctk_text_buffer_create_mark (priv->buffer, NULL, &start, TRUE);
		sr->end = ctk_text_buffer_create_mark (priv->buffer, NULL, &end, FALSE);

		if (start_node == NULL)
		{
			/* Append the new region. */
			priv->subregions = g_list_append (priv->subregions, sr);
		}
		else if (end_node == NULL)
		{
			/* Prepend the new region. */
			priv->subregions = g_list_prepend (priv->subregions, sr);
		}
		else
		{
			/* We are in the middle of two subregions. */
			priv->subregions = g_list_insert_before (priv->subregions, start_node, sr);
		}
	}
	else
	{
		CtkTextIter iter;
		Subregion *sr = start_node->data;

		if (start_node != end_node)
		{
			/* We need to merge some subregions. */
			GList *l = start_node->next;
			Subregion *q;

			ctk_text_buffer_delete_mark (priv->buffer, sr->end);

			while (l != end_node)
			{
				q = l->data;
				ctk_text_buffer_delete_mark (priv->buffer, q->start);
				ctk_text_buffer_delete_mark (priv->buffer, q->end);
				g_slice_free (Subregion, q);
				l = g_list_delete_link (l, l);
			}

			q = l->data;
			ctk_text_buffer_delete_mark (priv->buffer, q->start);
			sr->end = q->end;
			g_slice_free (Subregion, q);
			l = g_list_delete_link (l, l);
		}

		/* Now move marks if that action expands the region. */
		ctk_text_buffer_get_iter_at_mark (priv->buffer, &iter, sr->start);
		if (ctk_text_iter_compare (&iter, &start) > 0)
		{
			ctk_text_buffer_move_mark (priv->buffer, sr->start, &start);
		}

		ctk_text_buffer_get_iter_at_mark (priv->buffer, &iter, sr->end);
		if (ctk_text_iter_compare (&iter, &end) < 0)
		{
			ctk_text_buffer_move_mark (priv->buffer, sr->end, &end);
		}
	}

	priv->timestamp++;

	DEBUG (print_region (region));
}

/**
 * ctk_source_region_add_region:
 * @region: a #CtkSourceRegion.
 * @region_to_add: (nullable): the #CtkSourceRegion to add to @region, or %NULL.
 *
 * Adds @region_to_add to @region. @region_to_add is not modified.
 *
 * Since: 3.22
 */
void
ctk_source_region_add_region (CtkSourceRegion *region,
			      CtkSourceRegion *region_to_add)
{
	CtkSourceRegionIter iter;
	CtkTextBuffer *region_buffer;
	CtkTextBuffer *region_to_add_buffer;

	g_return_if_fail (CTK_SOURCE_IS_REGION (region));
	g_return_if_fail (region_to_add == NULL || CTK_SOURCE_IS_REGION (region_to_add));

	if (region_to_add == NULL)
	{
		return;
	}

	region_buffer = ctk_source_region_get_buffer (region);
	region_to_add_buffer = ctk_source_region_get_buffer (region_to_add);
	g_return_if_fail (region_buffer == region_to_add_buffer);

	if (region_buffer == NULL)
	{
		return;
	}

	ctk_source_region_get_start_region_iter (region_to_add, &iter);

	while (!ctk_source_region_iter_is_end (&iter))
	{
		CtkTextIter subregion_start;
		CtkTextIter subregion_end;

		if (!ctk_source_region_iter_get_subregion (&iter,
							   &subregion_start,
							   &subregion_end))
		{
			break;
		}

		ctk_source_region_add_subregion (region,
						 &subregion_start,
						 &subregion_end);

		ctk_source_region_iter_next (&iter);
	}
}

/**
 * ctk_source_region_subtract_subregion:
 * @region: a #CtkSourceRegion.
 * @_start: the start of the subregion.
 * @_end: the end of the subregion.
 *
 * Subtracts the subregion delimited by @_start and @_end from @region.
 *
 * Since: 3.22
 */
void
ctk_source_region_subtract_subregion (CtkSourceRegion   *region,
				      const CtkTextIter *_start,
				      const CtkTextIter *_end)
{
	CtkSourceRegionPrivate *priv;
	GList *start_node;
	GList *end_node;
	GList *node;
	CtkTextIter sr_start_iter;
	CtkTextIter sr_end_iter;
	gboolean done;
	gboolean start_is_outside;
	gboolean end_is_outside;
	Subregion *sr;
	CtkTextIter start;
	CtkTextIter end;

	g_return_if_fail (CTK_SOURCE_IS_REGION (region));
	g_return_if_fail (_start != NULL);
	g_return_if_fail (_end != NULL);

	priv = ctk_source_region_get_instance_private (region);

	if (priv->buffer == NULL)
	{
		return;
	}

	start = *_start;
	end = *_end;

	DEBUG (g_print ("---\n"));
	DEBUG (print_region (region));
	DEBUG (g_message ("region_substract (%d, %d)",
			  ctk_text_iter_get_offset (&start),
			  ctk_text_iter_get_offset (&end)));

	ctk_text_iter_order (&start, &end);

	/* Find bounding subregions. */
	start_node = find_nearest_subregion (region, &start, NULL, FALSE, FALSE);
	end_node = find_nearest_subregion (region, &end, start_node, TRUE, FALSE);

	/* Easy case first. */
	if (start_node == NULL || end_node == NULL || end_node == start_node->prev)
	{
		return;
	}

	/* Deal with the start point. */
	start_is_outside = end_is_outside = FALSE;

	sr = start_node->data;
	ctk_text_buffer_get_iter_at_mark (priv->buffer, &sr_start_iter, sr->start);
	ctk_text_buffer_get_iter_at_mark (priv->buffer, &sr_end_iter, sr->end);

	if (ctk_text_iter_in_range (&start, &sr_start_iter, &sr_end_iter) &&
	    !ctk_text_iter_equal (&start, &sr_start_iter))
	{
		/* The starting point is inside the first subregion. */
		if (ctk_text_iter_in_range (&end, &sr_start_iter, &sr_end_iter) &&
		    !ctk_text_iter_equal (&end, &sr_end_iter))
		{
			/* The ending point is also inside the first
			 * subregion: we need to split.
			 */
			Subregion *new_sr = g_slice_new0 (Subregion);
			new_sr->end = sr->end;
			new_sr->start = ctk_text_buffer_create_mark (priv->buffer,
								     NULL,
								     &end,
								     TRUE);

			start_node = g_list_insert_before (start_node, start_node->next, new_sr);

			sr->end = ctk_text_buffer_create_mark (priv->buffer,
							       NULL,
							       &start,
							       FALSE);

			/* No further processing needed. */
			DEBUG (g_message ("subregion splitted"));

			return;
		}
		else
		{
			/* The ending point is outside, so just move
			 * the end of the subregion to the starting point.
			 */
			ctk_text_buffer_move_mark (priv->buffer, sr->end, &start);
		}
	}
	else
	{
		/* The starting point is outside (and so to the left)
		 * of the first subregion.
		 */
		DEBUG (g_message ("start is outside"));

		start_is_outside = TRUE;
	}

	/* Deal with the end point. */
	if (start_node != end_node)
	{
		sr = end_node->data;
		ctk_text_buffer_get_iter_at_mark (priv->buffer, &sr_start_iter, sr->start);
		ctk_text_buffer_get_iter_at_mark (priv->buffer, &sr_end_iter, sr->end);
	}

	if (ctk_text_iter_in_range (&end, &sr_start_iter, &sr_end_iter) &&
	    !ctk_text_iter_equal (&end, &sr_end_iter))
	{
		/* Ending point is inside, move the start mark. */
		ctk_text_buffer_move_mark (priv->buffer, sr->start, &end);
	}
	else
	{
		end_is_outside = TRUE;
		DEBUG (g_message ("end is outside"));
	}

	/* Finally remove any intermediate subregions. */
	done = FALSE;
	node = start_node;

	while (!done)
	{
		if (node == end_node)
		{
			/* We are done, exit in the next iteration. */
			done = TRUE;
		}

		if ((node == start_node && !start_is_outside) ||
		    (node == end_node && !end_is_outside))
		{
			/* Skip starting or ending node. */
			node = node->next;
		}
		else
		{
			GList *l = node->next;
			sr = node->data;
			ctk_text_buffer_delete_mark (priv->buffer, sr->start);
			ctk_text_buffer_delete_mark (priv->buffer, sr->end);
			g_slice_free (Subregion, sr);
			priv->subregions = g_list_delete_link (priv->subregions, node);
			node = l;
		}
	}

	priv->timestamp++;

	DEBUG (print_region (region));

	/* Now get rid of empty subregions. */
	ctk_source_region_clear_zero_length_subregions (region);

	DEBUG (print_region (region));
}

/**
 * ctk_source_region_subtract_region:
 * @region: a #CtkSourceRegion.
 * @region_to_subtract: (nullable): the #CtkSourceRegion to subtract from
 *   @region, or %NULL.
 *
 * Subtracts @region_to_subtract from @region. @region_to_subtract is not
 * modified.
 *
 * Since: 3.22
 */
void
ctk_source_region_subtract_region (CtkSourceRegion *region,
				   CtkSourceRegion *region_to_subtract)
{
	CtkTextBuffer *region_buffer;
	CtkTextBuffer *region_to_subtract_buffer;
	CtkSourceRegionIter iter;

	g_return_if_fail (CTK_SOURCE_IS_REGION (region));
	g_return_if_fail (region_to_subtract == NULL || CTK_SOURCE_IS_REGION (region_to_subtract));

	region_buffer = ctk_source_region_get_buffer (region);
	region_to_subtract_buffer = ctk_source_region_get_buffer (region_to_subtract);
	g_return_if_fail (region_buffer == region_to_subtract_buffer);

	if (region_buffer == NULL)
	{
		return;
	}

	ctk_source_region_get_start_region_iter (region_to_subtract, &iter);

	while (!ctk_source_region_iter_is_end (&iter))
	{
		CtkTextIter subregion_start;
		CtkTextIter subregion_end;

		if (!ctk_source_region_iter_get_subregion (&iter,
							   &subregion_start,
							   &subregion_end))
		{
			break;
		}

		ctk_source_region_subtract_subregion (region,
						      &subregion_start,
						      &subregion_end);

		ctk_source_region_iter_next (&iter);
	}
}

/**
 * ctk_source_region_is_empty:
 * @region: (nullable): a #CtkSourceRegion, or %NULL.
 *
 * Returns whether the @region is empty. A %NULL @region is considered empty.
 *
 * Returns: whether the @region is empty.
 * Since: 3.22
 */
gboolean
ctk_source_region_is_empty (CtkSourceRegion *region)
{
	CtkSourceRegionIter region_iter;

	if (region == NULL)
	{
		return TRUE;
	}

	/* A #CtkSourceRegion can contain empty subregions. So checking the
	 * number of subregions is not sufficient.
	 * When calling ctk_source_region_add_subregion() with equal iters, the
	 * subregion is not added. But when a subregion becomes empty, due to
	 * text deletion, the subregion is not removed from the
	 * #CtkSourceRegion.
	 */

	ctk_source_region_get_start_region_iter (region, &region_iter);

	while (!ctk_source_region_iter_is_end (&region_iter))
	{
		CtkTextIter subregion_start;
		CtkTextIter subregion_end;

		if (!ctk_source_region_iter_get_subregion (&region_iter,
							   &subregion_start,
							   &subregion_end))
		{
			return TRUE;
		}

		if (!ctk_text_iter_equal (&subregion_start, &subregion_end))
		{
			return FALSE;
		}

		ctk_source_region_iter_next (&region_iter);
	}

	return TRUE;
}

/**
 * ctk_source_region_get_bounds:
 * @region: a #CtkSourceRegion.
 * @start: (out) (optional): iterator to initialize with the start of @region,
 *   or %NULL.
 * @end: (out) (optional): iterator to initialize with the end of @region,
 *   or %NULL.
 *
 * Gets the @start and @end bounds of the @region.
 *
 * Returns: %TRUE if @start and @end have been set successfully (if non-%NULL),
 *   or %FALSE if the @region is empty.
 * Since: 3.22
 */
gboolean
ctk_source_region_get_bounds (CtkSourceRegion *region,
			      CtkTextIter     *start,
			      CtkTextIter     *end)
{
	CtkSourceRegionPrivate *priv;

	g_return_val_if_fail (CTK_SOURCE_IS_REGION (region), FALSE);

	priv = ctk_source_region_get_instance_private (region);

	if (priv->buffer == NULL ||
	    ctk_source_region_is_empty (region))
	{
		return FALSE;
	}

	g_assert (priv->subregions != NULL);

	if (start != NULL)
	{
		Subregion *first_subregion = priv->subregions->data;
		ctk_text_buffer_get_iter_at_mark (priv->buffer, start, first_subregion->start);
	}

	if (end != NULL)
	{
		Subregion *last_subregion = g_list_last (priv->subregions)->data;
		ctk_text_buffer_get_iter_at_mark (priv->buffer, end, last_subregion->end);
	}

	return TRUE;
}

/**
 * ctk_source_region_intersect_subregion:
 * @region: a #CtkSourceRegion.
 * @_start: the start of the subregion.
 * @_end: the end of the subregion.
 *
 * Returns the intersection between @region and the subregion delimited by
 * @_start and @_end. @region is not modified.
 *
 * Returns: (transfer full) (nullable): the intersection as a new
 *   #CtkSourceRegion.
 * Since: 3.22
 */
CtkSourceRegion *
ctk_source_region_intersect_subregion (CtkSourceRegion   *region,
				       const CtkTextIter *_start,
				       const CtkTextIter *_end)
{
	CtkSourceRegionPrivate *priv;
	CtkSourceRegion *new_region;
	CtkSourceRegionPrivate *new_priv;
	GList *start_node;
	GList *end_node;
	GList *node;
	CtkTextIter sr_start_iter;
	CtkTextIter sr_end_iter;
	Subregion *sr;
	Subregion *new_sr;
	gboolean done;
	CtkTextIter start;
	CtkTextIter end;

	g_return_val_if_fail (CTK_SOURCE_IS_REGION (region), NULL);
	g_return_val_if_fail (_start != NULL, NULL);
	g_return_val_if_fail (_end != NULL, NULL);

	priv = ctk_source_region_get_instance_private (region);

	if (priv->buffer == NULL)
	{
		return NULL;
	}

	start = *_start;
	end = *_end;

	ctk_text_iter_order (&start, &end);

	/* Find bounding subregions. */
	start_node = find_nearest_subregion (region, &start, NULL, FALSE, FALSE);
	end_node = find_nearest_subregion (region, &end, start_node, TRUE, FALSE);

	/* Easy case first. */
	if (start_node == NULL || end_node == NULL || end_node == start_node->prev)
	{
		return NULL;
	}

	new_region = ctk_source_region_new (priv->buffer);
	new_priv = ctk_source_region_get_instance_private (new_region);
	done = FALSE;

	sr = start_node->data;
	ctk_text_buffer_get_iter_at_mark (priv->buffer, &sr_start_iter, sr->start);
	ctk_text_buffer_get_iter_at_mark (priv->buffer, &sr_end_iter, sr->end);

	/* Starting node. */
	if (ctk_text_iter_in_range (&start, &sr_start_iter, &sr_end_iter))
	{
		new_sr = g_slice_new0 (Subregion);
		new_priv->subregions = g_list_prepend (new_priv->subregions, new_sr);

		new_sr->start = ctk_text_buffer_create_mark (new_priv->buffer,
							     NULL,
							     &start,
							     TRUE);

		if (start_node == end_node)
		{
			/* Things will finish shortly. */
			done = TRUE;
			if (ctk_text_iter_in_range (&end, &sr_start_iter, &sr_end_iter))
			{
				new_sr->end = ctk_text_buffer_create_mark (new_priv->buffer,
									   NULL,
									   &end,
									   FALSE);
			}
			else
			{
				new_sr->end = ctk_text_buffer_create_mark (new_priv->buffer,
									   NULL,
									   &sr_end_iter,
									   FALSE);
			}
		}
		else
		{
			new_sr->end = ctk_text_buffer_create_mark (new_priv->buffer,
								   NULL,
								   &sr_end_iter,
								   FALSE);
		}

		node = start_node->next;
	}
	else
	{
		/* start should be the same as the subregion, so copy it in the
		 * loop.
		 */
		node = start_node;
	}

	if (!done)
	{
		while (node != end_node)
		{
			/* Copy intermediate subregions verbatim. */
			sr = node->data;
			ctk_text_buffer_get_iter_at_mark (priv->buffer, &sr_start_iter, sr->start);
			ctk_text_buffer_get_iter_at_mark (priv->buffer, &sr_end_iter, sr->end);

			new_sr = g_slice_new0 (Subregion);
			new_priv->subregions = g_list_prepend (new_priv->subregions, new_sr);

			new_sr->start = ctk_text_buffer_create_mark (new_priv->buffer,
								     NULL,
								     &sr_start_iter,
								     TRUE);

			new_sr->end = ctk_text_buffer_create_mark (new_priv->buffer,
								   NULL,
								   &sr_end_iter,
								   FALSE);

			/* Next node. */
			node = node->next;
		}

		/* Ending node. */
		sr = node->data;
		ctk_text_buffer_get_iter_at_mark (priv->buffer, &sr_start_iter, sr->start);
		ctk_text_buffer_get_iter_at_mark (priv->buffer, &sr_end_iter, sr->end);

		new_sr = g_slice_new0 (Subregion);
		new_priv->subregions = g_list_prepend (new_priv->subregions, new_sr);

		new_sr->start = ctk_text_buffer_create_mark (new_priv->buffer,
							     NULL,
							     &sr_start_iter,
							     TRUE);

		if (ctk_text_iter_in_range (&end, &sr_start_iter, &sr_end_iter))
		{
			new_sr->end = ctk_text_buffer_create_mark (new_priv->buffer,
								   NULL,
								   &end,
								   FALSE);
		}
		else
		{
			new_sr->end = ctk_text_buffer_create_mark (new_priv->buffer,
								   NULL,
								   &sr_end_iter,
								   FALSE);
		}
	}

	new_priv->subregions = g_list_reverse (new_priv->subregions);
	return new_region;
}

/**
 * ctk_source_region_intersect_region:
 * @region1: (nullable): a #CtkSourceRegion, or %NULL.
 * @region2: (nullable): a #CtkSourceRegion, or %NULL.
 *
 * Returns the intersection between @region1 and @region2. @region1 and
 * @region2 are not modified.
 *
 * Returns: (transfer full) (nullable): the intersection as a #CtkSourceRegion
 *   object.
 * Since: 3.22
 */
CtkSourceRegion *
ctk_source_region_intersect_region (CtkSourceRegion *region1,
				    CtkSourceRegion *region2)
{
	CtkTextBuffer *region1_buffer;
	CtkTextBuffer *region2_buffer;
	CtkSourceRegion *full_intersect = NULL;
	CtkSourceRegionIter region2_iter;

	g_return_val_if_fail (region1 == NULL || CTK_SOURCE_IS_REGION (region1), NULL);
	g_return_val_if_fail (region2 == NULL || CTK_SOURCE_IS_REGION (region2), NULL);

	if (region1 == NULL && region2 == NULL)
	{
		return NULL;
	}
	if (region1 == NULL)
	{
		return g_object_ref (region2);
	}
	if (region2 == NULL)
	{
		return g_object_ref (region1);
	}

	region1_buffer = ctk_source_region_get_buffer (region1);
	region2_buffer = ctk_source_region_get_buffer (region2);
	g_return_val_if_fail (region1_buffer == region2_buffer, NULL);

	if (region1_buffer == NULL)
	{
		return NULL;
	}

	ctk_source_region_get_start_region_iter (region2, &region2_iter);

	while (!ctk_source_region_iter_is_end (&region2_iter))
	{
		CtkTextIter subregion2_start;
		CtkTextIter subregion2_end;
		CtkSourceRegion *sub_intersect;

		if (!ctk_source_region_iter_get_subregion (&region2_iter,
							   &subregion2_start,
							   &subregion2_end))
		{
			break;
		}

		sub_intersect = ctk_source_region_intersect_subregion (region1,
								       &subregion2_start,
								       &subregion2_end);

		if (full_intersect == NULL)
		{
			full_intersect = sub_intersect;
		}
		else
		{
			ctk_source_region_add_region (full_intersect, sub_intersect);
			g_clear_object (&sub_intersect);
		}

		ctk_source_region_iter_next (&region2_iter);
	}

	return full_intersect;
}

static gboolean
check_iterator (CtkSourceRegionIterReal *real)
{
	CtkSourceRegionPrivate *priv;

	if (real->region == NULL)
	{
		goto invalid;
	}

	priv = ctk_source_region_get_instance_private (real->region);

	if (real->region_timestamp == priv->timestamp)
	{
		return TRUE;
	}

invalid:
	g_warning ("Invalid CtkSourceRegionIter: either the iterator is "
		   "uninitialized, or the region has been modified since the "
		   "iterator was created.");

	return FALSE;
}

/**
 * ctk_source_region_get_start_region_iter:
 * @region: a #CtkSourceRegion.
 * @iter: (out): iterator to initialize to the first subregion.
 *
 * Initializes a #CtkSourceRegionIter to the first subregion of @region. If
 * @region is empty, @iter will be initialized to the end iterator.
 *
 * Since: 3.22
 */
void
ctk_source_region_get_start_region_iter (CtkSourceRegion     *region,
					 CtkSourceRegionIter *iter)
{
	CtkSourceRegionPrivate *priv;
	CtkSourceRegionIterReal *real;

	g_return_if_fail (CTK_SOURCE_IS_REGION (region));
	g_return_if_fail (iter != NULL);

	priv = ctk_source_region_get_instance_private (region);
	real = (CtkSourceRegionIterReal *)iter;

	/* priv->subregions may be NULL, -> end iter */

	real->region = region;
	real->subregions = priv->subregions;
	real->region_timestamp = priv->timestamp;
}

/**
 * ctk_source_region_iter_is_end:
 * @iter: a #CtkSourceRegionIter.
 *
 * Returns: whether @iter is the end iterator.
 * Since: 3.22
 */
gboolean
ctk_source_region_iter_is_end (CtkSourceRegionIter *iter)
{
	CtkSourceRegionIterReal *real;

	g_return_val_if_fail (iter != NULL, FALSE);

	real = (CtkSourceRegionIterReal *)iter;
	g_return_val_if_fail (check_iterator (real), FALSE);

	return real->subregions == NULL;
}

/**
 * ctk_source_region_iter_next:
 * @iter: a #CtkSourceRegionIter.
 *
 * Moves @iter to the next subregion.
 *
 * Returns: %TRUE if @iter moved and is dereferenceable, or %FALSE if @iter has
 *   been set to the end iterator.
 * Since: 3.22
 */
gboolean
ctk_source_region_iter_next (CtkSourceRegionIter *iter)
{
	CtkSourceRegionIterReal *real;

	g_return_val_if_fail (iter != NULL, FALSE);

	real = (CtkSourceRegionIterReal *)iter;
	g_return_val_if_fail (check_iterator (real), FALSE);

	if (real->subregions != NULL)
	{
		real->subregions = real->subregions->next;
		return TRUE;
	}

	return FALSE;
}

/**
 * ctk_source_region_iter_get_subregion:
 * @iter: a #CtkSourceRegionIter.
 * @start: (out) (optional): iterator to initialize with the subregion start, or %NULL.
 * @end: (out) (optional): iterator to initialize with the subregion end, or %NULL.
 *
 * Gets the subregion at this iterator.
 *
 * Returns: %TRUE if @start and @end have been set successfully (if non-%NULL),
 *   or %FALSE if @iter is the end iterator or if the region is empty.
 * Since: 3.22
 */
gboolean
ctk_source_region_iter_get_subregion (CtkSourceRegionIter *iter,
				      CtkTextIter         *start,
				      CtkTextIter         *end)
{
	CtkSourceRegionIterReal *real;
	CtkSourceRegionPrivate *priv;
	Subregion *sr;

	g_return_val_if_fail (iter != NULL, FALSE);

	real = (CtkSourceRegionIterReal *)iter;
	g_return_val_if_fail (check_iterator (real), FALSE);

	if (real->subregions == NULL)
	{
		return FALSE;
	}

	priv = ctk_source_region_get_instance_private (real->region);

	if (priv->buffer == NULL)
	{
		return FALSE;
	}

	sr = real->subregions->data;
	g_return_val_if_fail (sr != NULL, FALSE);

	if (start != NULL)
	{
		ctk_text_buffer_get_iter_at_mark (priv->buffer, start, sr->start);
	}

	if (end != NULL)
	{
		ctk_text_buffer_get_iter_at_mark (priv->buffer, end, sr->end);
	}

	return TRUE;
}

/**
 * ctk_source_region_to_string:
 * @region: a #CtkSourceRegion.
 *
 * Gets a string represention of @region, for debugging purposes.
 *
 * The returned string contains the character offsets of the subregions. It
 * doesn't include a newline character at the end of the string.
 *
 * Returns: (transfer full) (nullable): a string represention of @region. Free
 *   with g_free() when no longer needed.
 * Since: 3.22
 */
gchar *
ctk_source_region_to_string (CtkSourceRegion *region)
{
	CtkSourceRegionPrivate *priv;
	GString *string;
	GList *l;

	g_return_val_if_fail (CTK_SOURCE_IS_REGION (region), NULL);

	priv = ctk_source_region_get_instance_private (region);

	if (priv->buffer == NULL)
	{
		return NULL;
	}

	string = g_string_new ("Subregions:");

	for (l = priv->subregions; l != NULL; l = l->next)
	{
		Subregion *sr = l->data;
		CtkTextIter start;
		CtkTextIter end;

		ctk_text_buffer_get_iter_at_mark (priv->buffer, &start, sr->start);
		ctk_text_buffer_get_iter_at_mark (priv->buffer, &end, sr->end);

		g_string_append_printf (string,
					" %d-%d",
					ctk_text_iter_get_offset (&start),
					ctk_text_iter_get_offset (&end));
	}

	return g_string_free (string, FALSE);
}
