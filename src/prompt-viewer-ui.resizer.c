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
#include <gtk/gtk.h>
#include "prompt-viewer-ui.resizer.h"

/** Retrieves the current height of a widget. */
static int get_widget_height(GtkWidget* widget){
    GtkAllocation allocation;
    gtk_widget_get_allocation(widget, &allocation);
    return allocation.height;
}

/** Clamps a value between a minimum and maximum limit. */
static inline int clamp_value(int value, int min, int max) {
    return value<min ? min : (value>max ? max : value);
}

/**
 * Internal structure holding the context for a drag operation on a grip.
 */
typedef struct DragContext {
    GtkWidget* target_widget;  ///< The widget whose height is adjusted during the drag.
    gboolean   dragging;       ///< Flag indicating whether a drag operation is currently active.
    int        min_height;     ///< The minimum allowed height for the target widget.
    int        max_height;     ///< The maximum allowed height for the target widget.
    double     y_root_start;   ///< The initial Y-coordinate (root window) where the drag started.
    int        height_start;   ///< The initial height of the target widget at the start of the drag.
} DragContext;


/*================================ EVENTS ================================*/

/**
 * Handles the mouse button press event on the grip.
 *
 * Initializes the drag operation by capturing the pointer, recording
 * the starting coordinate and the current height of the target widget.
 *
 * @param widget    The grip widget receiving the event.
 * @param event     The mouse button event details.
 * @param user_data Pointer to the internal `DragContext` structure.
 * @return TRUE if the event was handled, FALSE otherwise.
 */
static gboolean
on_button_press(GtkWidget*      widget,
                GdkEventButton* event,
                gpointer        user_data
){
    DragContext *ctx = (DragContext *)user_data;
    if (!ctx) { return FALSE; }
    if (event->type   != GDK_BUTTON_PRESS)   { return FALSE; }
    if (event->button != GDK_BUTTON_PRIMARY) { return FALSE; }

    // this block executes when the primary (left) mouse button is pressed;
    // we verify that the dragging flag is inactive for safety
    if (!ctx->dragging) {
        ctx->dragging     = TRUE;
        ctx->y_root_start = event->y_root;
        ctx->height_start = get_widget_height(ctx->target_widget);
        gtk_grab_add(widget); //< capture the pointer to continue receiving events
    }
    return TRUE;
}

/**
 * Handles the mouse button release event on the grip.
 *
 * Finalizes the drag operation by releasing the pointer capture
 * and resetting the dragging state.
 *
 * @param widget    The grip widget receiving the event.
 * @param event     The mouse button event details.
 * @param user_data Pointer to the internal `DragContext` structure.
 * @return TRUE if the event was handled, FALSE otherwise.
 */
static gboolean
on_button_release(GtkWidget*      widget,
                  GdkEventButton* event,
                  gpointer        user_data
){
    DragContext *ctx = (DragContext *)user_data;
    if (!ctx) { return FALSE; }
    if (event->type   != GDK_BUTTON_RELEASE) { return FALSE; }
    if (event->button != GDK_BUTTON_PRIMARY) { return FALSE; }
    
    // this block executes when the primary (left) mouse button is released;
    // we verify that the dragging flag is active for safety
    if (ctx->dragging) {
        ctx->dragging = FALSE;
        gtk_grab_remove(widget);
    }
    return TRUE;
}

/**
 * Handles mouse motion events during a drag operation.
 *
 * Calculates the vertical delta and updates the height of the
 * target widget, clamping it within defined minimum and maximum limits.
 *
 * @param widget    The grip widget receiving the event.
 * @param event     The mouse motion event details.
 * @param user_data Pointer to the DragContext structure.
 * @return TRUE if the event was handled, FALSE otherwise.
 */
static gboolean
on_motion_notify(GtkWidget*      widget,
                 GdkEventMotion* event,
                 gpointer        user_data
){
    DragContext *ctx = (DragContext *)user_data;
    if (!ctx) { return FALSE; }
    if (!ctx->dragging) { return FALSE; }
    
    // this block executes while the grip is being dragged;
    // we apply the updated size to the target widget
    const int delta_y    = (int)(event->y_root - ctx->y_root_start);
    const int new_height = clamp_value(ctx->height_start + delta_y, ctx->min_height, ctx->max_height);
    gtk_widget_set_size_request(ctx->target_widget, -1, new_height);
    return TRUE;
}

/**
 * Callback to free the drag context memory.
 *
 * This function is invoked when the associated grip widget is destroyed.
 * It safely frees the `DragContext` structure allocated for the grip,
 * preventing memory leaks.
 *
 * @param user_data Pointer to the `DragContext` structure to be freed.
 * @param closure   The closure associated with the signal handler (unused).
 */
static void
on_context_destroy(gpointer  user_data,
                   GClosure* closure
){
(void)closure;
    DragContext *ctx = (DragContext *)user_data;
    if (!ctx) { return; }
    g_free(ctx);
}


/*===========================================================================
|  ////////////////////////// PUBLIC FUNCTIONS ///////////////////////////  | 
===========================================================================*/

// Creates a draggable grip for vertically resizing the target widget.
GtkWidget* pvui_create_resizer(GtkWidget* target_widget,
                               int        min_height,
                               int        max_height
){

    // create a GtkEventBox and insert a separator as a 'handle'
    GtkWidget *event_box = gtk_event_box_new();
    GtkWidget *handle    = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_container_add(GTK_CONTAINER(event_box), handle);
    
    // set the GtkEventBox height to 8px for easy clicking
    gtk_widget_set_size_request(event_box, -1, 8);
    
    // create the drag context for this grip
    DragContext *ctx = g_new0(DragContext, 1);
    ctx->target_widget = target_widget;
    ctx->dragging      = FALSE;
    ctx->min_height    = min_height;
    ctx->max_height    = max_height;
    ctx->y_root_start  = 0.0;
    ctx->height_start  = 0;

    // connect mouse signals to the GtkEventBox
    g_signal_connect_data(event_box, "button-press-event"  , G_CALLBACK(on_button_press  ), ctx, (GClosureNotify)on_context_destroy, 0);
    g_signal_connect     (event_box, "button-release-event", G_CALLBACK(on_button_release), ctx);
    g_signal_connect     (event_box, "motion-notify-event" , G_CALLBACK(on_motion_notify ), ctx);

    return event_box;
}
