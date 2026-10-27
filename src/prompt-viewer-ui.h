/**
 * @file      prompt-viewer-ui.h
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
#ifndef PVP_PROMPT_VIEWER_UI_H
#define PVP_PROMPT_VIEWER_UI_H
#include <gtk/gtk.h>
#include <promptex/promptex.h>

struct LabelButtonWidgets {
    GtkWidget* box;
    GtkWidget* label;
    GtkWidget* button;
};
typedef struct LabelButtonWidgets LabelButtonWidgets;

LabelButtonWidgets pvpui_create_label_button(
    const gchar* label_text,
    const gchar* button_text
);


struct TextAreaWidgets {
    GtkWidget* box;         // El contenedor principal (GtkBox)
    GtkWidget* label;       // El GtkLabel del título del Frame
    GtkWidget* copy_button; // El GtkButton para copiar el texto al clipboard
    GtkWidget* text_view;   // El GtkTextView para el contenido
};
typedef struct TextAreaWidgets TextAreaWidgets;

TextAreaWidgets pvpui_create_text_area();




GtkWidget* pvpui_create_pane_array(GtkWidget* widgets[], int count);
GtkWidget* pvpui_create_pane4(GtkWidget* top,
                              GtkWidget* middle_top,
                              GtkWidget* middle_bottom,
                              GtkWidget* bottom);

                              




struct ParametersWidgets {
    GtkWidget* box;
    GtkWidget* sampler_field;
    GtkWidget* steps_field;
    GtkWidget* cfg_field;
    GtkWidget* seed_field;
    GtkWidget* width_field;
    GtkWidget* height_field;
};
typedef struct ParametersWidgets ParametersWidgets;

ParametersWidgets pvpui_create_parameters_widgets();

struct ToolBarWidgets {
    GtkWidget*   box;
    GtkToolItem* copy_button;
    GtkToolItem* preferences_button;
};
typedef struct ToolBarWidgets ToolBarWidgets;

struct MessageWidgets {
    GtkWidget* box;
    GtkWidget* label;
};
typedef struct MessageWidgets MessageWidgets;

struct LoadingWidgets {
    GtkWidget* box;
    GtkWidget* spinner;
};
typedef struct LoadingWidgets LoadingWidgets;

struct DataWidgets {
    GtkWidget*        box;
    TextAreaWidgets   prompt;
    TextAreaWidgets   negative_prompt;
    TextAreaWidgets   widlcard_prompt;
    ParametersWidgets parameters;
};
typedef struct DataWidgets DataWidgets;

struct PromptViewerUI {
    GtkWidget*      box;
    ToolBarWidgets  toolbar;
    MessageWidgets  message;
    LoadingWidgets  loading;
    DataWidgets     data;
};
typedef struct PromptViewerUI PromptViewerUI;



PromptViewerUI pvp_create_prompt_viewer_ui(void);
void pvpui_hide_all_widgets(const PromptViewerUI* ui);
void pvpui_show_message(const PromptViewerUI* ui, const gchar* message);
void pvpui_show_params(const PromptViewerUI* ui, const PeParams* params);


#endif /*PVP_PROMPT_VIEWER_UI_H*/
