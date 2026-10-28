/**
 * @file      prompt-viewer-ui.c
 * @author    Martin Rizzo | <martinrizzo@gmail.com>
 * @date      Oct 26, 2026
 * @brief     GTK UI components for Prompt Viewer Plugin
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
#include "prompt-viewer-ui.resizer.h"


/**
 * Applies a background color to a GTK widget for visual debugging.
 * @param widget     The GTK widget to apply the color to.
 * @param color_css  The CSS color string (e.g., "red", "#ff0000", "rgba(0,255,0,0.5)").
 */
static void
pvui_apply_debug_color(GtkWidget *widget, const gchar *color_css) {
    if (!widget || !color_css) { return; };

    // create a CSS style provider
    GtkCssProvider *provider = gtk_css_provider_new();
    
    // load the CSS with the received color into the provider
    gchar css_buffer[256];
    snprintf(css_buffer, sizeof(css_buffer), "* { background-color: %s; }", color_css);
    gtk_css_provider_load_from_data(provider, css_buffer, -1, NULL);
    
    // add the provider to the widget and release the provider
    // since the widget holds a reference to it
    GtkStyleContext *context = gtk_widget_get_style_context(widget);
    gtk_style_context_add_provider(context, 
                                   GTK_STYLE_PROVIDER(provider), 
                                   GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref(provider);
}

/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/**
 * A structure representing a label and button pair within a container.
 */
typedef struct LabelAndButtonWidgets {
    GtkWidget* box;      ///< The container.
    GtkWidget* label;    ///< The GtkLabel widget.
    GtkWidget* button;   ///< The GtkButton widget.
} LabelAndButtonWidgets;

/**
 * Creates a horizontal box containing a label and a button.
 *
 * @param label_text  The text for the label.
 * @param button_text The text for the button.
 * @return A `LabelAndButtonWidgets` structure containing the container box,
 *         the label, and the button.
 */
static LabelAndButtonWidgets
pvui_create_label_and_button(const gchar* label_text,
                             const gchar* button_text
){
    // create the main container (horizontal GtkBox)
    GtkWidget* box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    gtk_widget_set_visible(box, TRUE);
    
    // `GtkLabel` on the left
    GtkWidget *label = gtk_label_new(label_text ? label_text : "Label");
    gtk_widget_set_visible(label, TRUE);
    gtk_box_pack_start(GTK_BOX(box), label, FALSE, TRUE, 0);
    
    // `GtkSeparator` between label and button
    GtkWidget *expander = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_box_pack_start(GTK_BOX(box), expander, TRUE, FALSE, 0);
    
    // `GtkButton` on the right
    GtkWidget *button = gtk_button_new_with_label(button_text ? button_text : "Button");
    gtk_widget_set_visible(button, TRUE);
    gtk_widget_set_can_focus(button, TRUE);
    gtk_box_pack_start(GTK_BOX(box), button, FALSE, TRUE, 0);
    
    return (LabelAndButtonWidgets){
        .box    = box,
        .label  = label,
        .button = button
    };
}

/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/**
 * A structure representing two fields (each field being a label and entry pair).
 */
typedef struct TwoFieldsWidgets {
    GtkWidget* box;     ///< The container.
    GtkWidget* label1;  ///< The GtkLabel with the field name.
    GtkWidget* entry1;  ///< The GtkEntry with the field value.
    GtkWidget* label2;  ///< The GtkLabel with the second field name.
    GtkWidget* entry2;  ///< The GtkEntry with the second field value.
} TwoFieldsWidgets;


/**
 * Creates a horizontal box containing two fields.
 *
 * @param name1  The text for the first label.
 * @param width1 The width in characters for the first entry (-1 for default).
 * @param name2  The text for the second label.
 * @param width2 The width in characters for the second entry (-1 for default).
 * @return A `TwoFieldsWidgets` structure containing the container box,
 *         the labels, and the entry fields.
 */
static TwoFieldsWidgets
pvui_create_two_fields_widgets(const gchar* name1,
                               int          width1,
                               const gchar* name2,
                               int          width2
){
    // create the main container (horizontal GtkBox)
    GtkWidget* hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6); // 6px standard spacing
    gtk_widget_set_visible(hbox, TRUE);

    // expandable separator on the left to force right alignment
    GtkWidget* spacer = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_widget_set_visible(spacer, TRUE);
    gtk_box_pack_start(GTK_BOX(hbox), spacer, TRUE, TRUE, 0);

    // first field
    GtkWidget* label1 = gtk_label_new(name1);
    gtk_widget_set_visible(label1, TRUE);
    GtkWidget* entry1 = gtk_entry_new();
    gtk_widget_set_visible(entry1, TRUE);
    if (width1 >= 0) {
        gtk_entry_set_width_chars(GTK_ENTRY(entry1), width1);
    }

    // second field
    GtkWidget* label2 = gtk_label_new(name2);
    gtk_widget_set_visible(label2, TRUE);
    GtkWidget* entry2 = gtk_entry_new();
    gtk_widget_set_visible(entry2, TRUE);
    if (width2 >= 0) {
        gtk_entry_set_width_chars(GTK_ENTRY(entry2), width2);
    }

    // pack everything in order within the horizontal container
    gtk_box_pack_start(GTK_BOX(hbox), label1, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), entry1, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), label2, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), entry2, FALSE, FALSE, 0);

    return (TwoFieldsWidgets){
        .box    = hbox,
        .label1 = label1,
        .entry1 = entry1,
        .label2 = label2,
        .entry2 = entry2
    };
}

