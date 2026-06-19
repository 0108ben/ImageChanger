#include "main.hpp"

int main()
{
    auto startTotal = std::chrono::high_resolution_clock::now();

    ImageData imageData(imageLocationSeven);

    auto stopTotal = std::chrono::high_resolution_clock::now();
    auto durationTotal = std::chrono::duration_cast<std::chrono::nanoseconds>(stopTotal - startTotal);
    cout << "Decryption Time: " << durationTotal.count() << endl;

    cout << "Image type: " << imageData.signature << (imageData.signature == pngSignature ? " (PNG)" : "") << endl;

    for (auto& chunk : imageData.chunks)
    {
        imageData.displayChunk(chunk);
    }

    cout << imageData.hexString << endl;

    sf::RenderWindow window(sf::VideoMode(windowSize), "Image", sf::State::Windowed);
    window.setFramerateLimit(1);

    Renderer renderer(window);
    renderer.drawImage(imageData.image, imageData.imageWidth, imageData.imageHeight);
    window.display();

    while (window.isOpen())
    {
        while (const auto event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
            {
                window.close();
            }
        }
    }

    return 0;
}
