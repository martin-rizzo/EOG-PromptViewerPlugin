/**
 * @file    sdprompt-viewer-plugin.c
 * @brief   Implementation of the main user interface for the plugin.
 * @author  Martin Rizzo | <martinrizzo@gmail.com>
 * @date    Mar 25, 2023
 * @repo    https://github.com/martin-rizzo/SDPromptViewer
 * @license MIT
 *//*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
                       Prompt Viewer Plugin for EOG
      A plugin for "Eye of GNOME" that displays SD embedded prompts.
  
     Copyright (c) 2023-2026 Martin Rizzo
  
     Permission is hereby granted, free of charge, to any person obtaining
     a copy of this software and associated documentation files (the
     "Software"), to deal in the Software without restriction, including
     without limitation the rights to use, copy, modify, merge, publish,
     distribute, sublicense, and/or sell copies of the Software, and to
     permit persons to whom the Software is furnished to do so, subject to
     the following conditions:
  
     The above copyright notice and this permission notice shall be
     included in all copies or substantial portions of the Software.
  
     THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
     EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
     MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
     IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
     CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
     TORT OR OTHERWISE, ARISING FROM,OUT OF OR IN CONNECTION WITH THE
     SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _
*/
#include <glib/gi18n-lib.h>
#include <gtk/gtk.h>
#include <gtk/gtkcssprovider.h>
#include <libpeas-gtk/peas-gtk-configurable.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#include <eog/eog-debug.h>
#include <eog/eog-thumb-view.h>
#include <eog/eog-sidebar.h>
#include <eog/eog-window.h>
#include <eog/eog-window-activatable.h>
#pragma GCC diagnostic pop

#include <promptex/promptex.h>
#include "prompt-viewer-ui.h"
#include "../resources.h"
#include "themes/themes.h"
#include "utils_widget.h"
#include "sdprompt-viewer-plugin.h"
#include "sdprompt-viewer-preferences.h"

#define UNKNOWN_SIZE (-1974)
#define IS_EMPTY_STR(str) ((str)==NULL || (str)[0]=='\0')
#define DEBUG_MESSAGE(...) eog_debug_message( DEBUG_PLUGINS, __VA_ARGS__ )

static void
eog_window_activatable_iface_init (EogWindowActivatableInterface *iface);
static void
sdprompt_viewer_plugin_dispose( GObject *object );
static void
sdprompt_viewer_plugin_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec);
static void
sdprompt_viewer_plugin_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec);
static const PeParams*
get_image_generation_params( SDPromptViewerPlugin *plugin );

enum {
    PROP_O,
    PROP_WINDOW,
    PROP_SHOW_UNKNOWN_PARAMS,
    PROP_FORCE_MINIMUM_WIDTH,
    PROP_MINIMUM_WIDTH,
    PROP_FORCE_VISIBILITY,
    PROP_THEME_VISUAL_STYLE,
    PROP_THEME_BORDER_SIZE,
    PROP_THEME_FONT_SIZE,
    NUMBER_OF_PROPS
};

/*
  G_DEFINE_DYNAMIC_TYPE_EXTENDED defines or references the following items:
    TYPES:
      * {TypeName}      : Structure defining instance properties.
      * {TypeName}Class : Structure defining class properties.
    VARS:
      * {type_name}_parent_class  : a pointer to the parent class.
    FUNCTIONS:
      * {type_name}_class_init    : Registers signals and properties specific to the class.
      * {type_name}_class_finalize: Frees resources allocated to the class.
      * {type_name}_init          : Initializes class-specific data fields.
      * {type_name}_get_type      : Returns the a GType that identifiers the class
*/
G_DEFINE_DYNAMIC_TYPE_EXTENDED(
    SDPromptViewerPlugin,      /* TypeName    */
    sdprompt_viewer_plugin,    /* type_name   */
    PEAS_TYPE_EXTENSION_BASE,  /* TYPE_PARENT */
    0,                         /* flags       */
    /* custom code that gets inserted in the *_register_type() function */
    G_IMPLEMENT_INTERFACE_DYNAMIC(
        EOG_TYPE_WINDOW_ACTIVATABLE, eog_window_activatable_iface_init)
)


/*----------------------  INITIALIZATION/DESTRUCTION ----------------------*/

