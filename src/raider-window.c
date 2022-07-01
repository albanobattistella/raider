/* raider-window.c
 *
 * Copyright 2022 Alan Beveridge
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <glib/gi18n.h>
#include "raider-config.h"
#include "raider-window.h"
#include "raider-file-row.h"

struct _RaiderWindow {
	GtkApplicationWindow parent_instance;

    GtkBox* contents_box;

	AdwSplitButton* open_button_full;
	GtkStack* window_stack;

	AdwSplitButton* open_button;
	GtkRevealer* open_revealer;
	GtkButton* shred_button;
	GtkRevealer* shred_revealer;
	GtkButton* abort_button;
	GtkRevealer* abort_revealer;
	AdwToastOverlay *toast_overlay;
	GtkListBox* list_box;

	GtkDropTarget *target;

	GList* filenames;
	int file_count;
};

G_DEFINE_TYPE(RaiderWindow, raider_window, GTK_TYPE_APPLICATION_WINDOW)

/*
 * THE BIG FUNCTION
 *
 * This function does not use a queue because shred is not cpu intensive, just disk intensive, and
   it is up to the disk to queue io requests.
 */
void shred_file(GtkWidget *widget, gpointer data)
{
	/* Clear the subtitle. */
	RaiderWindow *window = RAIDER_WINDOW(data);

	/* Update the headerbar view. */
	gtk_revealer_set_reveal_child(window->shred_revealer, FALSE);   // Hide.
	gtk_revealer_set_reveal_child(window->abort_revealer, TRUE);    // Show.
	gtk_revealer_set_reveal_child(window->open_revealer, FALSE);    // Hide.

	/* Launch the shredding. */
	int row;

	for (row = 0; row < window->file_count; row++) {
		RaiderFileRow* file_row = RAIDER_FILE_ROW(gtk_list_box_get_row_at_index(window->list_box, row));
		launch_shredding((gpointer)file_row);
	}
}

static void
raider_window_class_init(RaiderWindowClass *klass)
{
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

	gtk_widget_class_set_template_from_resource(widget_class, "/com/github/ADBeveridge/Raider/raider-window.ui");
	gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(widget_class), RaiderWindow, open_button);
	gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(widget_class), RaiderWindow, open_revealer);
	gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(widget_class), RaiderWindow, shred_button);
	gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(widget_class), RaiderWindow, abort_button);
	gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(widget_class), RaiderWindow, list_box);
	gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(widget_class), RaiderWindow, window_stack);
	gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(widget_class), RaiderWindow, shred_revealer);
	gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(widget_class), RaiderWindow, abort_revealer);
	gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(widget_class), RaiderWindow, toast_overlay);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(widget_class), RaiderWindow, contents_box);
}

/* Make highlight. */
static GdkDragAction on_enter(GtkDropTarget *target, double x, double y, gpointer data)
{
	RaiderWindow* window = RAIDER_WINDOW(data);
    GtkStyleContext* context = gtk_widget_get_style_context(GTK_WIDGET(window->contents_box));

	gtk_style_context_add_class(context, "drop_hover");

	return GDK_ACTION_COPY;
}

/* Get rid of highlight. */
static void on_leave(GtkDropTarget *target, gpointer data)
{
	RaiderWindow* window = RAIDER_WINDOW(data);
    GtkStyleContext* context = gtk_widget_get_style_context(GTK_WIDGET(window->contents_box));

	gtk_style_context_remove_class(context, "drop_hover");
}

static gboolean on_drop(GtkDropTarget *target, const GValue  *value, double x, double y, gpointer data)
{
	/* GdkFileList is a boxed value so we use the boxed API. */
	GdkFileList* file_list = g_value_get_boxed(value);

	GSList* list = gdk_file_list_get_files(file_list);

	/* Loop through the files and print their names. */
	GSList *l;
	for (l = list; l != NULL; l = l->next) {
		GFile* file = g_file_dup(l->data);
		raider_window_open(file, data);
	}
	return TRUE;
}

