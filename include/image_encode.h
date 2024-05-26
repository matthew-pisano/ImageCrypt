//
// Created by matthew on 4/28/24.
//

#ifndef ICRYPT_IMAGE_ENCODE_H
#define ICRYPT_IMAGE_ENCODE_H

#include <opencv2/opencv.hpp>


/**
 * Adds an alpha channel to the image
 * @param image The image to add an alpha channel to
 */
void addAlphaChannel(cv::Mat& image);


/**
 * Encodes text into an image by using the modulo of the pixel values to encode the bytes of the text
 * @param image The image to encode the text into
 * @param text The text to encode
 * @param bitWidth The number of bits to use for encoding within each channel (1, 2, or 4)
 */
void encodeText(cv::Mat& image, const std::string& text, int bitWidth);


/**
 * Decodes text from an image by extracting the encoded bytes from the pixel values
 * @param image The image to decode the text from
 * @param bitWidth The number of bits used for encoding within each channel (1, 2, or 4)
 * @return The decoded text
 */
std::string decodeText(cv::Mat& image, int bitWidth);

#endif //ICRYPT_IMAGE_ENCODE_H
