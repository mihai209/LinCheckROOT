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

void Dialogs::show_command_details(GtkWindow* parent,
                                  const std::string& command,
                                  const std::string& output,
                                  bool success) {
    std::string title = success ? "✅ Command Executed" : "⚠️  Command Issues";
    
    GtkWidget* dialog = gtk_dialog_new_with_buttons(
        title.c_str(),
        parent,
        GTK_DIALOG_MODAL,
        "Copy Command", GTK_RESPONSE_YES,
        "OK", GTK_RESPONSE_OK,
        NULL
    );

    gtk_window_set_default_size(GTK_WINDOW(dialog), 550, 500);
    
    GtkWidget* content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    gtk_box_set_spacing(GTK_BOX(content_area), 12);
    
    // Main scrolled container
    GtkWidget* main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    gtk_widget_set_margin_start(main_box, 12);
    gtk_widget_set_margin_end(main_box, 12);
    gtk_widget_set_margin_top(main_box, 12);
    gtk_widget_set_margin_bottom(main_box, 12);
    
    // Command section
    GtkWidget* cmd_label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(cmd_label), "<b>Command to execute:</b>");
    gtk_label_set_xalign(GTK_LABEL(cmd_label), 0.0);
    gtk_box_append(GTK_BOX(main_box), cmd_label);
    
    // Command in frame
    GtkWidget* cmd_frame = gtk_frame_new(NULL);
    GtkWidget* cmd_text = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(cmd_text), FALSE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(cmd_text), GTK_WRAP_WORD);
    gtk_text_view_set_monospace(GTK_TEXT_VIEW(cmd_text), TRUE);
    GtkTextBuffer* cmd_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(cmd_text));
    gtk_text_buffer_set_text(cmd_buffer, command.c_str(), -1);
    gtk_widget_set_size_request(cmd_text, 500, 50);
    gtk_frame_set_child(GTK_FRAME(cmd_frame), cmd_text);
    gtk_box_append(GTK_BOX(main_box), cmd_frame);
    
    // Output section
    GtkWidget* out_label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(out_label), "<b>Instructions / Output:</b>");
    gtk_label_set_xalign(GTK_LABEL(out_label), 0.0);
    gtk_box_append(GTK_BOX(main_box), out_label);
    
    // Output in scrolled window
    GtkWidget* scroll = gtk_scrolled_window_new();
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
                                   GTK_POLICY_AUTOMATIC,
                                   GTK_POLICY_AUTOMATIC);
    gtk_widget_set_vexpand(scroll, TRUE);
    gtk_widget_set_hexpand(scroll, TRUE);
    
    GtkWidget* out_text = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(out_text), FALSE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(out_text), GTK_WRAP_WORD);
    GtkTextBuffer* out_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(out_text));
    gtk_text_buffer_set_text(out_buffer, output.empty() ? "(no output)" : output.c_str(), -1);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroll), out_text);
    gtk_box_append(GTK_BOX(main_box), scroll);
    
    gtk_box_append(GTK_BOX(content_area), main_box);

    g_signal_connect(dialog, "response", G_CALLBACK(on_dialog_response), NULL);
    gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);
    gtk_window_present(GTK_WINDOW(dialog));

    while (gtk_widget_get_visible(dialog)) {
        g_main_context_iteration(NULL, FALSE);
    }
}

} // namespace ui
