#include "main.hpp"

int main()
{
    ImageData imageData;

    std::vector<std::map<string, string>> chunks{};

    imageData.convertImageToHex(imageLocationOne);

    string signature;

    // Signature
    imageData.getSmallHexChunk(signature, 8);

    while (!imageData.endOfBytes)
    {
        std::map<string, string> chunk{};
        imageData.getHexChunk(chunk);

        chunks.push_back({chunk});
    }

    cout << imageData.hexString << endl;

    cout << "Image type: " << signature << (signature == pngSignature ? " (PNG)" : "") << endl;

    for (auto& chunk : chunks)
    {
        imageData.displayChunk(chunk);
    }

    return 0;
}
