#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include "opencv2/imgproc.hpp"
#include "opencv2/imgcodecs.hpp"

using namespace std;
using namespace cv;

void calcPSF(Mat& outputImg, Size filterSize, int len, double theta);
void fftshift(const Mat& inputImg, Mat& outputImg);
void filter2DFreq(const Mat& inputImg, Mat& outputImg, const Mat& H);
void calcWnrFilter(const Mat& input_h_PSF, Mat& output_G, double nsr);
int cal_PSNR(const Mat& img1, const Mat& img2);

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
    // Check if at least one command-line argument is provided
    if (argc < 1) {
        std::cerr << "Usage: " << argv[0] << " <input_id> " << std::endl;
        return 1;  // Return an error code
    }
    std::string input_num = argv[1];
    std::vector<int> Len(3), Snr(3);
    std::vector<double> THETA(3);
    if(input_num == "1") {
        Len = {25, 25, 25};
        THETA = {42, 42, 42};
        Snr = {30, 30, 30};
    }
    else if(input_num == "2") {
        Len = {30, 30, 30};
        THETA = {42, 42, 42};
        Snr = {80, 80, 80};
    }

    /* Read BMP */
    std::string filename = "input" + input_num + ".bmp";
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

    /* Restoration */
    cv::Mat imgRGB = cv::Mat(height, width, CV_8UC3);
    vector<cv::Mat> channels;
    cv::split(imgRGB, channels);
    for(int c = 0; c < 3; c++) {
        for(int i = 0; i < height; i++) {
            for(int j = 0; j < width; j++)
                channels[c].at<uchar>(height-1-i, j) = data[i * width * num_channel + j * num_channel + c];
        }
    }
    
    
    vector<cv::Mat> channelsOut;

    int i = 0;
    for(auto &channel : channels)
    {
        int len = Len[i];
        double theta = THETA[i];
        int snr = Snr[i];
        cv::Mat imgIn = channel;
        cv::Mat imgOut;
        // it needs to process even image only
        Rect roi = Rect(0, 0, imgIn.cols & -2, imgIn.rows & -2);
        //Hw calculation (start)
        cv::Mat Hw, h;
        calcPSF(h, roi.size(), len, theta);
        calcWnrFilter(h, Hw, 1.0 / double(snr));
        //Hw calculation (stop)
        imgIn.convertTo(imgIn, CV_32F);
        // filtering (start)
        filter2DFreq(imgIn(roi), imgOut, Hw);
        // filtering (stop)
        imgOut.convertTo(imgOut, CV_8U);
        normalize(imgOut, imgOut, 0, 255, NORM_MINMAX);
        channelsOut.push_back(imgOut);
        i++;
    }

    //merge 3 channel back to a vector
    vector<unsigned char> dataOut;
    for(int j = 0; j < height; j++)
    {
        for(int k = 0; k < width; k++)
        {
            dataOut.push_back(channelsOut[0].at<uchar>(height-j, k));
            dataOut.push_back(channelsOut[1].at<uchar>(height-j, k));
            dataOut.push_back(channelsOut[2].at<uchar>(height-j, k));
        }
    }
    
    


    /* Write BMP */
    string output_filename = "output" + input_num + ".bmp";
    ofstream output(output_filename, ios::out | ios::binary);
    if (!output.is_open()) {
        std::cerr << "Error creating the output file" << std::endl;
        return -1;
    }

    output.write(reinterpret_cast<const char*>(&header), sizeof(BMPHeader));
    output.write(reinterpret_cast<const char*>(&infoHeader), sizeof(BMPInfoHeader));
    
    output.write(reinterpret_cast<const char*>(dataOut.data()), dataOut.size());

    output.close();
    /* calculate PSNR for input1 */
    if(input_num == "1"){
        cv::Mat imgRGB_ori = imread("input" + input_num + "_ori.bmp");
        cv::Mat test_img = imread("output" + input_num + ".bmp");
        cout << "PSNR: " << cal_PSNR(imgRGB_ori, test_img) << endl;
    }

    return 0;
}

void calcPSF(Mat& outputImg, Size filterSize, int len, double theta)
{
    Mat h(filterSize, CV_32F, Scalar(0));
    // Calculate the center point of the ellipse
    Point point(filterSize.width / 2, filterSize.height / 2);
    // Draw an ellipse on the PSF matrix
    ellipse(h, point, Size(0, cvRound(float(len) / 2.0)), 90.0 - theta, 0, 360, Scalar(255), FILLED);
    // Calculate the sum of all values in the PSF matrix
    Scalar summa = sum(h);
    // Normalize the PSF matrix by dividing each element by the sum
    outputImg = h / summa[0];
}
void fftshift(const Mat& inputImg, Mat& outputImg)
{
    outputImg = inputImg.clone();
    int cx = outputImg.cols / 2;
    int cy = outputImg.rows / 2;
    // Create region-of-interest for each quadrant
    cv::Rect roiTopLeft(0, 0, cx, cy);
    cv::Rect roiTopRight(cx, 0, cx, cy);
    cv::Rect roiBottomLeft(0, cy, cx, cy);
    cv::Rect roiBottomRight(cx, cy, cx, cy);
    // Extract quadrants
    cv::Mat topLeft(outputImg, roiTopLeft);
    cv::Mat topRight(outputImg, roiTopRight);
    cv::Mat bottomLeft(outputImg, roiBottomLeft);
    cv::Mat bottomRight(outputImg, roiBottomRight);
    // Swap quadrants
    cv::Mat tmp;
    topLeft.copyTo(tmp);
    bottomRight.copyTo(topLeft);
    tmp.copyTo(bottomRight);

    topRight.copyTo(tmp);
    bottomLeft.copyTo(topRight);
    tmp.copyTo(bottomLeft);
}
void filter2DFreq(const Mat& inputImg, Mat& outputImg, const Mat& H)
{
    Mat planes[2] = { Mat_<float>(inputImg.clone()), Mat::zeros(inputImg.size(), CV_32F) };
    Mat complexI;
    merge(planes, 2, complexI);
    dft(complexI, complexI, DFT_SCALE);
    Mat planesH[2] = { Mat_<float>(H.clone()), Mat::zeros(H.size(), CV_32F) };
    Mat complexH;
    merge(planesH, 2, complexH);
    Mat complexIH;
    mulSpectrums(complexI, complexH, complexIH, 0);
    idft(complexIH, complexIH);
    split(complexIH, planes);
    outputImg = planes[0];
}

void calcWnrFilter(const Mat& input_h_PSF, Mat& output_G, double nsr)
{
    Mat h_PSF_shifted;
    fftshift(input_h_PSF, h_PSF_shifted);
    Mat planes[2] = { Mat_<float>(h_PSF_shifted.clone()), Mat::zeros(h_PSF_shifted.size(), CV_32F) };
    Mat complexI;
    merge(planes, 2, complexI);
    dft(complexI, complexI);
    split(complexI, planes);
    Mat denom;
    pow(abs(planes[0]), 2, denom);
    denom += nsr;
    divide(planes[0], denom, output_G);
}

int cal_PSNR(const Mat& img1, const Mat& img2)
{
    int MSE = 0;
    int PSNR = 0;
    int height = img1.rows;
    int width = img1.cols;
    for(int i = 0; i < height; i++)
    {
        for(int j = 0; j < width; j++)
            MSE += pow(img1.at<uchar>(i, j) - img2.at<uchar>(i, j), 2);
    }
    MSE /= (height * width);
    PSNR = 10 * log10(255 * 255 / MSE);
    return PSNR;
}


