#pragma once

#include <string_view>

// this is a quick test for calling opengl functions

namespace prebuilt_shaders {

  constexpr std::string_view vertex_shader_code = "#version 450 core\n" \
                                                  "layout (location = 0) in vec3 aPos; // the position variable has attribute position 0\n" \
                                                  "layout (location = 1) in vec3 aCol; // the color variable has attribute position 1\n" \
                                                  "layout (location = 2) in vec2 aUV0; // the texoord0 variable has attribute position 2\n" \
                                                  "layout (location = 3) in vec2 aUV1; // the texoord1 variable has attribute position 3\n" \
                                                  "\n" \
                                                  "uniform vec4 color; // specify a color output to the fragment shader\n" \
                                                  "uniform mat4 ortho; // specify 2d view matrix\n" \
                                                  "uniform mat4 model; // specify object location matrix\n" \
                                                  "\n" \
                                                  "out vec4 vertexColor; // specify a color output to the fragment shader\n" \
                                                  "out vec4 col; // specify a color output to the fragment shader\n" \
                                                  "out vec2 texcoord0; // specify a color output to the fragment shader\n" \
                                                  "\n" \
                                                  "void main()\n" \
                                                  "{\n" \
                                                  "    gl_Position = ortho * model * vec4(aPos, 1.0); // see how we directly give a vec3 to vec4's constructor\n" \
                                                  "    vertexColor = color;\n" \
                                                  "    col = vec4(aCol, 1.0);\n" \
                                                  "    texcoord0 = aUV0;\n" \
                                                  "}";

  constexpr std::string_view fragment_shader_code = "#version 450 core\n" \
                                                    "out vec4 FragColor;\n" \
                                                    "uniform sampler2D image;\n" \
                                                    "  \n" \
                                                    "in vec2 texcoord0;\n" \
                                                    "in vec4 col;\n" \
                                                    "in vec4 vertexColor; // the input variable from the vertex shader (same name and same type)  \n" \
                                                    "\n" \
                                                    "void main()\n" \
                                                    "{\n" \
                                                    "    vec4 tex_color = texture2D(image, texcoord0);" \
                                                    "    //FragColor = vertexColor;\n" \
                                                    "    FragColor = tex_color;\n" \
                                                    "}";

}