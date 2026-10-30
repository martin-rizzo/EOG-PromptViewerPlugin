#!/bin/bash
# File    : plugin.sh
# Brief   : Compiles, installs, and uninstalls the Prompt Viewer Plugin for EOG
# Author  : Martin Rizzo | <martinrizzo@gmail.com>
# Date    : Oct 29, 2026
# Repo    : https://github.com/martin-rizzo/EOG-PromptViewerPlugin
# License : MIT
#- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#                       Prompt Viewer Plugin for EOG
# An "Eye of GNOME" plugin to display prompts embedded in AI-generated images.
#- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#
#     Copyright (c) 2023-2026 Martin Rizzo
#
#     Permission is hereby granted, free of charge, to any person obtaining
#     a copy of this software and associated documentation files (the
#     "Software"), to deal in the Software without restriction, including
#     without limitation the rights to use, copy, modify, merge, publish,
#     distribute, sublicense, and/or sell copies of the Software, and to
#     permit persons to whom the Software is furnished to do so, subject to
#     the following conditions:
#
#     The above copyright notice and this permission notice shall be
#     included in all copies or substantial portions of the Software.
#
#     THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
#     EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
#     MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
#     IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
#     CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
#     TORT OR OTHERWISE, ARISING FROM,OUT OF OR IN CONNECTION WITH THE
#     SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#
#_ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _
SCRIPT_NAME=${BASH_SOURCE[0]##*/}
SCRIPT_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")" &>/dev/null && pwd)
SCRIPT_HELP="
Usage: ./$SCRIPT_NAME [OPTIONS]

This script compiles, installs, and uninstalls the Prompt Viewer Plugin for EOG.

Available options:
     install       Installs the plugin into Eye of GNOME.
     remove        Uninstalls the plugin from Eye of GNOME.
     install-debug ???
     status        ???

Example:
  ./$SCRIPT_NAME install   # Installs the plugin.
"

# The three files required for the plugin to function correctly
# -------------------------------------------------------------

# The compiled shared object (binary) containing the plugin logic
PROMPTVIEWER_SO="libprompt-viewer.so"

# The plugin descriptor file containing metadata and configuration details
PROMPTVIEWER_PLUGIN="libprompt-viewer.plugin"

# The GSettings schema definition file for plugin settings
PROMPTVIEWER_GSCHEMA="org.gnome.eog.plugins.prompt-viewer.gschema.xml"

# The different directories used in the code of this script
# ---------------------------------------------------------

# The base directory for user-specific data files,
# as defined by the 'XDG Base Directory Specification'.
# Defaults to $HOME/.local/share if the environment variable is not set.
[ -z "$XDG_DATA_HOME" ] && XDG_DATA_HOME="$HOME/.local/share"

# Directory where Eye of GNOME (EOG) stores its plugins.
EOG_PLUGINS_DIR="$XDG_DATA_HOME/eog/plugins"

# Directory where the GSettings schemas are stored
GIO_SCHEMAS_DIR="$XDG_DATA_HOME/glib-2.0/schemas"

# The root directory of the project.
PROJECT_ROOT_DIR="$SCRIPT_DIR"

# The project's `./bin` directory where all build outputs are generated.
PROJECT_BIN_DIR="$PROJECT_ROOT_DIR/bin"

# IMPORTANT:
#   Directory where test images are located. These images are used
#   when running './plugin.sh run' to test the plugin functionality.
TEST_IMAGES_DIR="$HOME/Extra/Test"

# Initialize script status to 0 (success)
SCRIPT_STATUS=0

#--------------------------- TERMINAL ANSI-COLORS --------------------------#

# Colores Normales
black='\033[0;30m'
red='\033[0;31m'
green='\033[0;32m'
yellow='\033[0;33m'
blue='\033[0;34m'
purple='\033[0;35m'
cyan='\033[0;36m'
white='\033[0;37m'

# Colores en Negrita (Bold)
BLACK='\033[1;30m'
RED='\033[1;31m'
GREEN='\033[1;32m'
YELLOW='\033[1;33m'
BLUE='\033[1;34m'
PURPLE='\033[1;35m'
CYAN='\033[1;36m'
WHITE='\033[1;37m'

