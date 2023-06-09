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

#include <ctk/ctk.h>
#include <ctksourceview/ctksource.h>
#include "ctksourceview/ctksourcecompletionmodel.h"

/* Basic provider.
 * The populate function is not implemented. Proposals are created
 * independendly (it is more convenient).
 */

typedef struct _TestProvider      TestProvider;
typedef struct _TestProviderClass TestProviderClass;

struct _TestProvider
{
	GObject parent_instance;
	gint priority;
};

struct _TestProviderClass
{
	GObjectClass parent_class;
};

GType test_provider_get_type (void);

static void test_provider_iface_init (CtkSourceCompletionProviderIface *iface);

G_DEFINE_TYPE_WITH_CODE (TestProvider,
			 test_provider,
			 G_TYPE_OBJECT,
			 G_IMPLEMENT_INTERFACE (CTK_SOURCE_TYPE_COMPLETION_PROVIDER,
						test_provider_iface_init));

static gchar *
test_provider_get_name (CtkSourceCompletionProvider *provider)
{
	return g_strdup ("Hobbits");
}

static gint
test_provider_get_priority (CtkSourceCompletionProvider *provider)
{
	return ((TestProvider *)provider)->priority;
}

static void
test_provider_iface_init (CtkSourceCompletionProviderIface *iface)
{
	iface->get_name = test_provider_get_name;
	iface->get_priority = test_provider_get_priority;
}

static void
test_provider_class_init (TestProviderClass *klass)
{
}

static void
test_provider_init (TestProvider *self)
{
	self->priority = 0;
}


static TestProvider *
test_provider_new (void)
{
	return g_object_new (test_provider_get_type (), NULL);
}

/* Utility functions */

static GList *
create_proposals (void)
{
	CtkSourceCompletionItem *item;
	GList *list = NULL;

	item = ctk_source_completion_item_new ();
	ctk_source_completion_item_set_label (item, "Bilbo");
	ctk_source_completion_item_set_text (item, "Bilbo");
	list = g_list_prepend (list, item);

	item = ctk_source_completion_item_new ();
	ctk_source_completion_item_set_label (item, "Frodo");
	ctk_source_completion_item_set_text (item, "Frodo");
	list = g_list_prepend (list, item);

	return list;
}

/* Each returned provider is associated with a list of proposals.
 * The providers are sorted in decreasing order of priority, i.e. the same order
 * as in the CompletionModel.
 */
static void
create_providers (GList **all_providers,
		  GList **all_list_proposals)
{
	TestProvider *provider;

	*all_providers = NULL;
	*all_list_proposals = NULL;

	provider = test_provider_new ();
	provider->priority = 5;
	*all_providers = g_list_append (*all_providers, provider);
	*all_list_proposals = g_list_append (*all_list_proposals, create_proposals ());

	provider = test_provider_new ();
	provider->priority = 3;
	*all_providers = g_list_append (*all_providers, provider);
	*all_list_proposals = g_list_append (*all_list_proposals, create_proposals ());
}

static void
populate_model (CtkSourceCompletionModel *model,
		GList                    *all_providers,
		GList                    *all_list_proposals)
{
	GList *cur_provider;
	GList *cur_list_proposals;

	for (cur_provider = all_providers,
	     cur_list_proposals = all_list_proposals;
	     cur_provider != NULL;
	     cur_provider = g_list_next (cur_provider),
	     cur_list_proposals = g_list_next (cur_list_proposals))
	{
		ctk_source_completion_model_add_proposals (model,
							   CTK_SOURCE_COMPLETION_PROVIDER (cur_provider->data),
							   cur_list_proposals->data);
	}
}

static void
free_providers (GList *all_providers,
		GList *all_list_proposals)
{
	GList *cur_list_proposals = NULL;

	g_list_free_full (all_providers, g_object_unref);

	for (cur_list_proposals = all_list_proposals;
	     cur_list_proposals != NULL;
	     cur_list_proposals = g_list_next (cur_list_proposals))
	{
		g_list_free_full (cur_list_proposals->data, g_object_unref);
	}

	g_list_free (all_list_proposals);
}

