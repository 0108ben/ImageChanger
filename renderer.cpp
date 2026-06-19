#include "renderer.hpp"

Renderer::Renderer(sf::RenderTarget& target_) : target(target_) {}

void Renderer::drawImage(std::vector<Pixel>& image, const float width, const float height)
{
    sf::Vector2f pixelSize{windowSize.x/width, windowSize.y/height};

    sf::RectangleShape pixelShape(pixelSize);

    for (float row = 0; row < height; row++)
    {
        for (float col = 0; col < width; col++)
        {
            const int loc = row * width + col;
            Pixel pixel = image[loc];

            pixelShape.setPosition({col*pixelSize.x, row*pixelSize.y});

            const sf::Color pixelColor = {std::uint8_t(pixel.red), std::uint8_t(pixel.green), std::uint8_t(pixel.blue), std::uint8_t(pixel.alpha)};

            pixelShape.setFillColor(pixelColor);
            target.draw(pixelShape);
        }
    }
}