#!/bin/bash

SHADER_DIR="/Users/deakzach/Desktop/Deako/Deako/assets/shaders"
OUTPUT_DIR="$SHADER_DIR/bin"

# Compile shaders if source files have changed
for shader in "$SHADER_DIR"/*.vert "$SHADER_DIR"/*.frag; do
    BASENAME=$(basename "$shader")
    OUTPUT="$OUTPUT_DIR/${BASENAME}.spv"

    # Check if the shader source has been modified since last compiled
    if [[ "$shader" -nt "$OUTPUT" ]]; then
        echo "Compiling $BASENAME..."
        /Users/deakzach/Desktop/Deako/Deako/vendor/vulkan/1.3.280.1/macOS/bin/glslc "$shader" -o "$OUTPUT"
    fi
done
