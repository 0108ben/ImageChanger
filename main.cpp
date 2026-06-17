#include "main.hpp"

#include <chrono>

int main()
{
    auto startTotal = std::chrono::high_resolution_clock::now();

    ImageData imageData(imageLocationThree);

    auto stopTotal = std::chrono::high_resolution_clock::now();
    auto durationTotal = std::chrono::duration_cast<std::chrono::nanoseconds>(stopTotal - startTotal);
    cout << "Decryption Time: " << durationTotal.count() << endl;

    cout << "Image type: " << imageData.signature << (imageData.signature == pngSignature ? " (PNG)" : "") << endl;

    for (auto& chunk : imageData.chunks)
    {
        imageData.displayChunk(chunk);
    }

    cout << imageData.hexString << endl;

    return 0;
}
