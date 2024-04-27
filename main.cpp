#include <iostream>
#include <fstream>
#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;


void preprocessImage(Mat& image) {

    for (int i = 0; i < image.rows; i++) {
        for (int j = 0; j < image.cols; j++) {

            Vec4b pixel = image.at<Vec4b>(i, j);
            if (pixel[0] >= 252) pixel[0] = 248;
            else pixel[0] = pixel[0] - (pixel[0] % 4);

            if (pixel[1] >= 252) pixel[1] = 248;
            else pixel[1] = pixel[1] - (pixel[1] % 4);

            if (pixel[2] >= 252) pixel[2] = 248;
            else pixel[2] = pixel[2] - (pixel[2] % 4);

            if (pixel[3] >= 252) pixel[3] = 248;
            else pixel[3] = pixel[3] - (pixel[3] % 4);
            image.at<Vec4b>(i, j) = pixel;
        }
    }
}


void encodeText(Mat& image, const string& text) {

    int textLength = text.length();
    int textIndex = 0;

    for (int i = 0; i < image.rows; i++) {
        for (int j = 0; j < image.cols; j++) {

            Vec4b pixel = image.at<Vec4b>(i, j);

            for (int k = 0; k < 4; k++) {
                if (pixel[k] % 4 != 0) {
                    pixel[k] -= pixel[k] % 4;
                }
            }

            if (textIndex < textLength) {
                pixel[0] += 2 * ((text[textIndex] >> 0) & 1) + ((text[textIndex] >> 1) & 1);
                pixel[1] += 2 * ((text[textIndex] >> 2) & 1) + ((text[textIndex] >> 3) & 1);
                pixel[2] += 2 * ((text[textIndex] >> 4) & 1) + ((text[textIndex] >> 5) & 1);
                pixel[3] += 2 * ((text[textIndex] >> 6) & 1) + ((text[textIndex] >> 7) & 1);
            } else if (textIndex > textLength+1) {
                pixel[0] += rand() % 4;
                pixel[1] += rand() % 4;
                pixel[2] += rand() % 4;
                pixel[3] += rand() % 4;
            }

            image.at<Vec4b>(i, j) = pixel;

            textIndex++;
        }
    }
}


string decodeText(Mat& image) {

    string text = "";

    for (int i = 0; i < image.rows; i++) {
        for (int j = 0; j < image.cols; j++) {

            Vec4b pixel = image.at<Vec4b>(i, j);

            char character = 0;
            character |= (((pixel[0] % 4) >> 1) & 1) << 0;
            character |= (((pixel[0] % 4) >> 0) & 1) << 1;
            character |= (((pixel[1] % 4) >> 1) & 1) << 2;
            character |= (((pixel[1] % 4) >> 0) & 1) << 3;
            character |= (((pixel[2] % 4) >> 1) & 1) << 4;
            character |= (((pixel[2] % 4) >> 0) & 1) << 5;
            character |= (((pixel[3] % 4) >> 1) & 1) << 6;
            character |= (((pixel[3] % 4) >> 0) & 1) << 7;

            // Break if null character is found
            if (character == 0) return text;

            text += character;
        }
    }

    return text;
}


void addAlphaChannel(Mat& image) {

    cout << "Warning: Image does not have an alpha channel. Adding an alpha channel." << endl;
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
        preprocessImage(outputImage);
        encodeText(outputImage, decodedText);
        // Write the image
        imwrite(outputImPth, outputImage);
    }
    else {
        decodedText = decodeText(image);
        outFile << decodedText;
        outFile.close();
    }

    return 0;
}
