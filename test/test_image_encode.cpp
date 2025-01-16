//
// Created by matthew on 1/15/25.
//

#include <catch2/catch_test_macros.hpp>

#include "image_encode.h"


TEST_CASE("Test Image Encoding") {
    const cv::Mat image = cv::Mat::zeros(1, 100, CV_8UC4);

    SECTION("Bit Width 1") {
        cv::Mat outputImage = image.clone();
        const std::string text = "az";
        encodeText(outputImage, text, 1);

        const std::vector<int> expected = { 0, 1, 1, 0, 1, 0, 0, 0, 1, 1, 1, 0, 0, 1, 0, 1 };
        int pixel = -1;
        for (int i = 0; i < expected.size(); i++) {
            if (i % 4 == 0) pixel++;  // Move to the next pixel every 4 elements
            REQUIRE(outputImage.at<cv::Vec4b>(0, pixel)[i % 4] == expected[i]);
        }
    }

    SECTION("Bit Width 2") {
        cv::Mat outputImage = image.clone();
        const std::string text = "hello there";
        encodeText(outputImage, text, 2);

        const std::vector<int> expected = { 0, 2, 2, 1, 1, 1, 2, 1, 0, 3, 2, 1, 0, 3, 2, 1 };
        int pixel = -1;
        for (int i = 0; i < expected.size(); i++) {
            if (i % 4 == 0) pixel++;  // Move to the next pixel every 4 elements
            REQUIRE(outputImage.at<cv::Vec4b>(0, pixel)[i % 4] == expected[i]);
        }
    }

    SECTION("Bit Width 4") {
        cv::Mat outputImage = image.clone();
        const std::string text = "What is your favorite idea";
        encodeText(outputImage, text, 4);

        const std::vector<int> expected = { 10, 14, 6, 1, 6, 8, 14, 2, 4, 0, 6, 9, 14 };
        int pixel = -1;
        for (int i = 0; i < expected.size(); i++) {
            if (i % 4 == 0) pixel++;  // Move to the next pixel every 4 elements
            REQUIRE(outputImage.at<cv::Vec4b>(0, pixel)[i % 4] == expected[i]);
        }
    }
}