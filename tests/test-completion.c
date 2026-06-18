/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8; coding: utf-8 -*-
 *
 * This file is part of CtkSourceView
 *
 * Copyright (C) 2007 - Jesús Barbero Rodríguez <chuchiperriman@gmail.com>
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

typedef struct _TestProvider TestProvider;
typedef struct _TestProviderClass TestProviderClass;

static CtkSourceCompletionWords *word_provider;
static TestProvider *fixed_provider;
static TestProvider *random_provider;

struct _TestProvider
{
	GObject parent;

	GList *proposals;
	gint priority;
	gchar *name;

	CdkPixbuf *provider_icon;

	CdkPixbuf *item_icon;
	GIcon *item_gicon;

	/* If it's a random provider, a subset of 'proposals' are choosen on
	 * each populate. Otherwise, all the proposals are shown. */
	guint is_random : 1;
};

struct _TestProviderClass
{
	GObjectClass parent_class;
};

static void test_provider_iface_init (CtkSourceCompletionProviderIface *iface);
GType test_provider_get_type (void);

G_DEFINE_TYPE_WITH_CODE (TestProvider,
			 test_provider,
			 G_TYPE_OBJECT,
			 G_IMPLEMENT_INTERFACE (CTK_SOURCE_TYPE_COMPLETION_PROVIDER,
				 		test_provider_iface_init))

static gchar *
test_provider_get_name (CtkSourceCompletionProvider *provider)
{
	return g_strdup (((TestProvider *)provider)->name);
}

static gint
test_provider_get_priority (CtkSourceCompletionProvider *provider)
{
	return ((TestProvider *)provider)->priority;
}

static GList *
select_random_proposals (GList *all_proposals)
{
	GList *selection = NULL;
	GList *prop;

	for (prop = all_proposals; prop != NULL; prop = g_list_next (prop))
	{
		if (g_random_boolean ())
		{
			selection = g_list_prepend (selection, prop->data);
		}
	}

	return selection;
}

static void
test_provider_populate (CtkSourceCompletionProvider *completion_provider,
                        CtkSourceCompletionContext  *context)
{
	TestProvider *provider = (TestProvider *)completion_provider;
	GList *proposals;

	if (provider->is_random)
	{
		proposals = select_random_proposals (provider->proposals);
	}
	else
	{
		proposals = provider->proposals;
	}

	ctk_source_completion_context_add_proposals (context,
						     completion_provider,
						     proposals,
						     TRUE);
}

static CdkPixbuf *
test_provider_get_icon (CtkSourceCompletionProvider *provider)
{
	TestProvider *tp = (TestProvider *)provider;

	return tp->is_random ? NULL : tp->provider_icon;
}

static void
test_provider_iface_init (CtkSourceCompletionProviderIface *iface)
{
	iface->get_name = test_provider_get_name;
	iface->populate = test_provider_populate;
	iface->get_priority = test_provider_get_priority;
	iface->get_icon = test_provider_get_icon;
}

static void
test_provider_dispose (GObject *gobject)
{
	TestProvider *self = (TestProvider *)gobject;

	g_list_free_full (self->proposals, g_object_unref);
	self->proposals = NULL;

	g_clear_object (&self->provider_icon);
	g_clear_object (&self->item_icon);
	g_clear_object (&self->item_gicon);

	G_OBJECT_CLASS (test_provider_parent_class)->dispose (gobject);
}

static void
test_provider_finalize (GObject *gobject)
{
	TestProvider *self = (TestProvider *)gobject;

	g_free (self->name);
	self->name = NULL;

	G_OBJECT_CLASS (test_provider_parent_class)->finalize (gobject);
}

static void
test_provider_class_init (TestProviderClass *klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

	gobject_class->dispose = test_provider_dispose;
	gobject_class->finalize = test_provider_finalize;
}

static void
test_provider_init (TestProvider *self)
{
	CtkIconTheme *theme;
	GIcon *icon;
	GIcon *emblem_icon;
	GEmblem *emblem;

	theme = ctk_icon_theme_get_default ();
	self->provider_icon = ctk_icon_theme_load_icon (theme, "dialog-information", 16, 0, NULL);

	self->item_icon = ctk_icon_theme_load_icon (theme, "trophy-gold", 16, 0, NULL);

	icon = g_themed_icon_new ("trophy-silver");
	emblem_icon = g_themed_icon_new ("emblem-urgent");
	emblem = g_emblem_new (emblem_icon);
	self->item_gicon = g_emblemed_icon_new (icon, emblem);
	g_object_unref (icon);
	g_object_unref (emblem_icon);
	g_object_unref (emblem);
}

