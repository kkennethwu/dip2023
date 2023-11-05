#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cmath>

using namespace std;

#pragma pack(push, 1) // Disable structure padding
struct BMPHeader {
    uint16_t type;
    uint32_t size;
    uint16_t reserved1;
    uint16_t reserved2;
    uint32_t offset;
};

struct BMPInfoHeader {
    uint32_t size;
    int32_t width;
    int32_t height;
    uint16_t planes;
    uint16_t bitsPerPixel;
    uint32_t compression;
    uint32_t imageSize;
    int32_t xPixelsPerMeter;
    int32_t yPixelsPerMeter;
    uint32_t colorsUsed;
    uint32_t colorsImportant;
};
#pragma pack(pop)

int main(int argc, char* argv[]) {
    if (argc != 3) {
        cerr << "Usage: " << argv[0] << " k d" << " : k is input_num, " << " d is enhance degree, which should be either 1 or 2." << endl;
        return 1;
    }
    string input_num = string(argv[1]);
    int enhance_degree = stoi(string(argv[2]));
    if ((enhance_degree < 1) || (enhance_degree > 2)) {
        cerr << "Usage: " << argv[0] << " k d" << " : k is input_num, " << " d is enhance degree, which should be either 1 or 2." << endl;
        return 1;
    }

    /* Read BMP */
    string filename = "input" + input_num + ".bmp";
    std::ifstream file(filename, std::ios::in | std::ios::binary);

    if (!file.is_open()) {
        std::cerr << "Error opening the file" << std::endl;
        return 1;
    }

    BMPHeader header;
    BMPInfoHeader infoHeader;

    // Read the headers
    file.read(reinterpret_cast<char*>(&header), sizeof(BMPHeader));
    file.read(reinterpret_cast<char*>(&infoHeader), sizeof(BMPInfoHeader));

    // Check if it's a BMP file
    if (header.type != 0x4D42) {
        std::cerr << "Not a BMP file" << std::endl;
        return 1;
    }
    
    int bitsPerPixel = infoHeader.bitsPerPixel;
    int width = infoHeader.width;
    int height = infoHeader.height;
    int num_channel = bitsPerPixel / 8;
    int imageSize = width * height * num_channel; // Each pixel has RGB or RGBA

    // Allocate memory to store pixel data
    std::vector<unsigned char> data(imageSize);
    // Read pixel data
    file.read(reinterpret_cast<char*>(data.data()), imageSize);
    // Close the file
    file.close();

    /* Apply Gaussian blur to denoise the image */
    int blurRadius = 3; // Adjust the blur radius for more or less blurring
    if(enhance_degree == 2){
        blurRadius = 5;
    }
    for (int y = blurRadius; y < height - blurRadius; y++) {
        for (int x = num_channel * blurRadius; x < (width - blurRadius) * num_channel; x += num_channel) {
            for (int c = 0; c < num_channel; c++) {
                int sum = 0;
                for (int j = -blurRadius; j <= blurRadius; j++) {
                    for (int i = -blurRadius; i <= blurRadius; i++) {
                        sum += data[(y + j) * (width * num_channel) + (x + i * num_channel) + c];
                    }
                }
                int blurredValue = sum / ((2 * blurRadius + 1) * (2 * blurRadius + 1));
                data[y * (width * num_channel) + x + c] = static_cast<unsigned char>(blurredValue);
            }
        }
    }
    string output_filename = "output3_" + to_string(enhance_degree) + ".bmp";
    ofstream output(output_filename, ios::out | ios::binary);
    if (!output.is_open()) {
        std::cerr << "Error creating the output file" << std::endl;
        return -1;
    }

    output.write(reinterpret_cast<const char*>(&header), sizeof(BMPHeader));
    output.write(reinterpret_cast<const char*>(&infoHeader), sizeof(BMPInfoHeader));
    output.write(reinterpret_cast<const char*>(data.data()), data.size());

    output.close();

    return 0;
}
