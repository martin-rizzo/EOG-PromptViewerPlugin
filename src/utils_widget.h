/**
 * @file    utils_widget.h
 * @brief   Utility functions for displaying text and numbers in GTK widgets.
 * @author  Martin Rizzo | <martinrizzo@gmail.com>
 * @date    Mar 25, 2023
 * @repo    https://github.com/martin-rizzo/EOG-PromptViewerPlugin
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
#include <gtk/gtk.h>

/**
 * get_widget:
 * @builder:     a #GtkBuilder object
 * @widget_name: the name of the widget to retrieve
 *
 * Retrieves a widget with the specified name from the given #GtkBuilder.
 *
 * Returns: (transfer none): a #GtkWidget pointer to the requested widget, or %NULL if not found.
 */
#define get_widget(builder, widget_name) \
    GTK_WIDGET( gtk_builder_get_object( builder, widget_name ) )

/**
 * ensure_valid_utf8:
 * @text:      a string that may contain invalid UTF-8 sequences
 * @max_bytes: the maximum number of bytes to process, or -1 to process until the NUL terminator
 *
 * Validates the given string and converts it to a valid UTF-8 format if necessary.
 * If the string is already valid UTF-8, it returns a duplicate of the original string.
 * If it is invalid, it attempts to convert it using the most appropriate encoding.
 *
 * Returns: (transfer full): a newly allocated string in valid UTF-8 format.
 *   The caller takes ownership of the returned string and is responsible for freeing it.
 */
static gchar *
ensure_valid_utf8(const char* text,
                  int         max_bytes
){
    gchar *utf8_text = NULL;
    if( !g_utf8_validate(text, max_bytes, NULL) ) {
        utf8_text = g_convert( text, max_bytes, "UTF-8", "ISO-8859-1",
                               NULL,NULL,NULL );
    }
    return utf8_text    ? utf8_text :
           max_bytes>=0 ? g_strndup( text, max_bytes ) : g_strdup( text );
}

/**
 * set_widget_text___:
 * @widget:    a #GtkWidget
 * @text:      a string of possibly invalid UTF-8 text
 * @max_bytes: the maximum number of bytes in @text, or -1
 * @depth:     the current recursion depth
 *
 * Internal recursive function that sets the text of a widget or its children.
 * This function is not intended to be called directly, but rather via `set_widget_text`.
 */
static void
set_widget_text__(GtkWidget*  widget,
                  const char* text,
                  int         max_bytes,
                  int         depth
){
    GtkTextBuffer *buffer; gchar *utf8_text;
    
    if( GTK_IS_LABEL(widget) && depth==0 ) {
        utf8_text = ensure_valid_utf8( text, max_bytes );
        gtk_label_set_text( GTK_LABEL(widget), utf8_text );
        g_free( utf8_text );
    }
    else if( GTK_IS_ENTRY(widget) ) {
        utf8_text = ensure_valid_utf8( text, max_bytes );
        gtk_entry_set_text( GTK_ENTRY(widget), utf8_text );
        g_free( utf8_text );
    }
    else if( GTK_IS_TEXT_VIEW(widget) ) {
        utf8_text = ensure_valid_utf8( text, max_bytes );
        buffer = gtk_text_view_get_buffer( GTK_TEXT_VIEW(widget) );
        if( buffer ) { gtk_text_buffer_set_text( buffer, utf8_text, -1 ); }
        g_free( utf8_text );
    }    
    else if( GTK_IS_CONTAINER(widget) ) {
        GList *children, *iter; GtkWidget *child;
        children = gtk_container_get_children( GTK_CONTAINER(widget) );
        for( iter = children ; iter ; iter = g_list_next(iter) ) {
            child = GTK_WIDGET( iter->data );
            set_widget_text__( child, text, max_bytes, depth+1 );
        }
    }
}