static void
sdprompt_viewer_plugin_class_init( SDPromptViewerPluginClass *klass )
{
    GParamFlags flags;
    GObjectClass *object_class = G_OBJECT_CLASS( klass );
    object_class->dispose      = sdprompt_viewer_plugin_dispose;

    /* register properties */
    object_class->set_property = sdprompt_viewer_plugin_set_property;
    object_class->get_property = sdprompt_viewer_plugin_get_property;
    g_object_class_override_property( object_class, PROP_WINDOW, "window" );
    
    flags = ( G_PARAM_READWRITE | G_PARAM_STATIC_NAME );
    
    g_object_class_install_property(
        object_class, PROP_SHOW_UNKNOWN_PARAMS,
        g_param_spec_boolean("show-unknown-params",0,0, FALSE, flags) );
    
    g_object_class_install_property(
        object_class, PROP_FORCE_MINIMUM_WIDTH,
        g_param_spec_boolean("force-minimum-width",0,0, FALSE, flags) );
    
    g_object_class_install_property(
        object_class, PROP_MINIMUM_WIDTH,
        g_param_spec_double("minimum-width",0,0, 100,1000, 480, flags) );
                                      
    g_object_class_install_property(
        object_class, PROP_FORCE_VISIBILITY,
        g_param_spec_boolean("force-visibility",0,0, FALSE, flags) );
    
    g_object_class_install_property(
        object_class, PROP_THEME_VISUAL_STYLE,
        g_param_spec_int("visual-style",0,0, 0,9, 0, flags) );
    
    g_object_class_install_property(
        object_class, PROP_THEME_BORDER_SIZE,
        g_param_spec_int("border-size",0,0, 0,9, 0, flags) );
    
    g_object_class_install_property(
        object_class, PROP_THEME_FONT_SIZE,
        g_param_spec_int("font-size",0,0, -2,2, 0, flags) );
    
    klass->sidebar_min_width = UNKNOWN_SIZE;
    klass->sidebar_original_min_width  = UNKNOWN_SIZE;
    klass->sidebar_original_min_height = UNKNOWN_SIZE;
}

static void
sdprompt_viewer_plugin_class_finalize( SDPromptViewerPluginClass *klass )
{
    /* needed for G_DEFINE_DYNAMIC_TYPE_EXTENDED */
}

static void
sdprompt_viewer_plugin_init( SDPromptViewerPlugin *plugin )
{
    DEBUG_MESSAGE( "SDPromptViewerPlugin initializing" );
}

static void
sdprompt_viewer_plugin_dispose( GObject *object )
{
    SDPromptViewerPlugin *plugin = SDPROMPT_VIEWER_PLUGIN( object );
    DEBUG_MESSAGE( "SDPromptViewerPlugin disposing" );
    
    if( plugin->window ) {
        g_object_unref( plugin->window );
        plugin->window = NULL;
    }

    G_OBJECT_CLASS( sdprompt_viewer_plugin_parent_class )->dispose( object );
}

/*------------------------------ OPERATIONS -------------------------------*/

/**
 * apply_sidebar_minimum_width - Applies minimum width of the sidebar.
 * @plugin    : A pointer to an #SDPromptViewerPlugin object
 * @min_width : The minimum width of the sidebar
 *
 * Applies the minimum width that the sidebar of this plugin can have.
 * If 'min_width == -1', the minimum width of the sidebar is reset
 * to the default value used by Eye of GNOME.
 */ 
static void
apply_sidebar_minimum_width( SDPromptViewerPlugin *plugin,
                             gint                  min_width )
{
    SDPromptViewerPluginClass *klass = SDPROMPT_VIEWER_PLUGIN_GET_CLASS( plugin );
    g_assert_nonnull( klass );
    
    /* ensure that the original minimum size is stored */
    if( klass->sidebar_original_min_width == UNKNOWN_SIZE ) {
        gtk_widget_get_size_request( GTK_WIDGET( plugin->sidebar ),
                                     &klass->sidebar_original_min_width,
                                     &klass->sidebar_original_min_height );
    }
    /* set the minimum size */
    if( min_width >= 0 ) {
        gtk_widget_set_size_request( GTK_WIDGET( plugin->sidebar ),
                                     min_width,
                                     klass->sidebar_original_min_height );
    }
    /* restore original minimum size */
    else if( klass->sidebar_original_min_width != UNKNOWN_SIZE ) {
        gtk_widget_set_size_request( GTK_WIDGET( plugin->sidebar ),
                                     klass->sidebar_original_min_width,
                                     klass->sidebar_original_min_height );
    }
}