/* Check whether the provider is correctly present in the CompletionModel, at
 * the position specified by @iter.
 */
static void
check_provider (CtkSourceCompletionModel    *model,
		CtkSourceCompletionProvider *provider,
		GList                       *list_proposals,
		gboolean                     is_header_visible,
		CtkTreeIter                 *iter)

{
	CtkSourceCompletionProposal *proposal_get = NULL;
	CtkSourceCompletionProvider *provider_get = NULL;
	GList *cur_proposal = NULL;

	/* Check the header */

	if (is_header_visible)
	{
		g_assert_true (ctk_source_completion_model_iter_is_header (model, iter));

		ctk_tree_model_get (CTK_TREE_MODEL (model), iter,
				    CTK_SOURCE_COMPLETION_MODEL_COLUMN_PROPOSAL, &proposal_get,
				    CTK_SOURCE_COMPLETION_MODEL_COLUMN_PROVIDER, &provider_get,
				    -1);

		g_assert_null (proposal_get);
		g_assert_true (provider_get == provider);
		g_assert_true (ctk_tree_model_iter_next (CTK_TREE_MODEL (model), iter));
	}

	/* Check the proposals */

	cur_proposal = list_proposals;
	while (TRUE)
	{
		ctk_tree_model_get (CTK_TREE_MODEL (model), iter,
				    CTK_SOURCE_COMPLETION_MODEL_COLUMN_PROPOSAL, &proposal_get,
				    CTK_SOURCE_COMPLETION_MODEL_COLUMN_PROVIDER, &provider_get,
				    -1);

		g_assert_true (proposal_get == cur_proposal->data);
		g_assert_true (provider_get == provider);

		cur_proposal = g_list_next (cur_proposal);

		if (cur_proposal == NULL)
		{
			break;
		}

		g_assert_true (ctk_tree_model_iter_next (CTK_TREE_MODEL (model), iter));
	}
}

/* Check the full contents of a CompletionModel. */
static void
check_all_providers (CtkSourceCompletionModel *model,
		     GList                    *all_providers,
		     GList                    *all_list_proposals,
		     gboolean                  is_header_visible)
{
	CtkTreeIter iter;
	GList *cur_provider = NULL;
	GList *cur_list_proposals = NULL;

	g_assert_true (ctk_tree_model_get_iter_first (CTK_TREE_MODEL (model), &iter));

	cur_provider = all_providers;
	cur_list_proposals = all_list_proposals;
	while (TRUE)
	{
		check_provider (model,
				CTK_SOURCE_COMPLETION_PROVIDER (cur_provider->data),
				cur_list_proposals->data,
				is_header_visible,
				&iter);

		cur_provider = g_list_next (cur_provider);
		cur_list_proposals = g_list_next (cur_list_proposals);

		if (cur_provider == NULL)
		{
			g_assert_null (cur_list_proposals);
			break;
		}

		g_assert_true (ctk_tree_model_iter_next (CTK_TREE_MODEL (model), &iter));
	}

	g_assert_true (!ctk_tree_model_iter_next (CTK_TREE_MODEL (model), &iter));
}

static void
check_all_providers_with_and_without_headers (CtkSourceCompletionModel *model,
					      GList                    *all_providers,
					      GList                    *all_list_proposals)
{
	ctk_source_completion_model_set_show_headers (model, TRUE);
	check_all_providers (model, all_providers, all_list_proposals, TRUE);

	ctk_source_completion_model_set_show_headers (model, FALSE);
	check_all_providers (model, all_providers, all_list_proposals, FALSE);
}

static gboolean
same_list_contents (GList *list1, GList *list2)
{
	GList *cur_item1 = list1;
	GList *cur_item2 = list2;

	if (g_list_length (list1) != g_list_length (list2))
	{
		return FALSE;
	}

	while (cur_item1 != NULL)
	{
		if (cur_item1->data != cur_item2->data)
		{
			return FALSE;
		}

		cur_item1 = g_list_next (cur_item1);
		cur_item2 = g_list_next (cur_item2);
	}

	return TRUE;
}