static void
test_provider_set_fixed (TestProvider *provider,
			 gint          nb_proposals)
{
	CtkSourceCompletionItem *item;
	GList *proposals = NULL;
	gint i;

	g_list_free_full (provider->proposals, g_object_unref);

	item = ctk_source_completion_item_new ();
	ctk_source_completion_item_set_markup (item, "A very <b>long</b> proposal. I <i>repeat</i>, a very long proposal!");
	ctk_source_completion_item_set_text (item, "A very long proposal. I repeat, a very long proposal!");
	ctk_source_completion_item_set_icon (item, provider->item_icon);
	ctk_source_completion_item_set_info (item, "To test the horizontal scrollbar and the markup.");
	proposals = g_list_prepend (proposals, item);

	item = ctk_source_completion_item_new ();
	ctk_source_completion_item_set_markup (item, "A proposal with a <b>symbolic</b> icon");
	ctk_source_completion_item_set_text (item, "Test setting the icon-name property");
	ctk_source_completion_item_set_icon_name (item, "face-cool-symbolic");
	proposals = g_list_prepend (proposals, item);

	item = ctk_source_completion_item_new ();
	ctk_source_completion_item_set_markup (item, "A proposal with an emblem <b>GIcon</b>");
	ctk_source_completion_item_set_text (item, "Test setting the GIcon property");
	ctk_source_completion_item_set_gicon (item, provider->item_gicon);
	proposals = g_list_prepend (proposals, item);

	for (i = nb_proposals - 1; i > 0; i--)
	{
		gchar *name = g_strdup_printf ("Proposal %d", i);

		item = ctk_source_completion_item_new ();
		ctk_source_completion_item_set_label (item, name);
		ctk_source_completion_item_set_text (item, name);
		ctk_source_completion_item_set_icon (item, provider->item_icon);
		ctk_source_completion_item_set_info (item, "The extra info of the proposal.\nA second line.");
		proposals = g_list_prepend (proposals, item);

		g_free (name);
	}

	provider->proposals = proposals;
	provider->is_random = 0;
}

static void
test_provider_set_random (TestProvider *provider,
			  gint          nb_proposals)
{
	GList *proposals = NULL;
	gint i;

	g_list_free_full (provider->proposals, g_object_unref);

	for (i = 0; i < nb_proposals; i++)
	{
		CtkSourceCompletionItem *item;
		gchar *padding = g_strnfill ((i * 3) % 10, 'o');
		gchar *name = g_strdup_printf ("Propo%ssal %d", padding, i);

		item = ctk_source_completion_item_new ();
		ctk_source_completion_item_set_label (item, name);
		ctk_source_completion_item_set_text (item, name);
		ctk_source_completion_item_set_icon (item, provider->item_icon);
		proposals = g_list_prepend (proposals, item);

		g_free (padding);
		g_free (name);
	}

	provider->proposals = proposals;
	provider->is_random = 1;
}

static void
add_remove_provider (CtkToggleButton             *button,
		     CtkSourceCompletion         *completion,
		     CtkSourceCompletionProvider *provider)
{
	g_return_if_fail (provider != NULL);

	if (ctk_toggle_button_get_active (button))
	{
		ctk_source_completion_add_provider (completion, provider, NULL);
	}
	else
	{
		ctk_source_completion_remove_provider (completion, provider, NULL);
	}
}

static void
enable_word_provider_toggled_cb (CtkToggleButton     *button,
				 CtkSourceCompletion *completion)
{
	add_remove_provider (button,
			     completion,
			     CTK_SOURCE_COMPLETION_PROVIDER (word_provider));
}

static void
enable_fixed_provider_toggled_cb (CtkToggleButton     *button,
				  CtkSourceCompletion *completion)
{
	add_remove_provider (button,
			     completion,
			     CTK_SOURCE_COMPLETION_PROVIDER (fixed_provider));
}

static void
enable_random_provider_toggled_cb (CtkToggleButton     *button,
				   CtkSourceCompletion *completion)
{
	add_remove_provider (button,
			     completion,
			     CTK_SOURCE_COMPLETION_PROVIDER (random_provider));
}

static void
nb_proposals_changed_cb (CtkSpinButton *spin_button,
			 TestProvider  *provider)
{
	gint nb_proposals = ctk_spin_button_get_value_as_int (spin_button);

	if (provider->is_random)
	{
		test_provider_set_random (provider, nb_proposals);
	}
	else
	{
		test_provider_set_fixed (provider, nb_proposals);
	}
}

static void
create_completion (CtkSourceView       *source_view,
		   CtkSourceCompletion *completion)
{
	/* Words completion provider */
	word_provider = ctk_source_completion_words_new (NULL, NULL);

	ctk_source_completion_words_register (word_provider,
	                                      ctk_text_view_get_buffer (CTK_TEXT_VIEW (source_view)));

	ctk_source_completion_add_provider (completion,
	                                    CTK_SOURCE_COMPLETION_PROVIDER (word_provider),
	                                    NULL);

	g_object_set (word_provider, "priority", 10, NULL);

	/* Fixed provider: the proposals don't change */
	fixed_provider = g_object_new (test_provider_get_type (), NULL);
	test_provider_set_fixed (fixed_provider, 3);
	fixed_provider->priority = 5;
	fixed_provider->name = g_strdup ("Fixed Provider");

	ctk_source_completion_add_provider (completion,
	                                    CTK_SOURCE_COMPLETION_PROVIDER (fixed_provider),
	                                    NULL);

	/* Random provider: the proposals vary on each populate */
	random_provider = g_object_new (test_provider_get_type (), NULL);
	test_provider_set_random (random_provider, 10);
	random_provider->priority = 1;
	random_provider->name = g_strdup ("Random Provider");

	ctk_source_completion_add_provider (completion,
	                                    CTK_SOURCE_COMPLETION_PROVIDER (random_provider),
	                                    NULL);
}

