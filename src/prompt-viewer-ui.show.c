/**
 * @file      prompt-viewer-ui.show.h
 * @author    Martin Rizzo | <martinrizzo@gmail.com>
 * @date      Oct 28, 2026
 * @brief     Functions to display the 3 different UI modes (text message, loading, extraction)
 * @copyright Copyright (c) 2023-2026 Martin Rizzo
 *            Distributed under the MIT License.
 * @see       https://github.com/martin-rizzo/EOG-PromptViewerPlugin
 *//*
+- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -+
                       Prompt Viewer Plugin for EOG
 An "Eye of GNOME" plugin to display prompts embedded in AI-generated images.
+_ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _*/
#include <gtk/gtk.h>
#include <promptex/promptex.h>
#include "prompt-viewer-ui.h"
#include "prompt-viewer-ui.show.h"

/**
 * Displays a text string in a GTK widget.
 *
 * This function updates the displayed text of a GTK widget based on its type.
 * It supports the following widget types: GtkLabel, GtkEntry, GtkTextView,
 * and GtkButton. If the widget type is not supported, a warning is logged.
 *
 * @param textwidget The GTK widget to update.
 * @param text       The text string to display.
 */
static
void show_text(GtkWidget* widget, const gchar* text) {
    if (!widget || !text) { return; }

    if (GTK_IS_LABEL(widget)) {
        gtk_label_set_text(GTK_LABEL(widget), text);
    }
    else if (GTK_IS_ENTRY(widget)) {
        gtk_entry_set_text(GTK_ENTRY(widget), text);
    }
    else if (GTK_IS_BUTTON(widget)) {
        // update the label of a button widget
        gtk_button_set_label(GTK_BUTTON(widget), text);
    }
    else if (GTK_IS_TEXT_VIEW(widget)) {
        // update the content of a multiline text view
        // GtkTextView stores text in its associated GtkTextBuffer
        GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(widget));
        gtk_text_buffer_set_text(buffer, text, -1); // -1 means the string is null-terminated
    }
    else {
        // fallback for unsupported widget types
        g_warning("show_text: Widget type '%s' is not supported or cannot display text.", 
                  G_OBJECT_TYPE_NAME(widget));
    }
}

/**
 * Updates the text of a GTK widget and toggles the visibility of its container box.
 *
 * If the provided prompt contains a valid value, the widget's text is
 * updated using the show_text function and the container box is made visible.
 * Otherwise, the box is hidden.
 *
 * @param box        The container widget to toggle visibility.
 * @param textwidget The GTK widget to update (can be GtkLabel, GtkEntry, GtkTextView, GtkButton, ...).
 * @param prompt     The optional prompt string value to display.
 */
static
void show_prompt(GtkWidget* box, GtkWidget* textwidget, const PeString* prompt) {
    const bool show = (prompt && prompt->has_value);
    if (box)                { gtk_widget_set_visible(box, show); }
    if (textwidget && show) { show_text(textwidget, prompt->value); }
}

/**
 * Updates a GTK widget with an optional long integer value.
 *
 * If the provided long value is present, it is converted to a string
 * and displayed in the widget. If the value is absent, the widget is
 * updated with an empty string.
 *
 * @param textwidget The GTK widget to update (can be GtkLabel, GtkEntry, GtkTextView, GtkButton, ...).
 * @param long_      The optional long integer value to display.
 */
static
void show_long(GtkWidget* textwidget, const PeLong* long_) {
    if (!textwidget) { return; }
    gchar* str_value = (long_ && long_->has_value) ? g_strdup_printf("%ld", long_->value) : NULL;
    show_text(textwidget, str_value ? str_value : "");
    g_free(str_value);
}

/**
 * Updates a GTK widget with a optional floating-point value.
 *
 * If the provided float value is present, it is formatted according to
 * the specified number of decimal places and displayed in the widget.
 * If the value is absent, the widget is updated with an empty string.
 *
 * @param textwidget     The GTK widget to update (can be GtkLabel, GtkEntry, GtkTextView, GtkButton, ...).
 * @param float_         The optional floating-point value to display.
 * @param decimal_places The number of decimal places to use for formatting.
 */
static
void show_float(GtkWidget* textwidget, const PeFloat* float_, int decimal_places) {
    if (!textwidget) { return; }
    gchar* str_value = NULL;
    if (float_ && float_->has_value) {
        switch( decimal_places ) {
            case 0:  str_value = g_strdup_printf("%.0f", float_->value); break;
            case 1:  str_value = g_strdup_printf("%.1f", float_->value); break;
            case 2:  str_value = g_strdup_printf("%.2f", float_->value); break;
            default: str_value = g_strdup_printf("%.3f", float_->value); break;
        }
    }
    show_text(textwidget, str_value ? str_value : "");
    g_free(str_value);
}


/**
 * Updates a GTK widget with an optional string value.
 *
 * If the provided string value is present, it is displayed in the widget.
 * If the value is absent, the widget is updated with an empty string.
 *
 * @param textwidget The GTK widget to update (can be GtkLabel, GtkEntry, GtkTextView, GtkButton, ...).
 * @param string     The optional string value to display.
 */
static
void show_string(GtkWidget* textwidget, const PeString* string) {
    if (!textwidget) { return; }
    if (string && string->has_value) { show_text(textwidget, string->value); }
    else                             { show_text(textwidget, "");            }
}

/*===========================================================================
|  ////////////////////////// PUBLIC FUNCTIONS ///////////////////////////  | 
===========================================================================*/

void pvui_hide_all_widgets(const PromptViewerUI* ui)
{
    gtk_widget_set_visible(ui->message.box   , FALSE);
    gtk_widget_set_visible(ui->loading.box   , FALSE);
    gtk_widget_set_visible(ui->extraction.box, FALSE);
}

void pvui_show_message(const PromptViewerUI* ui,
                       const gchar*          message
){
    if (!ui || !message) { return; }

    pvui_hide_all_widgets(ui);
    gtk_widget_set_visible(ui->message.box, TRUE);
    gtk_label_set_text(GTK_LABEL(ui->message.label), message);
}


void pvui_show_extraction(const PromptViewerUI* ui,
                          const PeParams* extracted_params
){
    const ExtractionWidgets* wid = &ui->extraction;
    const PeParams*          p   = extracted_params;
    // if( !params ) {
    //     pvui_show_message(ui,"No generation parameters found in the image.");
    //     return;
    // }

    pvui_hide_all_widgets(ui);
    gtk_widget_set_visible(wid->box, TRUE);

    show_prompt(wid->prompt.box         , wid->prompt.text_view         , &p->prompt         );
    show_prompt(wid->negative_prompt.box, wid->negative_prompt.text_view, &p->negative_prompt);
    show_prompt(wid->wildcard_prompt.box, wid->wildcard_prompt.text_view, &p->wildcard_prompt);

    show_string(wid->parameters.sampler_field, &p->sampler);
    show_long  (wid->parameters.steps_field  , &p->steps  );
    show_float (wid->parameters.cfg_field    , &p->cfg, 1 );
    show_long  (wid->parameters.seed_field   , &p->seed   );
    show_long  (wid->parameters.width_field  , &p->width  );
    show_long  (wid->parameters.height_field , &p->height );
}