/**
 * apply_visual_style - Applies a predefined visual style to the sidebar.
 * @plugin       : A pointer to an #SDPromptViewerPlugin object
 * @visual_style : An integer representing the visual style to apply
 *
 * Applies the specified visual style to all plugin's widgets by updating
 * their CSS. The @visual_style parameter should be an integer corresponding
 * to one of the predefined styles. If @visual_style is -1, any previously
 * applied style is removed and associated resources are freed.
 */ 
static void
apply_visual_style( SDPromptViewerPlugin *plugin,
                    SDPromptTheme         theme )
{
    SDPromptViewerPluginClass *klass  = SDPROMPT_VIEWER_PLUGIN_GET_CLASS( plugin );
    GdkScreen                 *screen = gdk_screen_get_default();
    if( !screen ) { return; }
    
    /* if the theme system was locked */
    /* then do nothing and return     */
    if( klass->is_theme_locked )
    { return; }
    
    /* if the input theme is the same than the current theme */
    /* then do nothing and return                            */
    if( theme.visual_style == klass->current_theme.visual_style &&
        theme.border_size  == klass->current_theme.border_size  &&
        theme.font_size    == klass->current_theme.font_size     )
    { return; }

    /*
    DEBUG_MESSAGE( "## visual-style = %d", theme.visual_style );
    DEBUG_MESSAGE( "## border-size  = %d", theme.border_size  );
    DEBUG_MESSAGE( "## font-size    = %d", theme.font_size    );
    */

    /* remove previous visual styles */
    if( klass->visual_style_provider ) {
        gtk_style_context_remove_provider_for_screen(
            screen, klass->visual_style_provider);
        g_object_unref( klass->visual_style_provider );
        klass->visual_style_provider = NULL;
    }
    if( klass->border_style_provider ) {
        gtk_style_context_remove_provider_for_screen(
            screen, klass->border_style_provider);
        g_object_unref( klass->border_style_provider );
        klass->border_style_provider = NULL;
    }
    if( klass->zoom_style_provider ) {
        gtk_style_context_remove_provider_for_screen(
            screen, klass->zoom_style_provider);
        g_object_unref( klass->zoom_style_provider );
        klass->zoom_style_provider = NULL;        
    }
    /* try to create the new style providers */
    klass->visual_style_provider = new_theme_style_provider(
                                        THEME_VISUAL_STYLE, theme.visual_style );
    if( klass->visual_style_provider==NULL ) { return; }
    klass->border_style_provider = new_theme_style_provider(
                                        THEME_BORDER_STYLE, theme.border_size );
    klass->zoom_style_provider   = new_theme_style_provider(
                                        THEME_ZOOM_STYLE, theme.font_size );
    
    /* add the new visual styles */
    if( klass->visual_style_provider ) {
        gtk_style_context_add_provider_for_screen(
            screen, klass->visual_style_provider,
            GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
        );
    }
    if( klass->border_style_provider ) {
        gtk_style_context_add_provider_for_screen(
            screen, klass->border_style_provider,
            GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
        );
    }
    if( klass->zoom_style_provider ) {
        gtk_style_context_add_provider_for_screen(
            screen, klass->zoom_style_provider,
            GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
        );
    }
}

/*-------------------- CONTROLLING THE USER INTERFACE ---------------------*/

#if 0
static void
hide_all_widgets( GtkBuilder  *builder )
{
    GtkWidget *main_container;
    GList *children, *iter; GtkWidget *child;
    
    main_container = builder ? get_widget( builder, "main_container" ) : NULL;
    if( main_container ) {
        children = gtk_container_get_children( GTK_CONTAINER(main_container) );
        for( iter = children ; iter ; iter = g_list_next(iter) ) {
            child = GTK_WIDGET( iter->data );
            gtk_widget_hide( child );
        }
    }
}

