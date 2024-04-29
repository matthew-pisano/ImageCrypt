# Image-Crypt Document Encoder

A simple document encoder in C++. Documents are encoded based off of noise inserted into a target image. The noise may be generated based off of a key file.

## Usage

The `icrypt` executable has two sub-commands, `encode` and `decode`. Each of these commands accepts one or more input files, a required output file, and optional arguments.

```bash
icrypt encode <input_image> <text-file> < -o output_image> [-e encoding] [-k key_file] [-b bit_width]

icrypt decode <input_image> < -o output_text> [-e encoding] [-k key_file] [-b bit_width]
```

## Image Encoding

Text is encoded into an image by inserting noise into the image. The noise is generated based off of the text to be encoded and an optional key file.  If a key file is given, the raw text will first be encoded using the key before being encoded into the image.  This encoding can only be performed on a 4-channel image (RGBA); if a three-channel image is given, the image will be converted to a four-channel image by adding an alpha channel.

### Noise Generation

The noise added to an image using one of three different bit widths. The bit width determines the number of bits that will be used to encode the text into each pixel channel. The bit width can be set using the `-b` flag. The following bit widths are available: 1, 2, and 4.

First, each channel of each pixel is rounded down to the nearest multiple of the selected bit width's max value (2 for b.w. 1, 4 for b.w. 2, and 16 for b.w. 4).  Next, the bits of each character are encoded into an image's pixels by adding `[0-bitWidth)` to it, depending on the bits within the character to be encoded.

For example, if the bit width is 2, each pixel channel will encode two bits of a character by adding `[0-4)` to the rounded down channel value. If the channel value is 100, the character value is *M* (*ASCII*-77), the first two bits of *M* (`1001101`) are `01` (little-endian), so the channel value will be set to 101.  The next channel will encode the next two bits of `1001101`: `11`.  If the value of the next channel is 124, its new value will be 127.  With this bit width, each 4-channel pixel can encode a single, 1-byte character as each channel encodes 2-bits within its noise.  The noise always stays within the space created by the rounding, so the modulo of the value can be used to losslessly decode the text.

The resulting image appears nearly identical to the original, but with a small amount of noise added from the text encoding.

## Text Encodings

The following encodings are available:

### Shift Encoding

Documents encoded with the shift encoding have their characters shifted upward based off of the hash of the key given.

### Character Shift Encoding

Documents encoded in this manner have each of their characters shifted based off of the hash of the character index and the key. This ensures that the contents cannot be decoded by simply guessing the shift value as each character may have a different shift.