/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/**
 * Creates a text area widget component for displaying prompts.
 *
 * Constructs a vertical box containing a header (label and copy button), a
 * text view, and a resizer handle. The text view is set to word wrapping and
 * is non-editable.
 *
 * @return A `TextAreaWidgets` structure containing the container box,
 *         the header label, the copy button, and the text view widget.
 */
static TextAreaWidgets
pvui_create_text_area(
){

    // create the main container (GtkBox)
    GtkWidget* vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_visible(vbox, TRUE);

    // top GtkSeparator
    GtkWidget *separator = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_widget_set_visible(separator, TRUE);

    // header with label and button
    LabelAndButtonWidgets label_button = pvui_create_label_and_button("Prompt", "Copy");

    // container GtkFrame
    GtkWidget *frame = gtk_frame_new(NULL);
    gtk_widget_set_visible(frame, TRUE);
    gtk_frame_set_label_align(GTK_FRAME(frame), 0.0f, 0.5f);
    gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_NONE);

    // GtkScrolledWindow inside the frame
    GtkWidget *scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_widget_set_visible(scrolled_window, TRUE);
    gtk_widget_set_can_focus(scrolled_window, TRUE);
    gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scrolled_window), GTK_SHADOW_IN);

    // apply "group-box" CSS class to the ScrolledWindow
    GtkStyleContext *context_scrolled = gtk_widget_get_style_context(scrolled_window);
    gtk_style_context_add_class(context_scrolled, "group-box");

    // GtkTextView inside the ScrolledWindow
    GtkWidget *prompt_text_view = gtk_text_view_new();
    gtk_widget_set_visible(prompt_text_view, TRUE);
    gtk_widget_set_can_focus(prompt_text_view, TRUE);
    gtk_text_view_set_editable(GTK_TEXT_VIEW(prompt_text_view), FALSE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(prompt_text_view), GTK_WRAP_WORD);

    // assemble the internal hierarchy
    gtk_container_add(GTK_CONTAINER(scrolled_window), prompt_text_view);
    gtk_container_add(GTK_CONTAINER(frame)          , scrolled_window);

    // crear un resizer que el usuario pueda arrastrar para cambiar el tamaño del vbox
    GtkWidget* resizer = pvui_create_resizer(vbox, 30, 8*1024);

    // empaquetar todo en secuencia dentro del vbox
    gtk_box_pack_start(GTK_BOX(vbox), separator       , FALSE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), label_button.box, FALSE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), frame           , TRUE , TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), resizer         , FALSE, TRUE, 0);

    return (TextAreaWidgets){
        .box         = vbox,
        .label       = label_button.label,
        .copy_button = label_button.button,
        .text_view   = prompt_text_view
    };
}

/*======================== CREATING WIDGET GROUPS ========================*/

/**
 * Creates the toolbar widget group containing the toolbar buttons.
 * @return
 *     A `ToolBarWidgets` structure containing the toolbar box and its buttons.
 */
