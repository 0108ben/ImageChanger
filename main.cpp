#include "main.hpp"

int main()
{
    ImageData imageData;

    imageData.convertImageToHex(imageLocationOne);

    string signature;

    // Signature
    imageData.getSmallHexChunk(signature, 8);

    while (!imageData.endOfBytes)
    {
        std::map<string, string> chunk{};
        imageData.getHexChunk(chunk);

        imageData.chunks.push_back({chunk});
    }

    cout << "Image type: " << signature << (signature == pngSignature ? " (PNG)" : "") << endl;

    for (auto& chunk : imageData.chunks)
    {
        imageData.displayChunk(chunk);
    }

    return 0;
}
