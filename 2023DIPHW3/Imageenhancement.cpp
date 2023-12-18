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

void rgbToHsv(unsigned char r, unsigned char g, unsigned char b, double& h, double& s, double& v) {
    double minVal = std::min(std::min(r, g), b);
    double maxVal = std::max(std::max(r, g), b);
    v = maxVal / 255.0;

    double delta = maxVal - minVal;

    if (maxVal == 0) {
        s = 0;
    } else {
        s = delta / maxVal;
    }

    if (delta == 0) {
        h = 0;
    } else {
        if (r == maxVal) {
            h = (g - b) / delta;
        } else if (g == maxVal) {
            h = 2 + (b - r) / delta;
        } else {
            h = 4 + (r - g) / delta;
        }

        h *= 60;

        if (h < 0) {
            h += 360;
        }
    }
}

// Function to convert HSV to RGB
void hsvToRgb(double h, double s, double v, unsigned char& r, unsigned char& g, unsigned char& b) {
    if (s == 0) {
        r = g = b = static_cast<unsigned char>(v * 255.0);
    } else {
        h /= 60;
        int i = static_cast<int>(std::floor(h));
        double f = h - i;
        double p = v * (1 - s);
        double q = v * (1 - s * f);
        double t = v * (1 - s * (1 - f));

        switch (i) {
            case 0: r = static_cast<unsigned char>(v * 255.0); g = static_cast<unsigned char>(t * 255.0); b = static_cast<unsigned char>(p * 255.0); break;
            case 1: r = static_cast<unsigned char>(q * 255.0); g = static_cast<unsigned char>(v * 255.0); b = static_cast<unsigned char>(p * 255.0); break;
            case 2: r = static_cast<unsigned char>(p * 255.0); g = static_cast<unsigned char>(v * 255.0); b = static_cast<unsigned char>(t * 255.0); break;
            case 3: r = static_cast<unsigned char>(p * 255.0); g = static_cast<unsigned char>(q * 255.0); b = static_cast<unsigned char>(v * 255.0); break;
            case 4: r = static_cast<unsigned char>(t * 255.0); g = static_cast<unsigned char>(p * 255.0); b = static_cast<unsigned char>(v * 255.0); break;
            default: r = static_cast<unsigned char>(v * 255.0); g = static_cast<unsigned char>(p * 255.0); b = static_cast<unsigned char>(q * 255.0); break;
        }
    }
}

// Function to enhance saturation
void enhanceSaturation(std::vector<unsigned char>& data, double factor, double val_factor) {
    for (size_t i = 0; i < data.size(); i += 3) { // Assuming 3 channels (RGB) per pixel
        unsigned char& r = data[i];
        unsigned char& g = data[i + 1];
        unsigned char& b = data[i + 2];

        // Convert RGB to HSV
        double h, s, v;
        rgbToHsv(r, g, b, h, s, v);

        // Enhance saturation
        s *= factor;

        // Clip saturation to the valid range [0, 1]
        s = std::max(0.0, std::min(1.0, s));

        // Enhance value
        v = std::min(1.0, v * val_factor);

        // Convert back to RGB
        hsvToRgb(h, s, v, r, g, b);
    }
}

void adjustContrast(std::vector<unsigned char>& data, double contrastFactor) {
    for (size_t i = 0; i < data.size(); ++i) {
        double adjustedIntensity = contrastFactor * (static_cast<double>(data[i]) - 128.0) + 128.0;
        
        // Clip the adjusted intensity to the valid range [0, 255]
        data[i] = static_cast<unsigned char>(std::max(0.0, std::min(255.0, adjustedIntensity)));
    }
}

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
    string filename = "output" + input_num + "_1.bmp";
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

    /* Chromatic Adaptation */
    if (input_num == "1") {
        enhanceSaturation(data, 1.3, 1.4);
        adjustContrast(data, 1.2);
    }
    else if (input_num == "2") {
        // applySharpeningFilter(data, width, height, 1);
        enhanceSaturation(data, 0.7, 1.5);
        adjustContrast(data, 1.2);
        // applySharpeningFilter(data, width, height, 1);
    }
    else if (input_num == "3") {
        enhanceSaturation(data, 1.4, 1.6);
        adjustContrast(data, 1.1);
    }
    else if (input_num == "4") {
        
        enhanceSaturation(data, 1.4, 0.8);
        adjustContrast(data, 1.4);
    }

    

        

    

    string output_filename = "output" + input_num + "_" + to_string(enhance_degree) + ".bmp";
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

