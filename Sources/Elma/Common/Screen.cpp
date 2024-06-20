/**
 * @File Screen.cpp
 * @Author dfnzhc (https://github.com/dfnzhc)
 * @Date 2024/6/21
 * @Brief This file is part of Elma.
 */

#include "Screen.hpp"

#include <nanogui/shader.h>
#include <nanogui/label.h>
#include <nanogui/slider.h>
#include <nanogui/layout.h>
#include <nanogui/renderpass.h>
#include <nanogui/texture.h>
#include <GLFW/glfw3.h>

using namespace elma;

ElmaScreen::ElmaScreen(const Image3f& img)
: nanogui::Screen(nanogui::Vector2i(img.width, img.height + 36), "Elma", false), _img(img)
{
    using namespace nanogui;
    inc_ref();

    /* Add some UI elements to adjust the exposure value */
    auto* panel = new Widget(this);
    panel->set_layout(new BoxLayout(Orientation::Horizontal, Alignment::Middle, 10, 10));
    new Label(panel, "Exposure value: ", "sans-bold");
    auto* slider = new Slider(panel);
    slider->set_value(.1f);
    slider->set_fixed_width(150);
    slider->set_callback([&](float value) { _scale = std::pow(2.f, (value - 0.5f) * 20); });

    panel->set_size(nanogui::Vector2i(_img.width, _img.height));
    perform_layout();

    panel->set_position(nanogui::Vector2i((m_size.x() - panel->size().x()) / 2, _img.height));

    /* Simple gamma tonemapper as a GLSL shader */
    _renderPass = new RenderPass({this});
    _renderPass->set_clear_color(0, Color(0.3f, 0.3f, 0.3f, 1.f));

    _shader = new Shader(_renderPass,
                         /* An identifying name */
                         "Tonemapper",
                         /* Vertex shader */
                         R"(#version 330
        uniform ivec2 size;
        uniform int borderSize;

        in vec2 position;
        out vec2 uv;
        void main() {
            gl_Position = vec4(position.x * 2 - 1, position.y * 2 - 1, 0.0, 1.0);

            // Crop away image border (due to pixel filter)
            vec2 total_size = size + 2 * borderSize;
            vec2 scale = size / total_size;
            uv = vec2(position.x * scale.x + borderSize / total_size.x,
                      1 - (position.y * scale.y + borderSize / total_size.y));
        })",
                         /* Fragment shader */
                         R"(#version 330
        uniform sampler2D source;
        uniform float scale;
        in vec2 uv;
        out vec4 out_color;
        float toSRGB(float value) {
            if (value < 0.0031308)
                return 12.92 * value;
            return 1.055 * pow(value, 0.41666) - 0.055;
        }
        void main() {
            vec4 color = texture(source, uv);
            color *= scale / color.w;
            out_color = vec4(toSRGB(color.r), toSRGB(color.g), toSRGB(color.b), 1);
        })");

    // Draw 2 triangles
    uint32_t indices[3 * 2] = {0, 1, 2, 2, 3, 0};
    float positions[2 * 4]  = {0.f, 0.f, 1.f, 0.f, 1.f, 1.f, 0.f, 1.f};

    _shader->set_buffer("indices", VariableType::UInt32, {3 * 2}, indices);
    _shader->set_buffer("position", VariableType::Float32, {4, 2}, positions);

    _shader->set_uniform("size", nanogui::Vector2i(_img.width, _img.height));
    _shader->set_uniform("borderSize", 0);

    // Allocate texture memory for the rendered image
    _texture = new nanogui::Texture(nanogui::Texture::PixelFormat::RGB,
                                    nanogui::Texture::ComponentFormat::Float32,
                                    nanogui::Vector2i(_img.width, _img.height),
                                    nanogui::Texture::InterpolationMode::Nearest,
                                    nanogui::Texture::InterpolationMode::Nearest);

    draw_all();
    set_visible(true);
}

void ElmaScreen::draw_contents()
{
    // Reload the partially rendered image onto the GPU
    _shader->set_uniform("scale", _scale);
    _renderPass->resize(framebuffer_size());
    _renderPass->begin();
    _renderPass->set_viewport(nanogui::Vector2i(0, 0), nanogui::Vector2i(_img.width, _img.height));
    _texture->upload((uint8_t*)_img.data.data());
    _shader->set_texture("source", _texture);
    _shader->begin();
    _shader->draw_array(nanogui::Shader::PrimitiveType::Triangle, 0, 6, true);
    _shader->end();
    _renderPass->set_viewport(nanogui::Vector2i(0, 0), framebuffer_size());
    _renderPass->end();
}

bool ElmaScreen::keyboard_event(int key, int scancode, int action, int modifiers)
{
    if (key == GLFW_KEY_ESCAPE || action == GLFW_PRESS) {
        nanogui::leave();
    }

    return Screen::keyboard_event(key, scancode, action, modifiers);
}