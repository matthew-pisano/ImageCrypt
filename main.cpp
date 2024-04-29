#include <iostream>
#include <fstream>
#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;


void preprocessImage(Mat& image, int bitWidth) {

    int bitMax = 1 << bitWidth;
    int channelLimit = 256 - bitMax;
    int channelSafeLimit = 256 - 2 * bitMax;
    for (int i = 0; i < image.rows; i++) {
        for (int j = 0; j < image.cols; j++) {

            Vec4b pixel = image.at<Vec4b>(i, j);
            if (pixel[0] >= channelLimit) pixel[0] = channelSafeLimit;
            else pixel[0] = pixel[0] - (pixel[0] % bitMax);

            if (pixel[1] >= channelLimit) pixel[1] = channelSafeLimit;
            else pixel[1] = pixel[1] - (pixel[1] % bitMax);

            if (pixel[2] >= channelLimit) pixel[2] = channelSafeLimit;
            else pixel[2] = pixel[2] - (pixel[2] % bitMax);

            if (pixel[3] >= channelLimit) pixel[3] = channelSafeLimit;
            else pixel[3] = pixel[3] - (pixel[3] % bitMax);
            image.at<Vec4b>(i, j) = pixel;
        }
    }
}


void encodeText(Mat& image, const string& text, int bitWidth) {

    int textLength = text.length();
    int textIndex = 0;
    bool evenPixel = true;

    for (int i = 0; i < image.rows; i++) {
        for (int j = 0; j < image.cols; j++) {

            Vec4b pixel = image.at<Vec4b>(i, j);

            if (textIndex < textLength) {
                if (bitWidth == 1) {
                    int shift = evenPixel ? 4 : 0;
                    for (int k = 0; k < 4; k++)
                        pixel[k] += (text[textIndex] >> (k + shift)) & 1;
                    if (!evenPixel) textIndex++;
                } else if (bitWidth == 2) {
                    for (int k = 0; k < 4; k++)
                        pixel[k] += ((text[textIndex] >> (k * 2)) & 1) + (((text[textIndex] >> (k * 2 + 1)) & 1) << 1);
                    textIndex++;
                }
                else if (bitWidth == 4) {
                    for (int k = 0; k < 2; k++) {
                        for (int l = 0; l < 4; l++)
                            pixel[k*2+1] += ((text[textIndex] >> l) & 1) << (3-l);
                        for (int l = 4; l < 8; l++)
                            pixel[k*2] += ((text[textIndex] >> l) & 1) << (7-l);
                        textIndex++;
                    }
                }
            }

            else if (textIndex > textLength + 1)
                for (int k = 0; k < 4; k++)
                    pixel[k] += rand() % (1 << bitWidth);

            image.at<Vec4b>(i, j) = pixel;

            evenPixel = !evenPixel;
        }
    }
}


string decodeText(Mat& image, int bitWidth) {

    string text = "";
    char character = 0;
    bool evenPixel = true;

    for (int i = 0; i < image.rows; i++) {
        for (int j = 0; j < image.cols; j++) {

            Vec4b pixel = image.at<Vec4b>(i, j);

            if (bitWidth == 1) {
                int shift = evenPixel ? 4 : 0;
                for (int k = 0; k < 4; k++)
                    character |= ((pixel[k] % 2) << (k + shift));

                if (!evenPixel) {
                    if (character == 0) return text;
                    text += character;
                    character = 0;
                }
            }
            else if (bitWidth == 2) {
                for (int k = 0; k < 8; k++)
                    character |= (((pixel[k / 2] % 4) >> (k % 2)) & 1) << k;

                if (character == 0) return text;
                text += character;
                character = 0;
            }
            else if (bitWidth == 4) {
                for (int k = 0; k < 2; k++) {

                    int pixelIdx = k*2;
                    for (int l = 0; l < 8; l++) {
                        character |= (((pixel[pixelIdx] % 16) >> (l % 4)) & 1) << (7 - l);
                        if (l == 3) pixelIdx++;
                    }

                    if (character == 0) return text;
                    text += character;
                    character = 0;
                }
            }

            evenPixel = !evenPixel;
        }
    }

    return text;
}


void addAlphaChannel(Mat& image) {

    cout << "Warning: Image does not have an alpha channel. Adding an alpha channel to output image." << endl;
    vector<Mat> matChannels;
    split(image, matChannels);

    // create alpha channel
    Mat alpha = Mat::ones(image.size(), CV_8UC1) * 255;
    matChannels.push_back(alpha);
    merge(matChannels, image);
}


int main(int argc, char** argv) {

    if (argc < 4 || argc > 5) {
        cout << "Usage: icrypt <encrypt|decrypt> <TEXT_FILE> <INPUT_IMAGE> [OUTPUT_IMAGE]" << endl;
        return -1;
    }
    
    string mode = argv[1];
    if (mode == "encrypt" && argc != 5) {
        cout << "Usage: icrypt encrypt <TEXT_FILE> <INPUT_IMAGE> <OUTPUT_IMAGE>" << endl;
        return -1;
    }
    else if (mode == "decrypt" && argc != 4) {
        cout << "Usage: icrypt decrypt <TEXT_FILE> <INPUT_IMAGE>" << endl;
        return -1;
    }
    else if (mode != "encrypt" && mode != "decrypt") {
        cout << "Usage: icrypt <encrypt|decrypt> <TEXT_FILE> <INPUT_IMAGE> <OUTPUT_IMAGE>" << endl;
        return -1;
    }

    string txtPth = argv[2];
    string inputImPth = argv[3];
    string outputImPth;
    if (mode == "encrypt") outputImPth = argv[4];
    int bitWidth = 2;

    string decodedText;

    ifstream inFile;
    ofstream outFile;
    if (mode == "encrypt") {
        inFile = ifstream(txtPth);
        if (!inFile.is_open()) {
            cout << "Could not open file" << endl;
            return -1;
        }
        string line;
        while (getline(inFile, line)) decodedText += line + "\n";
        decodedText.pop_back(); // Remove last newline character
        inFile.close();
    }
    else outFile = ofstream(txtPth);

    Mat image = imread(inputImPth, IMREAD_UNCHANGED);
    if (image.channels() == 3) {
        if (mode == "decrypt") {
            cout << "Error: Image does not have an alpha channel. Cannot decrypt." << endl;
            return -1;
        }
        addAlphaChannel(image);
    }

    if (image.empty()) {
        cout << "Could not open or find the image" << endl;
        return -1;
    }

    if (mode == "encrypt") {
        int overflow = decodedText.length() - (image.rows * image.cols);
        if (overflow > 0)
            cout << "Warning: The last " << overflow << "character of text will be truncated!" << endl;

        Mat outputImage = image.clone();
        preprocessImage(outputImage, bitWidth);
        encodeText(outputImage, decodedText, bitWidth);
        // Write the image
        imwrite(outputImPth, outputImage);
    }
    else {
        decodedText = decodeText(image, bitWidth);
        outFile << decodedText;
        outFile.close();
    }

    return 0;
}