static void
show_image_generation_params(SDPromptViewerPlugin *plugin
){
    GtkBuilder *b = plugin->page_builder;
    if( !b ) { return; }

    /* If no generation data is present, show a message and return */
    const PeParams* params = get_image_generation_params( plugin );
    if( !params )
    {
        show_message( plugin,
                      "No generation parameters found in the image." );
        return;
    }

    hide_all_widgets( b );
    if (params->prompt.has_value) {
        display_text(b, "prompt_text_view", params->prompt.value );
    }


    show_widget(b, "prompt_group", params->prompt.has_value);
    show_widget(b, "buttons_group"   , TRUE                             );
    show_widget(b, "parameters_group", TRUE                             );



#if 0
    SDParameters parameters; const gchar *image_generation_data;
    GtkTextView *text_view; GtkTextBuffer *buffer; int i;
    GtkBuilder *b = plugin->page_builder;
    if( !b ) { return; }
        
    /* If no generation data is present, show a message and return */
    image_generation_data = get_image_generation_data( plugin );
    if( IS_EMPTY_STR(image_generation_data) )
    {
        show_message( plugin,
                      "No Stable Diffusion parameters found in the image." );
        return;
    }
    
    parse_sd_parameters_from_buffer( &parameters,
                                     image_generation_data, -1 );
    
    hide_all_widgets( b );
    display_text(b, "prompt_text_view"       , parameters.prompt            );
    display_text(b, "negative_text_view"     , parameters.negative_prompt   );
    display_text(b, "wildcard_text_view"     , parameters.wildcard_prompt   );
    display_text(b, "model_entry"            , parameters.model.name        );
    display_text(b, "model_hash_entry"       , parameters.model.hash        );
    display_text(b, "sampler_entry"          , parameters.sampler           );
    display_text(b, "steps_entry"            , parameters.steps             );
    display_text(b, "cfg_scale_entry"        , parameters.cfg_scale         );
    display_text(b, "seed_entry"             , parameters.seed              );
    display_text(b, "width_entry"            , parameters.width             );
    display_text(b, "height_entry"           , parameters.height            );
    display_text(b, "hires_upscaler_entry"   , parameters.hires.upscaler    );
    display_text(b, "hires_steps_entry"      , parameters.hires.steps       );
    display_text(b, "hires_denoising_entry"  , parameters.hires.denoising   );    
    display_text(b, "inpaint_denoising_entry", parameters.inpaint.denoising );
    display_text(b, "inpaint_mask_blur_entry", parameters.inpaint.mask_blur );
    
    display_text_box(b, "eta_box"      , parameters.settings.eta       );
    display_text_box(b, "ensd_box"     , parameters.settings.ensd      );
    display_text_box(b, "clip_skip_box", parameters.settings.clip_skip );
    
    display_text_or_float(b, "hires_width_entry",
                      parameters.hires.width,
                      parameters.hires.calc_width,0);
    display_text_or_float(b, "hires_height_entry",
                      parameters.hires.height,
                      parameters.hires.calc_height,0);
    display_text_or_float(b, "hires_upscale_entry",
                      parameters.hires.upscale,
                      parameters.hires.calc_upscale,2);
    
    text_view = GTK_TEXT_VIEW( get_widget(b, "unknown_text_view") );
    buffer    = text_view ? gtk_text_view_get_buffer( text_view ) : NULL;
    if( buffer ) {
        show_widget( b, "unknown_group", parameters.unknowns_count > 0 );
        gtk_text_buffer_set_text( buffer, "", -1 );
        for( i=0; i<parameters.unknowns_count; ++i ) {
            const char *key   = parameters.unknowns[i].key;
            const char *value = parameters.unknowns[i].value;
            gtk_text_buffer_insert_at_cursor( buffer,  key  , -1 );
            gtk_text_buffer_insert_at_cursor( buffer,  ": " , -1 );
            gtk_text_buffer_insert_at_cursor( buffer, value , -1 );
            gtk_text_buffer_insert_at_cursor( buffer, "\n"  , -1 );
        }        
    }
    
    show_widget(b, "buttons_group"   , TRUE                             );
    show_widget(b, "prompt_group"    , parameters.prompt!=NULL          );
    show_widget(b, "negative_group"  , parameters.negative_prompt!=NULL ); 
    show_widget(b, "wildcard_group"  , parameters.wildcard_prompt!=NULL );
    show_widget(b, "parameters_group", TRUE                             );
    show_widget(b, "model_group"     , parameters.model.has_info        ); 
    show_widget(b, "hires_group"     , parameters.hires.has_info        );
    show_widget(b, "inpaint_group"   , parameters.inpaint.has_info      );
    show_widget(b, "settings_group"  , parameters.settings.has_info     );
    
#endif 

    if( plugin->force_visibility ) {
        if( plugin->sidebar ) {
            eog_sidebar_set_page( plugin->sidebar, plugin->page );
        }
    }
}