/* Tests */

static void
test_is_empty (void)
{
	CtkSourceCompletionModel *model;
	TestProvider *provider;
	GList *list_providers = NULL;
	GList *list_proposals = NULL;
	GList *visible_providers = NULL;

	/* Completely empty */
	model = ctk_source_completion_model_new ();

	g_assert_true (ctk_source_completion_model_is_empty (model, FALSE));
	g_assert_true (ctk_source_completion_model_is_empty (model, TRUE));

	/* One visible provider */
	provider = test_provider_new ();
	list_providers = g_list_append (list_providers, provider);
	list_proposals = create_proposals ();

	ctk_source_completion_model_add_proposals (model,
						   CTK_SOURCE_COMPLETION_PROVIDER (provider),
						   list_proposals);

	g_assert_false (ctk_source_completion_model_is_empty (model, FALSE));
	g_assert_false (ctk_source_completion_model_is_empty (model, TRUE));

	/* One invisible provider */
	visible_providers = g_list_append (visible_providers, test_provider_new ());
	ctk_source_completion_model_set_visible_providers (model, visible_providers);

	g_assert_false (ctk_source_completion_model_is_empty (model, FALSE));
	g_assert_true (ctk_source_completion_model_is_empty (model, TRUE));

	g_object_unref (model);
	g_list_free_full (list_providers, g_object_unref);
	g_list_free_full (list_proposals, g_object_unref);
	g_list_free_full (visible_providers, g_object_unref);
}

static void
test_get_visible_providers (void)
{
	CtkSourceCompletionModel *model;
	TestProvider *provider;
	GList *list_providers = NULL;
	GList *visible_providers = NULL;

	model = ctk_source_completion_model_new ();
	g_assert_null (ctk_source_completion_model_get_visible_providers (model));

	provider = test_provider_new ();
	list_providers = g_list_append (list_providers, provider);

	ctk_source_completion_model_set_visible_providers (model, list_providers);
	visible_providers = ctk_source_completion_model_get_visible_providers (model);

	g_assert_true (visible_providers->data == provider);

	g_object_unref (model);
	g_list_free_full (list_providers, g_object_unref);
}

/* Create several providers with associated proposals, populate them in a
 * CompletionModel, and check whether the CompletionModel correctly contains the
 * providers.
 */
static void
test_simple_populate (void)
{
	CtkSourceCompletionModel *model;
	GList *all_providers = NULL;
	GList *all_list_proposals = NULL;

	model = ctk_source_completion_model_new ();
	create_providers (&all_providers, &all_list_proposals);
	populate_model (model, all_providers, all_list_proposals);

	check_all_providers_with_and_without_headers (model, all_providers, all_list_proposals);

	g_object_unref (model);
	free_providers (all_providers, all_list_proposals);
}

static void
test_set_visible_providers (void)
{
	CtkSourceCompletionModel *model;
	GList *all_providers = NULL;
	GList *all_list_proposals = NULL;
	GList *subset_providers = NULL;
	GList *subset_list_proposals = NULL;
	TestProvider *other_provider;

	/* Populate the model with two providers */
	model = ctk_source_completion_model_new ();
	create_providers (&all_providers, &all_list_proposals);
	populate_model (model, all_providers, all_list_proposals);

	/* The two providers are initially visible */
	check_all_providers_with_and_without_headers (model, all_providers, all_list_proposals);

	ctk_source_completion_model_set_visible_providers (model, NULL);
	check_all_providers_with_and_without_headers (model, all_providers, all_list_proposals);

	ctk_source_completion_model_set_visible_providers (model, all_providers);
	check_all_providers_with_and_without_headers (model, all_providers, all_list_proposals);

	/* Only first provider visible */
	subset_providers = g_list_append (subset_providers, all_providers->data);
	subset_list_proposals = g_list_append (subset_list_proposals, all_list_proposals->data);

	ctk_source_completion_model_set_visible_providers (model, subset_providers);
	check_all_providers_with_and_without_headers (model, subset_providers, subset_list_proposals);

	/* Only second provider visible */
	subset_providers->data = all_providers->next->data;
	subset_list_proposals->data = all_list_proposals->next->data;

	ctk_source_completion_model_set_visible_providers (model, subset_providers);
	check_all_providers_with_and_without_headers (model, subset_providers, subset_list_proposals);

	/* No visible providers */
	other_provider = test_provider_new ();
	subset_providers->data = other_provider;
	ctk_source_completion_model_set_visible_providers (model, subset_providers);
	g_assert_true (ctk_source_completion_model_is_empty (model, TRUE));

	/* The two providers are visible again */
	ctk_source_completion_model_set_visible_providers (model, NULL);
	check_all_providers_with_and_without_headers (model, all_providers, all_list_proposals);

	g_object_unref (model);
	g_object_unref (other_provider);
	free_providers (all_providers, all_list_proposals);
	g_list_free (subset_providers);
	g_list_free (subset_list_proposals);
}

