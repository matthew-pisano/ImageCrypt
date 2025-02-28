//
// Created by matthew on 4/28/24.
//

#include "image_encode.h"

#include <random>


/**
 * Preprocesses the image by rounding the pixel values to the nearest multiple of the bit width
 * @param image The image to preprocess
 * @param bitWidth The number of bits to use for encoding within each channel (1, 2, or 4)
 */
void preprocessImage(cv::Mat& image, const int bitWidth) {

    const int bitMax = 1 << bitWidth;  // The maximum value that each channel can vary by
    for (int i = 0; i<image.rows; i++) {
        for (int j = 0; j<image.cols; j++) {

            cv::Vec4b pixel = image.at<cv::Vec4b>(i, j);
            // Ground the pixel values to the nearest multiple of the bit width
            pixel[0] = pixel[0] - (pixel[0] % bitMax);
            pixel[1] = pixel[1] - (pixel[1] % bitMax);
            pixel[2] = pixel[2] - (pixel[2] % bitMax);
            pixel[3] = pixel[3] - (pixel[3] % bitMax);

            image.at<cv::Vec4b>(i, j) = pixel;
        }
    }
}


/**
 * Adds an alpha channel to the image
 * @param image The image to add an alpha channel to
 */
void addAlphaChannel(cv::Mat& image) {

    std::cerr << "Warning: Image does not have an alpha channel. Adding an alpha channel to output image." << std::endl;
    std::vector<cv::Mat> matChannels;
    split(image, matChannels);

    // Create alpha channel
    const cv::Mat alpha = cv::Mat::ones(image.size(), CV_8UC1) * 255;
    matChannels.push_back(alpha);
    merge(matChannels, image);
}


/**
 * Gets the character at the given index in the text, or a random character if the index is out of bounds
 * @param text The text to get the character from
 * @param index The index of the character to get
 * @return The character at the given index in the text, or a random character if the index is out of bounds
 */
char getChar(const std::string& text, const int index) {
    if (index < text.length())
        return text[index];
    if (index > text.length() + 1) {  // Add noise outside the text to disguise the end of the message
        std::random_device dev;
        std::mt19937 rng(dev());
        std::uniform_int_distribution<std::mt19937::result_type> dist6(0,63);  // Random character from base64

        return "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[dist6(rng) % 64];
    }
    return 0;
}


void encodeText(cv::Mat& image, const std::string& text, const int bitWidth) {

    // Add an alpha channel if the image does not have one and ground the pixel values
    if (image.channels() == 3) addAlphaChannel(image);
    preprocessImage(image, bitWidth);

    int textIndex = 0;
    bool evenPixel = true;

    for (int i = 0; i<image.rows; i++) {
        for (int j = 0; j<image.cols; j++) {

            cv::Vec4b pixel = image.at<cv::Vec4b>(i, j);

            char current = getChar(text, textIndex);

            // 1-Bit encoding: encode the text over two pixels
            if (bitWidth == 1) {
                // Add each quartet of bits in reverse order like [3210][7654]
                const int shift = evenPixel ? 4 : 0;
                for (int k = 0; k < 4; k++)
                    pixel[k] += (current >> (k + shift)) & 1;
                // Process character every other pixel
                if (!evenPixel) textIndex++;
            }
            // 2-Bit encoding: encode the text over one pixel
            else if (bitWidth == 2) {
                // Add each pair of bits in reverse order like [67452301]
                for (int k = 0; k < 4; k++)
                    pixel[k] += ((current >> (k * 2)) & 1) + (((current >> (k * 2 + 1)) & 1) << 1);
                textIndex++;
            }
            // 4-Bit encoding: encode the text over two pixel channels
            else if (bitWidth == 4) {
                // Add two octets of bits in order in each pixel like [01234567 01234567]
                for (int k = 0; k < 2; k++) {
                    for (int l = 0; l < 4; l++)  // First 4 bits in the first channel
                        pixel[k * 2 + 1] += ((current >> l) & 1) << (3 - l);
                    for (int l = 4; l < 8; l++)  // Last 4 bits in the second channel
                        pixel[k * 2] += ((current >> l) & 1) << (7 - l);
                    textIndex++;
                    current = getChar(text, textIndex);
                }
            }

            image.at<cv::Vec4b>(i, j) = pixel;
            evenPixel = !evenPixel;
        }
    }
}


std::string decodeText(cv::Mat& image, const int bitWidth) {

    if (image.channels() == 3) {
        std::cerr << "Error: Image does not have an alpha channel. Cannot decode." << std::endl;
        exit(-1);
    }

    std::string text;
    unsigned char character = 0;
    bool evenPixel = true;

    for (int i = 0; i<image.rows; i++) {
        for (int j = 0; j<image.cols; j++) {

            cv::Vec4b pixel = image.at<cv::Vec4b>(i, j);

            // 1-Bit decoding: decode the text from two pixels
            if (bitWidth == 1) {
                const int shift = evenPixel ? 4 : 0;
                for (int k = 0; k < 4; k++)
                    character |= (pixel[k] % 2) << (k + shift);

                // Process character every other pixel
                if (!evenPixel) {
                    text += static_cast<char>(character);
                    character = 0;
                }

                evenPixel = !evenPixel;
            }
            // 2-Bit decoding: decode the text from one pixel
            else if (bitWidth == 2) {
                for (int k = 0; k < 8; k++)
                    character |= (((pixel[k / 2] % 4) >> (k % 2)) & 1) << k;

                text += static_cast<char>(character);
                character = 0;
            }
            // 4-Bit decoding: decode the text from two pixel channels
            else if (bitWidth == 4) {
                for (int k = 0; k < 2; k++) {
                    int pixelIdx = k * 2;
                    for (int l = 0; l<8; l++) {
                        character |= (((pixel[pixelIdx] % 16) >> (l % 4)) & 1) << (7 - l);
                        if (l == 3) pixelIdx++;
                    }

                    text += static_cast<char>(character);
                    character = 0;
                }
            }

            // Check for the end of the message
            if (!text.empty() && text.back() == '\0') {
                while (text.back() == '\0') text.pop_back();  // Remove null bytes at the end of the message
                return text;
            }
        }
    }

    return text;
}