static void
create_window (void)
{
	CtkBuilder *builder;
	GError *error = NULL;
	CtkWindow *window;
	CtkSourceView *source_view;
	CtkSourceCompletion *completion;
	CtkCheckButton *remember_info_visibility;
	CtkCheckButton *select_on_show;
	CtkCheckButton *show_headers;
	CtkCheckButton *show_icons;
	CtkCheckButton *enable_word_provider;
	CtkCheckButton *enable_fixed_provider;
	CtkCheckButton *enable_random_provider;
	CtkSpinButton *nb_fixed_proposals;
	CtkSpinButton *nb_random_proposals;

	builder = ctk_builder_new ();

	ctk_builder_add_from_resource (builder,
				       "/org/gnome/ctksourceview/tests/ui/test-completion.ui",
				       &error);

	if (error != NULL)
	{
		g_error ("Impossible to load test-completion.ui: %s", error->message);
	}

	window = CTK_WINDOW (ctk_builder_get_object (builder, "window"));
	source_view = CTK_SOURCE_VIEW (ctk_builder_get_object (builder, "source_view"));
	remember_info_visibility = CTK_CHECK_BUTTON (ctk_builder_get_object (builder, "checkbutton_remember_info_visibility"));
	select_on_show = CTK_CHECK_BUTTON (ctk_builder_get_object (builder, "checkbutton_select_on_show"));
	show_headers = CTK_CHECK_BUTTON (ctk_builder_get_object (builder, "checkbutton_show_headers"));
	show_icons = CTK_CHECK_BUTTON (ctk_builder_get_object (builder, "checkbutton_show_icons"));
	enable_word_provider = CTK_CHECK_BUTTON (ctk_builder_get_object (builder, "checkbutton_word_provider"));
	enable_fixed_provider = CTK_CHECK_BUTTON (ctk_builder_get_object (builder, "checkbutton_fixed_provider"));
	enable_random_provider = CTK_CHECK_BUTTON (ctk_builder_get_object (builder, "checkbutton_random_provider"));
	nb_fixed_proposals = CTK_SPIN_BUTTON (ctk_builder_get_object (builder, "spinbutton_nb_fixed_proposals"));
	nb_random_proposals = CTK_SPIN_BUTTON (ctk_builder_get_object (builder, "spinbutton_nb_random_proposals"));

	completion = ctk_source_view_get_completion (source_view);

	g_signal_connect (window,
			  "destroy",
			  G_CALLBACK (ctk_main_quit),
			  NULL);

	g_object_bind_property (completion, "remember-info-visibility",
				remember_info_visibility, "active",
				G_BINDING_SYNC_CREATE | G_BINDING_BIDIRECTIONAL);

	g_object_bind_property (completion, "select-on-show",
				select_on_show, "active",
				G_BINDING_SYNC_CREATE | G_BINDING_BIDIRECTIONAL);

	g_object_bind_property (completion, "show-headers",
				show_headers, "active",
				G_BINDING_SYNC_CREATE | G_BINDING_BIDIRECTIONAL);

	g_object_bind_property (completion, "show-icons",
				show_icons, "active",
				G_BINDING_SYNC_CREATE | G_BINDING_BIDIRECTIONAL);

	create_completion (source_view, completion);

	g_signal_connect (enable_word_provider,
			  "toggled",
			  G_CALLBACK (enable_word_provider_toggled_cb),
			  completion);

	g_signal_connect (enable_fixed_provider,
			  "toggled",
			  G_CALLBACK (enable_fixed_provider_toggled_cb),
			  completion);

	g_signal_connect (enable_random_provider,
			  "toggled",
			  G_CALLBACK (enable_random_provider_toggled_cb),
			  completion);

	g_signal_connect (nb_fixed_proposals,
			  "value-changed",
			  G_CALLBACK (nb_proposals_changed_cb),
			  fixed_provider);

	g_signal_connect (nb_random_proposals,
			  "value-changed",
			  G_CALLBACK (nb_proposals_changed_cb),
			  random_provider);

	g_object_unref (builder);
}

int
main (int argc, char *argv[])
{
	ctk_init (&argc, &argv);

	create_window ();

	ctk_main ();

	/* Not really useful, except for debugging memory leaks. */
	g_object_unref (word_provider);
	g_object_unref (fixed_provider);
	g_object_unref (random_provider);

	return 0;
}
