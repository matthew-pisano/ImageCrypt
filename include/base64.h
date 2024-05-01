//
// Created by matthew on 4/30/24.
//

#ifndef ICRYPT_BASE64_H
#define ICRYPT_BASE64_H


/**
 * Encodes a string into base64
 * @param in The string to encode
 * @return The base64 encoded string
 */
std::string base64Encode(const std::string& in);


/**
 * Decodes a base64 encoded string
 * @param in The base64 encoded string
 * @return The decoded string
 */
std::string base64Decode(const std::string& in);

#endif //ICRYPT_BASE64_H