static void
show_message( SDPromptViewerPlugin *plugin,
              const gchar          *message )
{
    GtkBuilder *builder = plugin->page_builder;
    if( !builder ) { return; }
    pvpui_hide_all_widgets(&plugin->ui);
    //display_text( builder , "message_label", message );
    //show_widget( builder  , "message_group", TRUE    );
}

#endif

static void
show_widget( GtkBuilder  *builder, 
             const gchar *widget_name,
             gboolean     show )
{
    GtkWidget *widget;
    widget = builder ? get_widget( builder, widget_name ) : NULL;
    if( widget ) {
        if( show ) { gtk_widget_show( widget ); }
        else       { gtk_widget_hide( widget ); }
    }
}

static void
show_spinner( SDPromptViewerPlugin *plugin )
{
    GtkBuilder *builder = plugin->page_builder;
    if( !builder ) { return; }

    pvpui_hide_all_widgets(&plugin->ui);
    //hide_all_widgets( builder );
    //show_widget( builder, "loading_group", TRUE );
}



/*------------------------------ PROPERTIES -------------------------------*/


/**
 * set_image_generation_params:
 * @plugin: A pointer to an #SDPromptViewerPlugin instance.
 * @params: A pointer to the #PeParams structure containing the image generation parameters, or %NULL.
 *
 * Sets the image generation parameters for the given plugin.
 *
 * This function updates the plugin's internal state by storing a reference
 * to the provided #PeParams structure. If a previous set of parameters
 * was already assigned, it will be properly destroyed and freed before
 * the new parameters are set.
 *
 * If @params is %NULL, the function clears the plugin's current parameters.
 */
static void
set_image_generation_params(SDPromptViewerPlugin* plugin,
                            const PeParams*       params
){
    pe_destroy_params( plugin->image_generation_params );
    plugin->image_generation_params = NULL;
    if( params ) {
        plugin->image_generation_params = params;
    }
}

static const PeParams*
get_image_generation_params(SDPromptViewerPlugin* plugin
){
    return plugin->image_generation_params;
}



/**
 * sdprompt_viewer_plugin_set_property:
 * @object:  A #GObject
 * @prop_id: The ID of the property to set
 * @value:   A #GValue containing the new value for the property
 * @pspec:   A #GParamSpec describing the property to set
 *
 * Sets the value of a property for the plugin.
 *
 * This function is a standard setter function used in GObject and GTK.
 * It is called by the framework whenever a property value needs to be set
 * for the plugin.
 */
