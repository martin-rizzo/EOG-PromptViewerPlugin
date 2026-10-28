/**
 * @file      prompt-viewer-ui.resizer.h
 * @author    Martin Rizzo | <martinrizzo@gmail.com>
 * @date      Oct 28, 2026
 * @brief     A GTK UI component for vertically resizing another widget.
 * @copyright Copyright (c) 2023-2026 Martin Rizzo
 *            Distributed under the MIT License.
 * @see       https://github.com/martin-rizzo/EOG-PromptViewerPlugin
 *//*
+- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -+
                       Prompt Viewer Plugin for EOG
 An "Eye of GNOME" plugin to display prompts embedded in AI-generated images.
+_ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _*/
#ifndef PVP_PROMPT_VIEWER_UI_RESIZER_H
#define PVP_PROMPT_VIEWER_UI_RESIZER_H
#include <gtk/gtk.h>

/**
 * Creates a draggable grip for vertically resizing the target widget.
 *
 * This function creates a draggable grip widget that allows the user
 * to resize the height of the specified \p target_widget vertically.
 * The resizing is constrained between \p min_height and \p max_height.
 *
 * @param target_widget The widget whose height will be adjusted.
 * @param min_height    The minimum allowed height for the target widget.
 * @param max_height    The maximum allowed height for the target widget.
 * @return A pointer to the newly created grip widget (GtkEventBox).
 */
GtkWidget* pvui_create_resizer(GtkWidget *target_widget, int min_height, int max_height);


#endif /*PVP_PROMPT_VIEWER_UI_RESIZER_H*/