static void
raider_window_init(RaiderWindow *self)
{
	gtk_widget_init_template(GTK_WIDGET(self));

	self->file_count = 0;
	self->filenames = NULL;

	g_signal_connect(self->shred_button, "clicked", G_CALLBACK(shred_file), self);

	self->target = gtk_drop_target_new(G_TYPE_INVALID, GDK_ACTION_COPY);

	GType drop_types[] = { GDK_TYPE_FILE_LIST };
	gtk_drop_target_set_gtypes(self->target, drop_types, 1);

	g_signal_connect(self->target, "drop", G_CALLBACK(on_drop), self);
	g_signal_connect(self->target, "enter", G_CALLBACK(on_enter), self);
	g_signal_connect(self->target, "leave", G_CALLBACK(on_leave), self);

	gtk_widget_add_controller(GTK_WIDGET(self->contents_box), GTK_EVENT_CONTROLLER(self->target));
}

/* This is called when the handler for the close button destroys the row. This handles the application state */
void raider_window_close(gpointer data, gpointer user_data)
{
	RaiderWindow *window = RAIDER_WINDOW(user_data);

	RaiderFileRow* row = RAIDER_FILE_ROW(data);
	gchar* filename = raider_file_row_get_filename(row);

	gboolean removed = FALSE;

	/* Search to delete the entry. */
	GList * item = window->filenames;
	while (item != NULL) {
		GList *next = item->next;

		/* Get the filename for this round. */
		gchar* text = (gchar*)item->data;

		if (g_strcmp0(text, filename) == 0) {
			window->filenames = g_list_remove(window->filenames, text);
			removed = TRUE;
		}
		item = next;
	}

	if (removed == FALSE) {
		g_error("Could not remove file from quick list.");
	}

	window->file_count--;
	window->filenames = g_list_remove(window->filenames, raider_file_row_get_filename(row));

	if (window->file_count == 0) {
		gtk_stack_set_visible_child_name(window->window_stack, "empty_page");

		/* Update the view. */
		gtk_revealer_set_reveal_child(window->shred_revealer, FALSE);
		gtk_revealer_set_reveal_child(window->abort_revealer, FALSE);
		gtk_revealer_set_reveal_child(window->open_revealer, TRUE);
	}
}

/* This is used to open a single file at a time */
void raider_window_open(GFile* file, gpointer data)
{
	RaiderWindow *window = RAIDER_WINDOW(data);

	if (g_file_query_exists(file, NULL) == FALSE) {
		g_object_unref(file);
		return;
	}
	if (g_file_query_file_type(file, G_FILE_QUERY_INFO_NONE, NULL) == G_FILE_TYPE_DIRECTORY) {
		g_autofree char *msg = g_strdup(_("Directories are not supported!"));
		adw_toast_overlay_add_toast(window->toast_overlay, adw_toast_new(msg));
		g_object_unref(file);
		return;
	}

	/* Search to see if a file with that path is already loaded. */
	GList* item = window->filenames;
	gchar* filename = g_file_get_path(file);

	while (item != NULL) {
		GList *next = item->next;

		/* Get the filename for this round. */
		gchar* text = (gchar*)item->data;

		if (g_strcmp0(text, filename) == 0) {
			g_autofree char *msg = g_strdup_printf(_("“%s” is already loaded!"), g_file_get_basename(file));
			adw_toast_overlay_add_toast(window->toast_overlay, adw_toast_new(msg));
			g_object_unref(file);
			return;
		}
		item = next;
	}

	GtkWidget *file_row = GTK_WIDGET(raider_file_row_new(file));
	gtk_list_box_append(window->list_box, file_row);

	gtk_stack_set_visible_child_name(GTK_STACK(window->window_stack), "list_page");
	gtk_revealer_set_reveal_child(GTK_REVEALER(window->shred_revealer), TRUE);

	window->file_count++;
	window->filenames = g_list_append(window->filenames, g_file_get_path(file));
}