static void
sdprompt_viewer_plugin_set_property( GObject       *object,
                                     guint          prop_id,
                                     const GValue  *value,
                                     GParamSpec    *pspec )
{
    SDPromptViewerPlugin      *plugin = SDPROMPT_VIEWER_PLUGIN (object);
    switch (prop_id)
    {
        case PROP_WINDOW:
            plugin->window = EOG_WINDOW( g_value_dup_object(value) );
            break;
            
        case PROP_SHOW_UNKNOWN_PARAMS:
            plugin->show_unknown_params = g_value_get_boolean(value);
            break;
            
        case PROP_FORCE_MINIMUM_WIDTH:
            plugin->force_minimum_width = g_value_get_boolean(value);
            apply_sidebar_minimum_width( plugin,
                plugin->force_minimum_width ? (gint)plugin->minimum_width : -1 );
            break;
            
        case PROP_MINIMUM_WIDTH:
            plugin->minimum_width = g_value_get_double(value);
            apply_sidebar_minimum_width( plugin,
                plugin->force_minimum_width ? (gint)plugin->minimum_width : -1 );
            break;
            
        case PROP_FORCE_VISIBILITY:
            plugin->force_visibility = g_value_get_boolean(value);
            break;
            
        case PROP_THEME_VISUAL_STYLE:
            plugin->theme.visual_style = g_value_get_int(value);
            apply_visual_style( plugin, plugin->theme );
            break;

        case PROP_THEME_BORDER_SIZE:
            plugin->theme.border_size = g_value_get_int(value);
            apply_visual_style( plugin, plugin->theme );
            break;
            
        case PROP_THEME_FONT_SIZE:
            plugin->theme.font_size = g_value_get_int(value);
            apply_visual_style( plugin, plugin->theme );
            break;
                        
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}

/**
 * sdprompt_viewer_plugin_get_property:
 * @object  : A #GObject
 * @prop_id : The ID of the property to get
 * @value   : A #GValue to store the property value in
 * @pspec   : A #GParamSpec describing the property to get
 *
 * Retrieves the value of a property for the plugin.
 *
 * This function is a standard getter function used in GObject and GTK.
 * It is called by the framework whenever a property value needs to be
 * retrieved for the plugin.
 */
static void
sdprompt_viewer_plugin_get_property( GObject    *object,
                                     guint       prop_id,
                                     GValue     *value,
                                     GParamSpec *pspec )
{
    SDPromptViewerPlugin *plugin = SDPROMPT_VIEWER_PLUGIN( object );
    switch (prop_id)
    {
        case PROP_WINDOW:
            g_value_set_object(value, plugin->window);
            break;

        case PROP_SHOW_UNKNOWN_PARAMS:
            g_value_set_boolean(value, plugin->show_unknown_params);
            break;
            
        case PROP_FORCE_MINIMUM_WIDTH:
            g_value_set_boolean(value, plugin->force_minimum_width);
            break;
            
        case PROP_MINIMUM_WIDTH:
            g_value_set_double(value, plugin->minimum_width);
            break;
            
        case PROP_FORCE_VISIBILITY:
            g_value_set_boolean(value, plugin->force_visibility);
            break;
            
        case PROP_THEME_VISUAL_STYLE:
            g_value_set_int(value, plugin->theme.visual_style);
            break;
            
        case PROP_THEME_BORDER_SIZE:
            g_value_set_int(value, plugin->theme.border_size);
            break;
            
        case PROP_THEME_FONT_SIZE:
            g_value_set_int(value, plugin->theme.font_size);
            break;
            
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}

/*-------------------------------- EVENTS ---------------------------------*/

/**
 * on_report:
 * @type:    A #PEREPORT_TYPE indicating the type of report (e.g., warning, error, etc..).
 * @message: A #PeUTF8 string containing the report message.
 *
 * Callback invoked by the PromptExtractor library to report events such as
 * messages or errors found during extraction. This callback is primarily
 * designed for CLI applications, printing the messages to the standard
 * error stream.
 */
static void
on_report(PEREPORT_TYPE type, const PeUTF8* message) {
    switch( type ) {
        case PEREPORT_WARNING:
            fprintf(stderr, "WARNING: %s\n", message);
            break;
        case PEREPORT_ERROR:
            fprintf(stderr, "ERROR: %s\n", message);
            break;
        default:
            fprintf(stderr, "%s\n", message);
            break;
    }
}

/**
 * on_generation_params_loaded:
 * @params:   A pointer to the extracted generation parameters.
 * @user_ptr: The plugin instance to update.
 * @user_int: Additional user data (unused).
 *
 * Callback invoked when embedded generation params are loaded from an image.
 * It stores the parameters in the plugin and updates the user interface.
 */
static void
on_generation_params_loaded(const PeParams* params, gpointer user_ptr, int user_int) {
    SDPromptViewerPlugin *plugin = SDPROMPT_VIEWER_PLUGIN( user_ptr );
    set_image_generation_params( plugin, params );
    pvpui_show_params(&plugin->ui, plugin->image_generation_params);

    if( plugin->force_visibility ) {
        if( plugin->sidebar ) {
            eog_sidebar_set_page( plugin->sidebar, plugin->page );
        }
    }
}

/**
 * on_image_changed:
 * @view:   A pointer to the #EogThumbView where the selection changed.
 * @plugin: A pointer to the #SDPromptViewerPlugin instance.
 *
 * Callback invoked by Eye of GNOME (EOG) when the selected image in the
 * thumbnail view changes. It retrieves the first selected image, extracts
 * its file path, and loads the corresponding generation parameters using
 * the PromptExtractor library. If no image is selected, it displays a
 * "No image selected." message in the UI.
 */
static void
on_image_changed(EogThumbView*         view,
                 SDPromptViewerPlugin* plugin
){
    
    if( eog_thumb_view_get_n_selected( view ) == 0 ) {
        pvpui_show_message(&plugin->ui, "No image selected.");
        return;
    }
    
    EogImage* image = eog_thumb_view_get_first_selected_image( view );
    GFile*    gfile = image ? eog_image_get_file( image ) : NULL;
    if( gfile!=NULL ) {
        g_autofree gchar *path = g_file_get_path(gfile);
        const PeParams* params = path ? pe_params_from_path(path, on_report) : NULL;
        on_generation_params_loaded(params, plugin, 0);
    }
    if( gfile ) { g_object_unref(gfile); }
    if( image ) { g_object_unref(image); }
}

static void
on_copy_data_clicked( GtkWidget *widget, gpointer data ) {
    SDPromptViewerPlugin *plugin = SDPROMPT_VIEWER_PLUGIN( data );
    (void)plugin;
    /*
    gtk_clipboard_set_text(
        gtk_clipboard_get( GDK_SELECTION_CLIPBOARD ),
        get_image_generation_data( plugin ), -1 );
    */
}

static void
on_preferences_clicked( GtkWidget *widget, gpointer data ) {
    
    SDPromptViewerPreferences *preferences =
        g_object_new( TYPE_SDPROMPT_VIEWER_PREFERENCES, NULL);
        
    GtkWidget *preferences_widget =
      peas_gtk_configurable_create_configure_widget( PEAS_GTK_CONFIGURABLE( preferences ) );

    /* create a Gtk dialog that contains the plugin's configuration widget */
    GtkWidget *dialog = gtk_dialog_new_with_buttons("Prompt Viewer Plugin for EOG",
                                                     NULL,
                                                     GTK_DIALOG_MODAL,
                                                     "Close",
                                                     GTK_RESPONSE_CLOSE,
                                                     NULL);
    gtk_container_add(GTK_CONTAINER(gtk_dialog_get_content_area(GTK_DIALOG(dialog))), preferences_widget);
    gtk_widget_show_all(dialog);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
    g_object_unref( preferences_widget );
    g_object_unref( preferences );
}

static void
on_activate( EogWindowActivatable *activatable )
{
    SDPromptViewerPlugin      *plugin   = SDPROMPT_VIEWER_PLUGIN( activatable );
    SDPromptViewerPluginClass *klass    = SDPROMPT_VIEWER_PLUGIN_GET_CLASS( plugin );
    EogWindow                 *window   = plugin ? plugin->window :  NULL;
    GSettings                 *settings = g_settings_new( SDPROMPT_VIEWER_GSCHEMA_ID );
    GError                    *error    = NULL;
    
    g_assert_nonnull( window );
    g_assert_nonnull( settings );
    
    klass->instance_count++;

    /*-- build the user interface --*/
    plugin->page_builder = gtk_builder_new();
    gtk_builder_set_translation_domain( plugin->page_builder,
                                        GETTEXT_PACKAGE );
    if( !gtk_builder_add_from_resource( plugin->page_builder,
                                        UI_RESOURCE_DIR"/sdprompt-viewer-plugin.ui",
                                        &error) ) {
        g_warning( "Couldn't load UI resource: %s", error->message );
        g_error_free( error );
    }
    
    plugin->thumbview = EOG_THUMB_VIEW( eog_window_get_thumb_view( window ) );
    plugin->sidebar   = EOG_SIDEBAR( eog_window_get_sidebar( window ) );

    plugin->ui   = pvp_create_prompt_viewer_ui();
    plugin->page = plugin->ui.box;

    //plugin->page = GTK_WIDGET(plugin->ui_prompt.box);

    /*-- add the user interface to the sidebar */
    eog_sidebar_add_page( plugin->sidebar, _("Prompt Viewer Plugin"), plugin->page );
    gtk_widget_show_all( plugin->page );

#if 0
    plugin->page      = get_widget( plugin->page_builder, "viewport1" );


    /*-- add the user interface to the sidebar */
    eog_sidebar_add_page( plugin->sidebar,
                          _("Prompt Viewer Plugin"),
                          plugin->page );
    gtk_widget_show_all( plugin->page );

    /*-- binding configurable properties --*/
    g_settings_bind( settings, SETTINGS_SHOW_UNKNOWN_PARAMS,
                     plugin, "show-unknown-params", G_SETTINGS_BIND_GET);
    g_settings_bind( settings, SETTINGS_FORCE_MINIMUM_WIDTH,
                     plugin, "force-minimum-width", G_SETTINGS_BIND_GET);
    g_settings_bind( settings, SETTINGS_MINIMUM_WIDTH,
                     plugin, "minimum-width", G_SETTINGS_BIND_GET);
    g_settings_bind( settings, SETTINGS_FORCE_VISIBILITY,
                     plugin, "force-visibility", G_SETTINGS_BIND_GET);
    g_settings_bind( settings, SETTINGS_VISUAL_STYLE,
                     plugin, "visual-style", G_SETTINGS_BIND_GET);
    g_settings_bind( settings, SETTINGS_BORDER_SIZE,
                     plugin, "border-size", G_SETTINGS_BIND_GET);
    g_settings_bind( settings, SETTINGS_FONT_SIZE,
                     plugin, "font-size", G_SETTINGS_BIND_GET);
    
#endif

    /*-- binding events using signals --*/
    plugin->thumbview_sel_changed_signal_id =
        g_signal_connect( G_OBJECT( plugin->thumbview ),
                          "selection-changed",
                          G_CALLBACK( on_image_changed ),
                          plugin );

    // plugin->preferences_button_signal_id =
    //     g_signal_connect( get_widget( plugin->page_builder, "preferences_button" ),
    //                       "clicked",
    //                       G_CALLBACK( on_preferences_clicked ),
    //                       plugin );
        
    // plugin->copy_button_signal_id =
    //     g_signal_connect( get_widget( plugin->page_builder, "copy_button" ),
    //                       "clicked",
    //                       G_CALLBACK( on_copy_data_clicked ),
    //                       plugin );


    /*-- force update image information for first time --*/
    on_image_changed(plugin->thumbview, plugin);

    /*-- clean up --*/
    if( settings ) { g_object_unref( settings ); }
    
}

static void
on_deactivate( EogWindowActivatable *activatable )
{
    SDPromptViewerPlugin      *plugin = SDPROMPT_VIEWER_PLUGIN( activatable );
    SDPromptViewerPluginClass *klass  = SDPROMPT_VIEWER_PLUGIN_GET_CLASS( plugin );
    static const SDPromptTheme NULL_THEME = { -1, -1, -1 };

    /*-- restore sidebar width and release image generation data --*/
    set_image_generation_params( plugin, NULL );
    apply_sidebar_minimum_width( plugin, -1 );

    /*-- remove the user interface from the sidebar --*/
    eog_sidebar_remove_page( plugin->sidebar,
                             plugin->page );

    /*-- remove signals --*/
    g_signal_handler_disconnect( plugin->thumbview,
                                 plugin->thumbview_sel_changed_signal_id );
    g_signal_handler_disconnect( get_widget( plugin->page_builder, "preferences_button" ),
                                 plugin->preferences_button_signal_id );
    g_signal_handler_disconnect( get_widget( plugin->page_builder, "copy_button" ),
                                 plugin->copy_button_signal_id );
    
    if( plugin->page_builder ) {
        g_object_unref( plugin->page_builder );
        plugin->page_builder = NULL;
    }

    /* if the current object is the last instance of the class        */
    /* then it removes any visual styles applied to free up resources */
    if( --klass->instance_count == 0 ) {
        apply_visual_style( plugin, NULL_THEME );
    }
    
    /* This line locks the visual styles system due to a bug that crashes */
    /* the application if any window is closed and a style is applied.    */
    /* The bug has not been fixed yet, so the visual styles system is     */
    /* locked immediately after a window is closed to prevent the crash.  */
    klass->is_theme_locked = TRUE;

}

static void
eog_window_activatable_iface_init(EogWindowActivatableInterface *iface)
{
    iface->activate   = on_activate;
    iface->deactivate = on_deactivate;
}

/*========================= PLUGIN MAIN FUNCTION ==========================*/

G_MODULE_EXPORT void
peas_register_types(PeasObjectModule *module)
{
    sdprompt_viewer_plugin_register_type( G_TYPE_MODULE(module) );
    peas_object_module_register_extension_type(
        module,
        EOG_TYPE_WINDOW_ACTIVATABLE, /* <- the interface to implement */
        TYPE_SDPROMPT_VIEWER_PLUGIN  /* <- my object                  */
    );

    /* register the "Preferences" object */
    sdprompt_viewer_preferences_register_types( module );
}