ToolBarWidgets pvui_create_toolbar_widgets(void) {

    // create the main container (GtkToolbar)
    GtkWidget *toolbar = gtk_toolbar_new();
    gtk_widget_set_visible(toolbar, TRUE);
    gtk_widget_set_can_focus(toolbar, FALSE);
    gtk_toolbar_set_style(GTK_TOOLBAR(toolbar), GTK_TOOLBAR_BOTH_HORIZ);

    // first element: GtkToolButton (copy_button)
    // has a custom CSS class (<style><class name="noticeable"/></style>)
    GtkToolItem *copy_button = gtk_tool_button_new(NULL, "Copy Image Generation Data");
    gtk_widget_set_visible(GTK_WIDGET(copy_button), TRUE);
    gtk_widget_set_can_focus(GTK_WIDGET(copy_button), FALSE);
    gtk_tool_button_set_use_underline(GTK_TOOL_BUTTON(copy_button), TRUE);
    gtk_tool_item_set_expand(copy_button, TRUE);
    gtk_tool_item_set_homogeneous(copy_button, TRUE);
    GtkStyleContext *copy_ctx = gtk_widget_get_style_context(GTK_WIDGET(copy_button));
    gtk_style_context_add_class(copy_ctx, "noticeable");
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), copy_button, -1);

    // second element: GtkSeparatorToolItem
    GtkToolItem *separator = gtk_separator_tool_item_new();
    gtk_widget_set_visible(GTK_WIDGET(separator), TRUE);
    gtk_widget_set_can_focus(GTK_WIDGET(separator), FALSE);
    gtk_tool_item_set_expand(separator, FALSE);
    gtk_tool_item_set_homogeneous(separator, TRUE);
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), separator, -1);

    // third element: GtkToolButton (preferences_button)
    GtkToolItem *preferences_button = gtk_tool_button_new(NULL, "Preferences");
    gtk_widget_set_visible(GTK_WIDGET(preferences_button), TRUE);
    gtk_widget_set_can_focus(GTK_WIDGET(preferences_button), FALSE);
    gtk_tool_button_set_use_underline(GTK_TOOL_BUTTON(preferences_button), TRUE);
    gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON(preferences_button), "preferences-other-symbolic");
    gtk_tool_item_set_expand(preferences_button, FALSE);
    gtk_tool_item_set_homogeneous(preferences_button, TRUE);
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), preferences_button, -1);

    return (ToolBarWidgets){
        .box                = toolbar,
        .copy_button        = copy_button,
        .preferences_button = preferences_button
    };
}


/**
 * Creates the message widget group for displaying informational text.
 * @return
 *     A `MessageWidgets` structure containing the message box and label.
 */
MessageWidgets pvui_create_message_widgets(void) {

    // create the main container (vertical GtkBox)
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_visible(box, TRUE);
    gtk_widget_set_can_focus(box, FALSE);

    // single element: GtkLabel (message_label)
    GtkWidget *message_label = gtk_label_new("< No prompt information >");
    gtk_widget_set_visible(message_label, TRUE);
    gtk_widget_set_can_focus(message_label, FALSE);
    gtk_widget_set_opacity(message_label, 0.5);
    gtk_box_pack_start(GTK_BOX(box), message_label, FALSE, TRUE, 0);

    return (MessageWidgets){
        .box   = box,
        .label = message_label
    };
}


/**
 * Creates the loading widget group for indicating ongoing processes.
 * @return
 *     A `LoadingWidgets` structure containing the loading box and spinner.
 */
LoadingWidgets pvui_create_loading_widgets(void) {

    // create the main container (vertical GtkBox)
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_visible(box, TRUE);
    gtk_widget_set_can_focus(box, FALSE);

    // single element: GtkSpinner (spinner)
    GtkWidget *spinner = gtk_spinner_new();
    gtk_widget_set_visible(spinner, TRUE);
    gtk_widget_set_can_focus(spinner, FALSE);
    gtk_widget_set_size_request(spinner, -1, 64);
    gtk_spinner_start(GTK_SPINNER(spinner));
    gtk_box_pack_start(GTK_BOX(box), spinner, FALSE, TRUE, 0);

    return (LoadingWidgets){
        .box     = box,
        .spinner = spinner
    };
}


/**
 * Creates the parameters widget group for displaying generation settings.
 * @return
 *     A `ParametersWidgets` structure containing the parameters box and entry fields.
 */
ParametersWidgets pvui_create_parameters_widgets() {

    // create the main container (vertical GtkBox)
    GtkWidget* box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_visible(box, TRUE);

    // the numbers represent the width in characters of the GtkEntry;
    // everything should be right-aligned, so the box should be an hbox
    // with an expandable spacer on the left
    TwoFieldsWidgets sampler_steps = pvui_create_two_fields_widgets("Sampler",-1, "Steps",4);
    gtk_box_pack_start(GTK_BOX(box), sampler_steps.box, FALSE, TRUE, 0);

    TwoFieldsWidgets cfg_width = pvui_create_two_fields_widgets("CFG Scale",4,"Width",5);
    gtk_box_pack_start(GTK_BOX(box), cfg_width.box, FALSE, TRUE, 0);

    TwoFieldsWidgets seed_height = pvui_create_two_fields_widgets("Seed",10,"Height",5);
    gtk_box_pack_start(GTK_BOX(box), seed_height.box, FALSE, TRUE, 0);

    return (ParametersWidgets){
        .box           = box,
        .sampler_field = sampler_steps.entry1,
        .steps_field   = sampler_steps.entry2,
        .cfg_field     = cfg_width.entry1,
        .width_field   = cfg_width.entry2,
        .seed_field    = seed_height.entry1,
        .height_field  = seed_height.entry2
    };
}