static void
test_populate_several_batches (void)
{
	CtkSourceCompletionModel *model = ctk_source_completion_model_new ();
	CtkSourceCompletionProvider *provider = CTK_SOURCE_COMPLETION_PROVIDER (test_provider_new ());
	GList *list_providers = g_list_append (NULL, provider);
	GList *first_proposals = create_proposals ();
	GList *second_proposals = create_proposals ();
	GList *all_proposals;
	CtkTreeIter iter;

	ctk_source_completion_model_set_show_headers (model, TRUE);

	/* First batch */
	ctk_source_completion_model_add_proposals (model, provider, first_proposals);

	g_assert_true (ctk_tree_model_get_iter_first (CTK_TREE_MODEL (model), &iter));
	check_provider (model, provider, first_proposals, TRUE, &iter);
	g_assert_false (ctk_tree_model_iter_next (CTK_TREE_MODEL (model), &iter));

	/* Second batch */
	ctk_source_completion_model_add_proposals (model, provider, second_proposals);

	all_proposals = g_list_copy (first_proposals);
	all_proposals = g_list_concat (all_proposals, g_list_copy (second_proposals));

	g_assert_true (ctk_tree_model_get_iter_first (CTK_TREE_MODEL (model), &iter));
	check_provider (model, provider, all_proposals, TRUE, &iter);
	g_assert_false (ctk_tree_model_iter_next (CTK_TREE_MODEL (model), &iter));

	g_object_unref (model);
	g_object_unref (provider);
	g_list_free (list_providers);
	g_list_free (first_proposals);
	g_list_free (second_proposals);
	g_list_free_full (all_proposals, g_object_unref);
}

static void
test_get_providers (void)
{
	CtkSourceCompletionModel *model = ctk_source_completion_model_new ();
	GList *all_providers = NULL;
	GList *all_list_proposals = NULL;
	GList *providers_get = NULL;

	/* Empty */
	g_assert_null (ctk_source_completion_model_get_providers (model));

	/* Non-empty */
	create_providers (&all_providers, &all_list_proposals);
	populate_model (model, all_providers, all_list_proposals);

	providers_get = ctk_source_completion_model_get_providers (model);
	g_assert_true (same_list_contents (all_providers, providers_get));

	g_object_unref (model);
	free_providers (all_providers, all_list_proposals);
	g_list_free (providers_get);
}

