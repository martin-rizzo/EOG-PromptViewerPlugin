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
#ifndef PVP_PROMPT_VIEWER_UI_SHOW_H
#define PVP_PROMPT_VIEWER_UI_SHOW_H
#include <gtk/gtk.h>
#include <promptex/promptex.h>
#include "prompt-viewer-ui.h"


void pvui_show_message(const PromptViewerUI* ui, const gchar* message);
void pvui_show_loading(const PromptViewerUI* ui, const gchar* loading_message);
void pvui_show_extraction(const PromptViewerUI* ui, const PeParams* extracted_params);
void pvui_hide_all_widgets(const PromptViewerUI* ui);


#endif /*PVP_PROMPT_VIEWER_UI_SHOW_H*/
