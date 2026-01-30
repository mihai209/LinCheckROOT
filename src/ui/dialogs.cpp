#include "ui/dialogs.hpp"

namespace ui {

// GTK4 compatible dialog helper structures
static bool dialog_response_ok = false;

static void on_dialog_response(GtkDialog* dialog, gint response_id, gpointer user_data) {
    dialog_response_ok = (response_id == GTK_RESPONSE_OK);
    gtk_window_close(GTK_WINDOW(dialog));
}

bool Dialogs::confirm_reboot(GtkWindow* parent,
                            const std::string& reboot_type,
                            const std::string& description,
                            const std::string& warning) {
    std::string message = description;
    if (!warning.empty()) {
        message += "\n\n⚠️  " + warning;
    }
    message += "\n\nThis action cannot be undone. Continue?";

    dialog_response_ok = false;

    GtkWidget* dialog = gtk_dialog_new_with_buttons(
        ("Confirm: " + reboot_type).c_str(),
        parent,
        GTK_DIALOG_MODAL,
        "Cancel", GTK_RESPONSE_CANCEL,
        "Reboot", GTK_RESPONSE_OK,
        NULL
    );

    GtkWidget* content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget* label = gtk_label_new(message.c_str());
    gtk_label_set_wrap(GTK_LABEL(label), TRUE);
    gtk_widget_set_margin_start(label, 12);
    gtk_widget_set_margin_end(label, 12);
    gtk_widget_set_margin_top(label, 12);
    gtk_widget_set_margin_bottom(label, 12);
    gtk_box_append(GTK_BOX(content_area), label);

    g_signal_connect(dialog, "response", G_CALLBACK(on_dialog_response), NULL);
    gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);
    gtk_widget_show(dialog);

    // Simple blocking wait (GTK4 replacement for gtk_dialog_run)
    while (gtk_widget_get_visible(dialog)) {
        g_main_context_iteration(NULL, FALSE);
    }

    return dialog_response_ok;
}

void Dialogs::show_warning(GtkWindow* parent,
                          const std::string& title,
                          const std::string& message) {
    GtkWidget* dialog = gtk_dialog_new_with_buttons(
        title.c_str(),
        parent,
        GTK_DIALOG_MODAL,
        "OK", GTK_RESPONSE_OK,
        NULL
    );

    GtkWidget* content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget* label = gtk_label_new(message.c_str());
    gtk_label_set_wrap(GTK_LABEL(label), TRUE);
    gtk_widget_set_margin_start(label, 12);
    gtk_widget_set_margin_end(label, 12);
    gtk_widget_set_margin_top(label, 12);
    gtk_widget_set_margin_bottom(label, 12);
    gtk_box_append(GTK_BOX(content_area), label);

    g_signal_connect(dialog, "response", G_CALLBACK(on_dialog_response), NULL);
    gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);
    gtk_widget_show(dialog);

    while (gtk_widget_get_visible(dialog)) {
        g_main_context_iteration(NULL, FALSE);
    }
}

void Dialogs::show_error(GtkWindow* parent,
                        const std::string& title,
                        const std::string& message) {
    GtkWidget* dialog = gtk_dialog_new_with_buttons(
        title.c_str(),
        parent,
        GTK_DIALOG_MODAL,
        "OK", GTK_RESPONSE_OK,
        NULL
    );

    GtkWidget* content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget* label = gtk_label_new(message.c_str());
    gtk_label_set_wrap(GTK_LABEL(label), TRUE);
    gtk_widget_set_margin_start(label, 12);
    gtk_widget_set_margin_end(label, 12);
    gtk_widget_set_margin_top(label, 12);
    gtk_widget_set_margin_bottom(label, 12);
    gtk_box_append(GTK_BOX(content_area), label);

    g_signal_connect(dialog, "response", G_CALLBACK(on_dialog_response), NULL);
    gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);
    gtk_widget_show(dialog);

    while (gtk_widget_get_visible(dialog)) {
        g_main_context_iteration(NULL, FALSE);
    }
}

void Dialogs::show_info(GtkWindow* parent,
                       const std::string& title,
                       const std::string& message) {
    GtkWidget* dialog = gtk_dialog_new_with_buttons(
        title.c_str(),
        parent,
        GTK_DIALOG_MODAL,
        "OK", GTK_RESPONSE_OK,
        NULL
    );

    GtkWidget* content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget* label = gtk_label_new(message.c_str());
    gtk_label_set_wrap(GTK_LABEL(label), TRUE);
    gtk_widget_set_margin_start(label, 12);
    gtk_widget_set_margin_end(label, 12);
    gtk_widget_set_margin_top(label, 12);
    gtk_widget_set_margin_bottom(label, 12);
    gtk_box_append(GTK_BOX(content_area), label);

    g_signal_connect(dialog, "response", G_CALLBACK(on_dialog_response), NULL);
    gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);
    gtk_widget_show(dialog);

    while (gtk_widget_get_visible(dialog)) {
        g_main_context_iteration(NULL, FALSE);
    }
}

} // namespace ui