/**
 * Creates the data widget group for displaying prompts and parameters.
 * @return
 *     A `DataWidgets` structure containing the data box and all sub-widgets.
 */
DataWidgets pvui_create_data_widgets(void) {

    TextAreaWidgets   prompt          = pvui_create_text_area();
    TextAreaWidgets   negative_prompt = pvui_create_text_area();
    TextAreaWidgets   wildcard_prompt = pvui_create_text_area();
    ParametersWidgets parameters      = pvui_create_parameters_widgets();
    
    GtkWidget* vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_box_pack_start(GTK_BOX(vbox), prompt.box         , FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), negative_prompt.box, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), wildcard_prompt.box, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), parameters.box     , FALSE, FALSE, 0);

    return (DataWidgets){
        .box             = vbox,
        .prompt          = prompt,
        .negative_prompt = negative_prompt,
        .wildcard_prompt = wildcard_prompt,
        .parameters      = parameters
    };
}


/*===========================================================================
|  ////////////////////////// PUBLIC FUNCTIONS ///////////////////////////  | 
===========================================================================*/

/**
 * Creates the complete UI widget structure for the Prompt Viewer plugin.
 *
 * Initializes and assembles the four main UI groups (toolbar, message, loading,
 * and data) into a single vertical box layout. This function serves as the main
 * entry point for constructing the plugin's side panel interface.
 *
 * @return A fully initialized PromptViewerUI structure containing all widgets.
 */
PromptViewerUI pvui_create_prompt_viewer_ui(void) {
    PromptViewerUI ui;

    // the 4 widget groups of the Prompt Viewer UI
    ui.toolbar = pvui_create_toolbar_widgets();
    ui.message = pvui_create_message_widgets();
    ui.loading = pvui_create_loading_widgets();
    ui.data    = pvui_create_data_widgets();

    // crear un box interno para los elementos que irán dentro del scroll
    GtkWidget *scroll_content_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_visible(scroll_content_box, TRUE);
    gtk_box_pack_start(GTK_BOX(scroll_content_box), ui.message.box, FALSE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(scroll_content_box), ui.loading.box, FALSE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(scroll_content_box), ui.data.box   , TRUE , TRUE, 0);

    // crear el contenedor con scroll vertical y añadirle el contenido
    GtkWidget *scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_widget_set_visible(scrolled_window, TRUE);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window),
                                   GTK_POLICY_NEVER, 
                                   GTK_POLICY_ALWAYS);
    gtk_container_add(GTK_CONTAINER(scrolled_window), scroll_content_box);

    // crear el VBox principal con solo los 2 componentes principales
    ui.box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_visible(ui.box, TRUE);
    gtk_box_pack_start(GTK_BOX(ui.box), ui.toolbar.box, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(ui.box), scrolled_window, TRUE, TRUE, 0);

    return ui;
}

void pvui_hide_all_widgets(const PromptViewerUI* ui)
{
    gtk_widget_set_visible(ui->message.box, FALSE);
    gtk_widget_set_visible(ui->loading.box, FALSE);
    gtk_widget_set_visible(ui->data.box   , FALSE);
}

void pvui_show_message(const PromptViewerUI* ui,
                       const gchar*          message
){
    if (!ui || !message) { return; }

    pvui_hide_all_widgets(ui);
    gtk_widget_set_visible(ui->message.box, TRUE);
    gtk_label_set_text(GTK_LABEL(ui->message.label), message);
}

void pvui_show_params(const PromptViewerUI* ui,
                      const PeParams*       params
){
    // if( !params ) {
    //     pvui_show_message(ui,"No generation parameters found in the image.");
    //     return;
    // }

    pvui_hide_all_widgets(ui);
    gtk_widget_set_visible(ui->data.box, TRUE);

    bool show_prompt = params->prompt.has_value;
    gtk_widget_set_visible(ui->data.prompt.box, show_prompt);
    if (show_prompt) {
        GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(ui->data.prompt.text_view));
        gtk_text_buffer_set_text(buffer, params->prompt.value, -1);
    }

    bool show_negative = params->negative_prompt.has_value;
    gtk_widget_set_visible(ui->data.negative_prompt.box, show_negative);
    if (show_negative) {
        GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(ui->data.negative_prompt.text_view));
        gtk_text_buffer_set_text(buffer, params->prompt.value, -1);
    }

    bool show_wildcard = params->wildcard_prompt.has_value;
    gtk_widget_set_visible(ui->data.wildcard_prompt.box, show_wildcard);
    if (show_wildcard) {
        GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(ui->data.wildcard_prompt.text_view));
        gtk_text_buffer_set_text(buffer, params->wildcard_prompt.value, -1);
    }


}
