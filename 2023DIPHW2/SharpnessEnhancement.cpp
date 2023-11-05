#include <iostream>
#include <fstream>
#include <vector>
#include <string>

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

// Function to apply sharpening filter to the image data
void applySharpeningFilter(std::vector<unsigned char>& data, int width, int height, int enhance_degree) {
    std::vector<unsigned char> resultData = data; // Create a copy of the data

    
    int kernel[3][3] = {
            {0, -1, 0},
            {-1,  5, -1},
            {0, -1, 0}
        };

    if (enhance_degree == 2) /* Composite Laplacian kernell 2 (sharper) */
    {
        for(int i = 0; i < 3; i++) {
            for(int j = 0; j < 3; j++) {
                kernel[i][j] = -1;
                if ((i == 1) && (j == 1))
                {
                    kernel[i][j] = 9;
                }                
            }
        }
    }

    int channels = 3; // Assuming RGB image (3 channels)

    for (int y = 1; y < (height - 1); y++){
        for (int x = 1; x < (width - 1); x++){
            for(int c = 0; c < channels; c++){
                int sum = 0;
                for(int j = -1; j <= 1; j++){
                    for(int i = -1; i <= 1; i++){
                        sum += data[(y + j) * (width * channels) + (x + i) * channels + c] * kernel[j + 1][i + 1];
                    }
                }
                if (sum < 0) sum = 0;
                if (sum > 255) sum = 255;
                resultData[y * (width * channels) + x * channels + c] = static_cast<unsigned char>(sum);
            }
        }
    }

    data = resultData; // Update the data with the sharpened result
}

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

    /*Do Sharpness Enhancement on images*/
    applySharpeningFilter(data, width, height, enhance_degree);


    string output_filename = "output2_" + to_string(enhance_degree) + ".bmp";
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

