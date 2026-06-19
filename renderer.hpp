#pragma once

#include "ImageConverter.hpp"
#include <SFML/Graphics.hpp>

inline static const sf::Vector2u windowSize(800, 800);

struct Renderer
{
    Renderer(sf::RenderTarget& target);

    void drawImage(std::vector<Pixel>& image, float width, float height);

private:
    sf::RenderTarget& target;
};