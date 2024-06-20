/**
 * @File Screen.hpp
 * @Author dfnzhc (https://github.com/dfnzhc)
 * @Date 2024/6/21
 * @Brief This file is part of Elma.
 */

#pragma once

#include "Image.hpp"
#include <nanogui/screen.h>

namespace elma {

class ElmaScreen : public nanogui::Screen
{
public:
    explicit ElmaScreen(const Image3f& img);

    void draw_contents() override;

    bool keyboard_event(int key, int scancode, int action, int modifiers) override;

private:
    const Image3f& _img;
    nanogui::ref<nanogui::Shader> _shader;
    nanogui::ref<nanogui::Texture> _texture;
    nanogui::ref<nanogui::RenderPass> _renderPass;
    float _scale = 0.007f;
};

} // namespace elma