static void
test_iters_impl (gboolean show_headers)
{
	CtkSourceCompletionModel *model = ctk_source_completion_model_new ();
	GList *all_providers = NULL;
	GList *all_list_proposals = NULL;
	CtkTreeIter first_iter;
	CtkTreeIter last_iter;
	CtkTreeIter other_iter;
	CtkTreePath *path = NULL;
	gint nb_items;
	gint *indices;

	/* Test last_proposal() */
	g_assert_false (ctk_source_completion_model_last_proposal (model, &last_iter));

	create_providers (&all_providers, &all_list_proposals);
	populate_model (model, all_providers, all_list_proposals);

	ctk_source_completion_model_set_show_headers (model, show_headers);

	g_assert_true (ctk_source_completion_model_last_proposal (model, &last_iter));

	/* Get the last proposal by another means, and compare it */
	nb_items = ctk_tree_model_iter_n_children (CTK_TREE_MODEL (model), NULL);

	g_assert_true (ctk_tree_model_iter_nth_child (CTK_TREE_MODEL (model),
	                                              &other_iter,
	                                              NULL,
	                                              nb_items - 1));

	g_assert_true (ctk_source_completion_model_iter_equal (model, &last_iter, &other_iter));

	/* Test get_path() */
	path = ctk_tree_model_get_path (CTK_TREE_MODEL (model), &last_iter);
	indices = ctk_tree_path_get_indices (path);
	g_assert_cmpint (indices[0], ==, nb_items - 1);

	/* Test iter_previous() */
	while (ctk_source_completion_model_iter_previous (model, &other_iter));

	g_assert_true (ctk_tree_model_get_iter_first (CTK_TREE_MODEL (model), &first_iter));
	g_assert_true (ctk_source_completion_model_iter_equal (model, &first_iter, &other_iter));

	/* Test iter_children() */
	g_assert_true (ctk_tree_model_iter_children (CTK_TREE_MODEL (model), &other_iter, NULL));
	g_assert_true (ctk_source_completion_model_iter_equal (model, &first_iter, &other_iter));
	g_assert_false (ctk_tree_model_iter_children (CTK_TREE_MODEL (model), &other_iter, &first_iter));

	/* Test iter_has_child() */
	g_assert_false (ctk_tree_model_iter_has_child (CTK_TREE_MODEL (model), &first_iter));

	/* Test iter_parent() */
	g_assert_false (ctk_tree_model_iter_parent (CTK_TREE_MODEL (model), &other_iter, &first_iter));

	g_object_unref (model);
	free_providers (all_providers, all_list_proposals);
	ctk_tree_path_free (path);
}

static void
test_iters (void)
{
	test_iters_impl (FALSE);
	test_iters_impl (TRUE);
}

static void
on_row_changed (CtkTreeModel                *model,
		CtkTreePath                 *path,
		CtkTreeIter                 *iter,
		CtkSourceCompletionProposal *proposal)
{
	CtkSourceCompletionProposal *row_proposal = NULL;

	ctk_tree_model_get (model, iter,
			    CTK_SOURCE_COMPLETION_MODEL_COLUMN_PROPOSAL, &row_proposal,
			    -1);

	/* Make sure that the signal was emitted for the good row. */
	g_assert_true (proposal == row_proposal);
}

static void
test_row_changed (void)
{
	CtkSourceCompletionModel *model = ctk_source_completion_model_new ();
	TestProvider *provider = test_provider_new ();
	GList *proposals = create_proposals ();
	CtkSourceCompletionProposal *proposal = proposals->data;

	ctk_source_completion_model_add_proposals (model,
						   CTK_SOURCE_COMPLETION_PROVIDER (provider),
						   proposals);

	g_signal_connect (model,
			  "row-changed",
			  G_CALLBACK (on_row_changed),
			  proposal);

	ctk_source_completion_proposal_changed (proposal);

	g_object_unref (model);
	g_object_unref (provider);
	g_list_free_full (proposals, g_object_unref);
}

int
main (int argc, char **argv)
{
	ctk_test_init (&argc, &argv);

	g_test_add_func ("/CompletionModel/is-empty",
			 test_is_empty);

	g_test_add_func ("/CompletionModel/get-visible-providers",
			 test_get_visible_providers);

	g_test_add_func ("/CompletionModel/simple-populate",
			 test_simple_populate);

	g_test_add_func ("/CompletionModel/set-visible-providers",
			 test_set_visible_providers);

	g_test_add_func ("/CompletionModel/populate-several-batches",
			 test_populate_several_batches);

	g_test_add_func ("/CompletionModel/get-providers",
			 test_get_providers);

	g_test_add_func ("/CompletionModel/iters",
			 test_iters);

	g_test_add_func ("/CompletionModel/row-changed",
			 test_row_changed);

	return g_test_run ();
}