/**
 * set_widget_text:
 * @widget:    a #GtkWidget
 * @text:      a string of possibly invalid UTF-8 text to set as the widget's text
 * @max_bytes: the maximum number of bytes in @text, or -1 to process until the NUL terminator
 *
 * Sets the text of the specified widget to @text. If @max_bytes is -1, the
 * text is assumed to be NUL-terminated. If the provided text contains invalid
 * UTF-8 sequences, it will be automatically converted to a valid UTF-8 string
 * using the most appropriate method.
 *
 * This function handles various widget types (labels, entries, text views) and
 * recursively applies the text to child widgets if the widget is a container.
 */
static void
set_widget_text(GtkWidget*  widget,
                const char* text,
                int         max_bytes
){
    set_widget_text__( widget, text, max_bytes, 0 );
}

/*---------------------------- DISPLAYING TEXT ----------------------------*/

/**
 * display_text:
 * @builder:     A pointer to the #GtkBuilder object that contains the widget.
 * @widget_name: The name of the widget to set the text of.
 * @text:        A string to set as the widget's text.
 *
 * Sets the text of the specified widget to the provided @text. If @text is
 * %NULL, an empty string is used instead. If the text contains invalid UTF-8
 * sequences, it will be automatically converted to a valid UTF-8 string
 * using the most appropriate method.
 */
static void
display_text(GtkBuilder*  builder,
             const gchar* widget_name,
             const gchar* text
){
    GtkWidget *widget;
    widget = builder ? get_widget( builder, widget_name ) : NULL;
    if( widget ) {
        set_widget_text( widget, text ? text : "", -1 );
    }
}

/**
 * display_text_box:
 * @builder:     a #GtkBuilder object containing the widget
 * @widget_name: the name of the widget to update
 * @text:        the text to set, or %NULL to hide the widget
 *
 * Sets the text of the specified widget to @text. If @text is %NULL, the
 * widget is hidden; otherwise, it is shown. If the provided text contains
 * invalid UTF-8 sequences, they are automatically converted to a valid
 * UTF-8 string.
 * 
 * This function handles various widget types (labels, entries, text views) and
 * recursively applies the text to child widgets if the widget is a box or container.
 */
static void
display_text_box(GtkBuilder*  builder,
                 const gchar* widget_name,
                 const gchar* text
){
    GtkWidget *widget_box;
    widget_box = builder ? get_widget( builder, widget_name ) : NULL;
    if( widget_box ) {
        if( text ) { gtk_widget_show( widget_box ); }
        else       { gtk_widget_hide( widget_box ); }
        set_widget_text( widget_box, text ? text : "", -1 );
    }
}

/**
 * display_text_or_float:
 * @builder:      a #GtkBuilder object containing the widget
 * @widget_name:  the name of the widget to update
 * @text:         a string to set as the widget's text, or %NULL
 * @float_value:  the floating-point value to display if @text is %NULL
 * @num_decimals: the number of decimal places to display for the float value
 *
 * Sets the text of the specified widget to @text if provided, or formats
 * @float_value as a string otherwise. When @text is %NULL, @float_value is
 * converted to a string with the specified number of decimal places.
 * Invalid UTF-8 sequences in @text are automatically converted to valid
 * UTF-8.
 */
static void
display_text_or_float(GtkBuilder*  builder,
                      const gchar* widget_name,
                      const gchar* text,
                      float        float_value,
                      int          num_decimals
){
    if( text ) {
        display_text( builder, widget_name, text );
    } else {
        gchar *str_value = NULL;
        switch( num_decimals ) {
            case 0:  str_value = g_strdup_printf("%.0f", float_value); break;
            case 1:  str_value = g_strdup_printf("%.1f", float_value); break;
            case 2:  str_value = g_strdup_printf("%.2f", float_value); break;
            default: str_value = g_strdup_printf("%.3f", float_value); break;
        }
        if( str_value ) {
            display_text( builder, widget_name, str_value );
            g_free( str_value );
        }
    }
}
