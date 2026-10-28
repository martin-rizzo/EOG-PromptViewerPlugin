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



typedef struct TextAreaWidgets {
    GtkWidget* box;         // El contenedor principal (GtkBox)
    GtkWidget* label;       // El GtkLabel del título del Frame
    GtkWidget* copy_button; // El GtkButton para copiar el texto al clipboard
    GtkWidget* text_view;   // El GtkTextView para el contenido
} TextAreaWidgets;


typedef struct ParametersWidgets {
    GtkWidget* box;
    GtkWidget* sampler_field;
    GtkWidget* steps_field;
    GtkWidget* cfg_field;
    GtkWidget* seed_field;
    GtkWidget* width_field;
    GtkWidget* height_field;
} ParametersWidgets;

typedef struct HighresFixWidgets {
    GtkWidget* box;
    GtkWidget* upscaler_entry;
    GtkWidget* steps_entry;
    GtkWidget* denoising_entry;
    GtkWidget* upscaleby_entry;
    GtkWidget* width_entry;
    GtkWidget* height_entry;
} HighresFixWidgets;

typedef struct InpaintingWidgets {
    GtkWidget* box;
    GtkWidget* denoising_entry;
    GtkWidget* maskblur_entry;
} InpaintingWidgets;


typedef struct ToolBarWidgets {
    GtkWidget*   box;
    GtkToolItem* copy_button;
    GtkToolItem* preferences_button;
} ToolBarWidgets;


typedef struct MessageWidgets {
    GtkWidget* box;
    GtkWidget* label;
} MessageWidgets;


typedef struct LoadingWidgets {
    GtkWidget* box;
    GtkWidget* spinner;
} LoadingWidgets;


typedef struct ExtractionWidgets {
    GtkWidget*        box;
    TextAreaWidgets   prompt;
    TextAreaWidgets   negative_prompt;
    TextAreaWidgets   wildcard_prompt;
    ParametersWidgets parameters;
    HighresFixWidgets highresfix;
    InpaintingWidgets inpainting;
} ExtractionWidgets;



/**
 * The main structure containing all widgets for the Prompt Viewer UI.
 *
 * This structure serves as the root container for the entire Prompt Viewer
 * plugin interface, aggregating the toolbar, message display, loading
 * indicator, and data panels (prompts and parameters).
 */
typedef struct PromptViewerUI {
    GtkWidget*        box;       ///< The main container widget for the entire UI.
    ToolBarWidgets    toolbar;   ///< The toolbar widget group containing action buttons.
    MessageWidgets    message;   ///< The message widget group for displaying informational text.
    LoadingWidgets    loading;   ///< The loading widget group for indicating ongoing processes.
    ExtractionWidgets extraction;///< The data widget group displaying the extracted prompt and parameters.
} PromptViewerUI;


PromptViewerUI pvui_create_prompt_viewer_ui(void);


#endif /*PVP_PROMPT_VIEWER_UI_H*/
