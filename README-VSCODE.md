
# VSCode Setup

To quickly configure your build and debugging environment in Visual Studio Code,
you can use the configurations provided below.

## Configuration Files

Create a `.vscode` directory in your project's root folder if it doesn't
already exist, and add the following files:

### `c_cpp_properties.json`

```json
{
    "configurations": [
        {
            //
            // To use this configuration, the "compile_commands.json" file must
            // be generated; the Makefile already includes the necessary commands
            // to generate this file, so simply run:
            //  > make compile_commands.json
            //
            "name"           : "Linux",
            "compilerPath"   : "/usr/bin/gcc",
            "cStandard"      : "c99",
            "compileCommands": "${workspaceFolder}/compile_commands.json"
        }
    ],
    "version": 4
}

```

## How to Use

1. **Create the folder:** Create a `.vscode/` directory at the root of your project if you haven't already.
2. **Create the files:** Inside `.vscode/`, create the files.
3. **Copy the content:** Copy the JSON snippets above into their respective files.
4. **Customize:** Adjust paths or compiler flags if your local toolchain differs from the defaults.

> **Note:** Ensure you have the necessary build tools (such as `gcc`,
>           `clang`, or `gdb`) installed on your system and available
>           in your `PATH`.