NC='\033[0m' # No Color


#------------------------------ PRINT STATUS -------------------------------#

# Prints the status of the compiled shared object (binary) file.
# Globals:
#   PROMPTVIEWER_SO: Name of the shared object file.
#   EOG_PLUGINS_DIR: Directory where the plugin is installed.
#   PROJECT_BIN_DIR: Directory containing the project's build outputs.
# Outputs:
#   Prints status information to stdout with ANSI color codes.
print_status_so() {
  local so_file so_version so_debugpath
  so_file="$EOG_PLUGINS_DIR/$PROMPTVIEWER_SO"
  so_version=$(strings "${so_file}" 2>/dev/null | grep "@(#) PVP_VERSION" | awk '{print $NF}')
  so_debugpath="$PROJECT_BIN_DIR/debug/$PROMPTVIEWER_SO"

  echo -ne " - ${CYAN}📦Binary ${PURPLE}(${WHITE}${PROMPTVIEWER_SO}${PURPLE})${NC}          ${BLUE}➔${NC} "

  if [ ! -e "$so_file" ]; then
    echo -e "${RED}✖ Not installed${NC}"
  elif [ -L "$so_file" ]; then
    local link_target short_target
    link_target=$(readlink "$so_file")
    short_target=${link_target/#$HOME/\~}
    if [ "$link_target" -ef "$so_debugpath" ]; then
      echo -e "${YELLOW}🪲 Installed in DEBUG mode ${CYAN}(v${so_version})${NC}"
    else
      echo -e "${RED}🚨 softlink to: ${CYAN}${short_target}${NC}"
    fi
  elif [ -f "$so_file" ]; then
    echo -e "${GREEN}✔ Installed ${CYAN}(v${so_version})${NC}"
  else
    echo -e "${RED}⚠ Error: Not a regular file or symlink${NC}"
  fi
}

# Prints the status of the plugin descriptor file.
# Globals:
#   PROMPTVIEWER_PLUGIN: Name of the plugin descriptor file.
#   EOG_PLUGINS_DIR:     Directory where the plugin is installed.
#   PROJECT_BIN_DIR:     Directory containing the project's build outputs.
# Outputs:
#   Prints status information to stdout with ANSI color codes.
print_status_plugin() {
  local plugin_file plugin_version plugin_debugpath
  plugin_file="$EOG_PLUGINS_DIR/libprompt-viewer.plugin"
  plugin_version=$(grep -E "^Version=" "$plugin_file" 2>/dev/null | cut -d'=' -f2)
  plugin_debugpath="$PROJECT_BIN_DIR/debug/$PROMPTVIEWER_PLUGIN"

  echo -ne " - ${CYAN}🧩Plugin ${PURPLE}(${WHITE}${PROMPTVIEWER_PLUGIN}${PURPLE})${NC}      ${BLUE}➔${NC} "

  if [ ! -e "$plugin_file" ]; then
    echo -e "${RED}✖ Not installed${NC}"
  elif [ -L "$plugin_file" ]; then
    local link_target short_target
    link_target=$(readlink "$plugin_file")
    short_target=${link_target/#$HOME/\~}
    if [ "$link_target" -ef "$plugin_debugpath" ]; then
      echo -e "${YELLOW}🪲 Installed in DEBUG mode ${CYAN}(v${plugin_version})${NC}"
    else
      echo -e "${RED}🚨 softlink to: ${CYAN}${short_target}${NC}"
    fi
  elif [ -f "$plugin_file" ]; then
    echo -e "${GREEN}✔ Installed ${CYAN}(v${plugin_version})${NC}"
  else
    echo -e "${RED}⚠ Error: Not a regular file or symlink${NC}"
  fi
}

# Prints the status of the GSettings schema definition file.
# Globals:
#   PROMPTVIEWER_GSCHEMA: Name of the GSettings schema definition file.
#   GIO_SCHEMAS_DIR:      Directory where the GSettings schemas are stored.
# Outputs:
#   Prints status information to stdout with ANSI color codes.
print_status_gschema() {
  local gschema_file="$GIO_SCHEMAS_DIR/$PROMPTVIEWER_GSCHEMA"

  # left part listing the type and filename
  echo -ne " - ${CYAN}📜Schema ${PURPLE}(${WHITE}...prompt-viewer.gschema.xml${PURPLE})${NC} ${BLUE}➔${NC} "

  # right part showing the file status
  if [ -e "$gschema_file" ]; then
    echo -e "${GREEN}✔ Installed correctly${NC}"
  else
    echo -e "${RED}✖ Not installed${NC}"
  fi
}

print_status() {
  print_status_so
  print_status_plugin
  print_status_gschema
}

#---------------------------- HANDLING GSCHEMA -----------------------------#

install_gschema() {
    echo "Installing GSettings schema..."

    local src_file="$PROJECT_ROOT_DIR/$PROMPTVIEWER_GSCHEMA"
    local dest_dir="$GIO_SCHEMAS_DIR"
    local dest_file="$dest_dir/$PROMPTVIEWER_GSCHEMA"

    # validar que el archivo fuente exista
    if [ ! -f "$src_file" ]; then
        echo "Error: GSchema source file not found at '$src_file'." >&2
        return 1
    fi

    # asegurarse de que el directorio de destino exista
    if ! mkdir -p "$dest_dir"; then
        echo "Error: Failed to create destination directory '$dest_dir'." >&2
        return 1
    fi

    # copiar el archivo usando cp
    if ! cp -f "$src_file" "$dest_file"; then
        echo "Error: Failed to copy GSchema file to '$dest_file'." >&2
        return 1
    fi

    # asegurar permisos correctos (0644 es estándar para archivos de configuración)
    chmod 0644 "$dest_file"

    # recompilar esquemas y capturar posibles errores de sintaxis XML
    echo "Compiling GSettings schemas in '$dest_dir'..."
    if ! glib-compile-schemas "$dest_dir"; then
        echo "Error: glib-compile-schemas failed. Check your GSchema XML syntax." >&2
        return 1
    fi

    echo "GSettings schema installed and compiled successfully."
    return 0
}

remove_gschema() {
    local gschema_file="$GIO_SCHEMAS_DIR/$PROMPTVIEWER_GSCHEMA"

    # Verificar si el archivo realmente está instalado
    if [ -f "$gschema_file" ]; then
        echo "Removing GSettings schema: $gschema_file"
        
        if ! rm -f "$gschema_file"; then
            echo "Error: Failed to remove schema file '$gschema_file'." >&2
            return 1
        fi

        # Recompilar para purgar el esquema eliminado del caché del sistema
        echo "Recompiling GSettings schemas..."
        if ! glib-compile-schemas "$GIO_SCHEMAS_DIR"; then
            echo "Error: Failed to recompile schemas after removal." >&2
            return 1
        fi
        
        echo "GSettings schema removed successfully."
    else
        echo "Notice: GSettings schema not found at '$gschema_file' (already removed or never installed)."
    fi
}

#--------------------------------- TARGETS ---------------------------------#

user_accept_overwrite_files() {                                                                                                                                                                                     
  local files=("$EOG_PLUGINS_DIR/$PROMPTVIEWER_SO" "$EOG_PLUGINS_DIR/$PROMPTVIEWER_PLUGIN")                                                                                                                     
  local file                                                                                                                                                                                                    
  local any_exist=false                                                                                                                                                                                         
                                                                                                                                                                                                                
  # check if any file exists
  for file in "${files[@]}"; do                                                                                                                                                                                 
    if [ -e "$file" ]; then                                                                                                                                                                                     
      any_exist=true                                                                                                                                                                                            
      break                                                                                                                                                                                                     
    fi                                                                                                                                                                                                          
  done                                                                                                                                                                                                          
                                                                                                                                                                                                                
  # If any exist, ask for overwrite confirmation once                                                                                                                                                           
  if $any_exist; then                                                                                                                                                                                           
    read -p "Files already exist in $EOG_PLUGINS_DIR. Overwrite? (y/n): " overwrite                                                                                                                             
    if [[ ! "$overwrite" =~ ^[Yy] ]]; then                                                                                                                                                                      
      return 1                                                                                                                                                                                                  
    fi                                                                                                                                                                                                          
  fi                                                                                                                                                                                                            
}                                                                                                                                                                                                               
    

install() {
  echo "Executing install command..."
  make

  if ! user_accept_overwrite_files; then
      echo "Aborting install-debug."                                                                                                                                                                            
      exit 1
  fi

  # copy the files from ./bin to the corresponding EOG directories
  cp -f "$PROJECT_BIN_DIR/$PROMPTVIEWER_SO"     "$EOG_PLUGINS_DIR/$PROMPTVIEWER_SO"                                                                                                                             
  cp -f "$PROJECT_BIN_DIR/$PROMPTVIEWER_PLUGIN" "$EOG_PLUGINS_DIR/$PROMPTVIEWER_PLUGIN"                                                                                                                         
                                                                                                                                                                                                                
  install_gschema                                                                                                                                                                                               
  echo "Install complete."                                                                                                                                                                                      

}

install_debug() {
  echo "Executing install-debug command..."
  make debug

  if ! user_accept_overwrite_files; then
      echo "Aborting install-debug."                                                                                                                                                                            
      exit 1
  fi

  # create soft links
  ln -sf "$PROJECT_BIN_DIR/debug/$PROMPTVIEWER_SO"     "$EOG_PLUGINS_DIR/$PROMPTVIEWER_SO"
  ln -sf "$PROJECT_BIN_DIR/debug/$PROMPTVIEWER_PLUGIN" "$EOG_PLUGINS_DIR/$PROMPTVIEWER_PLUGIN"

  install_gschema
  echo "Debug install complete."
}

# Removes the Prompt Viewer Plugin from Eye of GNOME.
# Globals:
#   EOG_PLUGINS_DIR: Directory where the plugin files are located.
#   GIO_SCHEMAS_DIR: Directory where the GSettings schemas are stored.
#   PROMPTVIEWER_PLUGIN: Name of the plugin descriptor file.
#   PROMPTVIEWER_SO:     Name of the binary file containing the plugin logic.
# Outputs:
#   Prints status information to stdout.
remove() {
  echo "Executing remove command..."

  # remove the two plugin files from EOG_PLUGINS_DIR
  rm -f "$EOG_PLUGINS_DIR/$PROMPTVIEWER_SO"
  rm -f "$EOG_PLUGINS_DIR/$PROMPTVIEWER_PLUGIN"

  # remove the GSettings schema
  remove_gschema
}


# run() {
#   echo "Executing run command..."
#   make install || return 1
#   if [ -e "$TEST_IMAGES_DIR" ]; then
#     EOG_DEBUG_PLUGINS='true' GOBJECT_DEBUG='instance-count' eog "$TEST_IMAGES_DIR" &
#     disown
#   else
#     EOG_DEBUG_PLUGINS='true' GOBJECT_DEBUG='instance-count' eog &
#     disown
#   fi
#   return 0
# }



#================================== START ==================================#

# if no arguments are provided, show the help message
if [ $# -eq 0 ]; then
  echo "$SCRIPT_HELP"
  exit 0
fi

# change to the script directory
cd "$SCRIPT_DIR" || exit

# execute the appropriate function based on the first argument
SCRIPT_STATUS=0
case "$1" in
install)
  install
  SCRIPT_STATUS=$?
  print_status
  ;;
install-debug)
  install_debug
  SCRIPT_STATUS=$?
  print_status
  ;;
remove)
  remove
  SCRIPT_STATUS=$?
  print_status
  ;;
status)
  print_status
  SCRIPT_STATUS=$?
  ;;
# run)
#   run
#   SCRIPT_STATUS=$?
#   ;;
*)
  echo "Invalid command: $1"
  echo "$SCRIPT_HELP"
  exit 1
  ;;
esac

echo "Exiting..."
exit $SCRIPT_STATUS
