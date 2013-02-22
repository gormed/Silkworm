
/*

    image.cpp

    class for loading opengl textures from png files

*/


#include <GL3/gl3w.h>

#include "log.h"

#include "image.h"

#include <vector>


struct PNGInfo
{
  unsigned long width, height, colorType, bitDepth, compressionMethod, filterMethod, interlaceMethod, key_r, key_g, key_b;
  bool key_defined; //is a transparent color key given?
  std::vector<unsigned char> palette;
};


int decodePNG(std::vector<unsigned char>& out_image, PNGInfo& out_info, const unsigned char* in_png, size_t in_size)
{
  // picoPNG version 20101224
  // Copyright (c) 2005-2010 Lode Vandevenne
  //
  // This software is provided 'as-is', without any express or implied
  // warranty. In no event will the authors be held liable for any damages
  // arising from the use of this software.
  //
  // Permission is granted to anyone to use this software for any purpose,
  // including commercial applications, and to alter it and redistribute it
  // freely, subject to the following restrictions:
  //
  //     1. The origin of this software must not be misrepresented; you must not
  //     claim that you wrote the original software. If you use this software
  //     in a product, an acknowledgment in the product documentation would be
  //     appreciated but is not required.
  //     2. Altered source versions must be plainly marked as such, and must not be
  //     misrepresented as being the original software.
  //     3. This notice may not be removed or altered from any source distribution.

  // picoPNG is a PNG decoder in one C++ function of around 500 lines. Use picoPNG for
  // programs that need only 1 .cpp file. Since it's a single function, it's very limited,
  // it can convert a PNG to raw pixel data either converted to 32-bit RGBA color or
  // with no color conversion at all. For anything more complex, another tiny library
  // is available: LodePNG (lodepng.c(pp)), which is a single source and header file.
  // Apologies for the compact code style, it's to make this tiny.

  //    --- i have altered this code. roman ----


  static const unsigned long LENBASE[29] =  {3,4,5,6,7,8,9,10,11,13,15,17,19,23,27,31,35,43,51,59,67,83,99,115,131,163,195,227,258};
  static const unsigned long LENEXTRA[29] = {0,0,0,0,0,0,0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4,  4,  5,  5,  5,  5,  0};
  static const unsigned long DISTBASE[30] =  {1,2,3,4,5,7,9,13,17,25,33,49,65,97,129,193,257,385,513,769,1025,1537,2049,3073,4097,6145,8193,12289,16385,24577};
  static const unsigned long DISTEXTRA[30] = {0,0,0,0,1,1,2, 2, 3, 3, 4, 4, 5, 5,  6,  6,  7,  7,  8,  8,   9,   9,  10,  10,  11,  11,  12,   12,   13,   13};
  static const unsigned long CLCL[19] = {16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15}; //code length code lengths
  struct Zlib //nested functions for zlib decompression
  {
    static unsigned long readBitFromStream(size_t& bitp, const unsigned char* bits) { unsigned long result = (bits[bitp >> 3] >> (bitp & 0x7)) & 1; bitp++; return result;}
    static unsigned long readBitsFromStream(size_t& bitp, const unsigned char* bits, size_t nbits)
    {
      unsigned long result = 0;
      for(size_t i = 0; i < nbits; i++) result += (readBitFromStream(bitp, bits)) << i;
      return result;
    }
    struct HuffmanTree
    {
      int makeFromLengths(const std::vector<unsigned long>& bitlen, unsigned long maxbitlen)
      { //make tree given the lengths
        unsigned long numcodes = (unsigned long)(bitlen.size()), treepos = 0, nodefilled = 0;
        std::vector<unsigned long> tree1d(numcodes), blcount(maxbitlen + 1, 0), nextcode(maxbitlen + 1, 0);
        for(unsigned long bits = 0; bits < numcodes; bits++) blcount[bitlen[bits]]++; //count number of instances of each code length
        for(unsigned long bits = 1; bits <= maxbitlen; bits++) nextcode[bits] = (nextcode[bits - 1] + blcount[bits - 1]) << 1;
        for(unsigned long n = 0; n < numcodes; n++) if(bitlen[n] != 0) tree1d[n] = nextcode[bitlen[n]]++; //generate all the codes
        tree2d.clear(); tree2d.resize(numcodes * 2, 32767); //32767 here means the tree2d isn't filled there yet
        for(unsigned long n = 0; n < numcodes; n++) //the codes
        for(unsigned long i = 0; i < bitlen[n]; i++) //the bits for this code
        {
          unsigned long bit = (tree1d[n] >> (bitlen[n] - i - 1)) & 1;
          if(treepos > numcodes - 2) return 55;
          if(tree2d[2 * treepos + bit] == 32767) //not yet filled in
          {
            if(i + 1 == bitlen[n]) { tree2d[2 * treepos + bit] = n; treepos = 0; } //last bit
            else { tree2d[2 * treepos + bit] = ++nodefilled + numcodes; treepos = nodefilled; } //addresses are encoded as values > numcodes
          }
          else treepos = tree2d[2 * treepos + bit] - numcodes; //subtract numcodes from address to get address value
        }
        return 0;
      }
      int decode(bool& decoded, unsigned long& result, size_t& treepos, unsigned long bit) const
      { //Decodes a symbol from the tree
        unsigned long numcodes = (unsigned long)tree2d.size() / 2;
        if(treepos >= numcodes) return 11; //error: you appeared outside the codetree
        result = tree2d[2 * treepos + bit];
        decoded = (result < numcodes);
        treepos = decoded ? 0 : result - numcodes;
        return 0;
      }
      std::vector<unsigned long> tree2d; //2D representation of a huffman tree: The one dimension is "0" or "1", the other contains all nodes and leaves of the tree.
    };
    struct Inflator
    {
      int error;
      void inflate(std::vector<unsigned char>& out, const std::vector<unsigned char>& in, size_t inpos = 0)
      {
        size_t bp = 0, pos = 0; //bit pointer and byte pointer
        error = 0;
        unsigned long BFINAL = 0;
        while(!BFINAL && !error)
        {
          if(bp >> 3 >= in.size()) { error = 52; return; } //error, bit pointer will jump past memory
          BFINAL = readBitFromStream(bp, &in[inpos]);
          unsigned long BTYPE = readBitFromStream(bp, &in[inpos]); BTYPE += 2 * readBitFromStream(bp, &in[inpos]);
          if(BTYPE == 3) { error = 20; return; } //error: invalid BTYPE
          else if(BTYPE == 0) inflateNoCompression(out, &in[inpos], bp, pos, in.size());
          else inflateHuffmanBlock(out, &in[inpos], bp, pos, in.size(), BTYPE);
        }
        if(!error) out.resize(pos); //Only now we know the true size of out, resize it to that
      }
      void generateFixedTrees(HuffmanTree& tree, HuffmanTree& treeD) //get the tree of a deflated block with fixed tree
      {
        std::vector<unsigned long> bitlen(288, 8), bitlenD(32, 5);;
        for(size_t i = 144; i <= 255; i++) bitlen[i] = 9;
        for(size_t i = 256; i <= 279; i++) bitlen[i] = 7;
        tree.makeFromLengths(bitlen, 15);
        treeD.makeFromLengths(bitlenD, 15);
      }
      HuffmanTree codetree, codetreeD, codelengthcodetree; //the code tree for Huffman codes, dist codes, and code length codes
      unsigned long huffmanDecodeSymbol(const unsigned char* in, size_t& bp, const HuffmanTree& codetree, size_t inlength)
      { //decode a single symbol from given list of bits with given code tree. return value is the symbol
        bool decoded; unsigned long ct;
        for(size_t treepos = 0;;)
        {
          if((bp & 0x07) == 0 && (bp >> 3) > inlength) { error = 10; return 0; } //error: end reached without endcode
          error = codetree.decode(decoded, ct, treepos, readBitFromStream(bp, in)); if(error) return 0; //stop, an error happened
          if(decoded) return ct;
        }
      }
      void getTreeInflateDynamic(HuffmanTree& tree, HuffmanTree& treeD, const unsigned char* in, size_t& bp, size_t inlength)
      { //get the tree of a deflated block with dynamic tree, the tree itself is also Huffman compressed with a known tree
        std::vector<unsigned long> bitlen(288, 0), bitlenD(32, 0);
        if(bp >> 3 >= inlength - 2) { error = 49; return; } //the bit pointer is or will go past the memory
        size_t HLIT =  readBitsFromStream(bp, in, 5) + 257; //number of literal/length codes + 257
        size_t HDIST = readBitsFromStream(bp, in, 5) + 1; //number of dist codes + 1
        size_t HCLEN = readBitsFromStream(bp, in, 4) + 4; //number of code length codes + 4
        std::vector<unsigned long> codelengthcode(19); //lengths of tree to decode the lengths of the dynamic tree
        for(size_t i = 0; i < 19; i++) codelengthcode[CLCL[i]] = (i < HCLEN) ? readBitsFromStream(bp, in, 3) : 0;
        error = codelengthcodetree.makeFromLengths(codelengthcode, 7); if(error) return;
        size_t i = 0, replength;
        while(i < HLIT + HDIST)
        {
          unsigned long code = huffmanDecodeSymbol(in, bp, codelengthcodetree, inlength); if(error) return;
          if(code <= 15)  { if(i < HLIT) bitlen[i++] = code; else bitlenD[i++ - HLIT] = code; } //a length code
          else if(code == 16) //repeat previous
          {
            if(bp >> 3 >= inlength) { error = 50; return; } //error, bit pointer jumps past memory
            replength = 3 + readBitsFromStream(bp, in, 2);
            unsigned long value; //set value to the previous code
            if((i - 1) < HLIT) value = bitlen[i - 1];
            else value = bitlenD[i - HLIT - 1];
            for(size_t n = 0; n < replength; n++) //repeat this value in the next lengths
            {
              if(i >= HLIT + HDIST) { error = 13; return; } //error: i is larger than the amount of codes
              if(i < HLIT) bitlen[i++] = value; else bitlenD[i++ - HLIT] = value;
            }
          }
          else if(code == 17) //repeat "0" 3-10 times
          {
            if(bp >> 3 >= inlength) { error = 50; return; } //error, bit pointer jumps past memory
            replength = 3 + readBitsFromStream(bp, in, 3);
            for(size_t n = 0; n < replength; n++) //repeat this value in the next lengths
            {
              if(i >= HLIT + HDIST) { error = 14; return; } //error: i is larger than the amount of codes
              if(i < HLIT) bitlen[i++] = 0; else bitlenD[i++ - HLIT] = 0;
            }
          }
          else if(code == 18) //repeat "0" 11-138 times
          {
            if(bp >> 3 >= inlength) { error = 50; return; } //error, bit pointer jumps past memory
            replength = 11 + readBitsFromStream(bp, in, 7);
            for(size_t n = 0; n < replength; n++) //repeat this value in the next lengths
            {
              if(i >= HLIT + HDIST) { error = 15; return; } //error: i is larger than the amount of codes
              if(i < HLIT) bitlen[i++] = 0; else bitlenD[i++ - HLIT] = 0;
            }
          }
          else { error = 16; return; } //error: somehow an unexisting code appeared. This can never happen.
        }
        if(bitlen[256] == 0) { error = 64; return; } //the length of the end code 256 must be larger than 0
        error = tree.makeFromLengths(bitlen, 15); if(error) return; //now we've finally got HLIT and HDIST, so generate the code trees, and the function is done
        error = treeD.makeFromLengths(bitlenD, 15); if(error) return;
      }
      void inflateHuffmanBlock(std::vector<unsigned char>& out, const unsigned char* in, size_t& bp, size_t& pos, size_t inlength, unsigned long btype)
      {
        if(btype == 1) { generateFixedTrees(codetree, codetreeD); }
        else if(btype == 2) { getTreeInflateDynamic(codetree, codetreeD, in, bp, inlength); if(error) return; }
        for(;;)
        {
          unsigned long code = huffmanDecodeSymbol(in, bp, codetree, inlength); if(error) return;
          if(code == 256) return; //end code
          else if(code <= 255) //literal symbol
          {
            if(pos >= out.size()) out.resize((pos + 1) * 2); //reserve more room
            out[pos++] = (unsigned char)(code);
          }
          else if(code >= 257 && code <= 285) //length code
          {
            size_t length = LENBASE[code - 257], numextrabits = LENEXTRA[code - 257];
            if((bp >> 3) >= inlength) { error = 51; return; } //error, bit pointer will jump past memory
            length += readBitsFromStream(bp, in, numextrabits);
            unsigned long codeD = huffmanDecodeSymbol(in, bp, codetreeD, inlength); if(error) return;
            if(codeD > 29) { error = 18; return; } //error: invalid dist code (30-31 are never used)
            unsigned long dist = DISTBASE[codeD], numextrabitsD = DISTEXTRA[codeD];
            if((bp >> 3) >= inlength) { error = 51; return; } //error, bit pointer will jump past memory
            dist += readBitsFromStream(bp, in, numextrabitsD);
            size_t start = pos, back = start - dist; //backwards
            if(pos + length >= out.size()) out.resize((pos + length) * 2); //reserve more room
            for(size_t i = 0; i < length; i++) { out[pos++] = out[back++]; if(back >= start) back = start - dist; }
          }
        }
      }
      void inflateNoCompression(std::vector<unsigned char>& out, const unsigned char* in, size_t& bp, size_t& pos, size_t inlength)
      {
        while((bp & 0x7) != 0) bp++; //go to first boundary of byte
        size_t p = bp / 8;
        if(p >= inlength - 4) { error = 52; return; } //error, bit pointer will jump past memory
        unsigned long LEN = in[p] + 256 * in[p + 1], NLEN = in[p + 2] + 256 * in[p + 3]; p += 4;
        if(LEN + NLEN != 65535) { error = 21; return; } //error: NLEN is not one's complement of LEN
        if(pos + LEN >= out.size()) out.resize(pos + LEN);
        if(p + LEN > inlength) { error = 23; return; } //error: reading outside of in buffer
        for(unsigned long n = 0; n < LEN; n++) out[pos++] = in[p++]; //read LEN bytes of literal data
        bp = p * 8;
      }
    };
    int decompress(std::vector<unsigned char>& out, const std::vector<unsigned char>& in) //returns error value
    {
      Inflator inflator;
      if(in.size() < 2) { return 53; } //error, size of zlib data too small
      if((in[0] * 256 + in[1]) % 31 != 0) { return 24; } //error: 256 * in[0] + in[1] must be a multiple of 31, the FCHECK value is supposed to be made that way
      unsigned long CM = in[0] & 15, CINFO = (in[0] >> 4) & 15, FDICT = (in[1] >> 5) & 1;
      if(CM != 8 || CINFO > 7) { return 25; } //error: only compression method 8: inflate with sliding window of 32k is supported by the PNG spec
      if(FDICT != 0) { return 26; } //error: the specification of PNG says about the zlib stream: "The additional flags shall not specify a preset dictionary."
      inflator.inflate(out, in, 2);
      return inflator.error; //note: adler32 checksum was skipped and ignored
    }
  };
  struct PNG //nested functions for PNG decoding
  {
      PNGInfo info;
    int error;
    void decode(std::vector<unsigned char>& out, const unsigned char* in, size_t size)
    {
      error = 0;
      if(size == 0 || in == 0) { error = 48; return; } //the given data is empty
      readPngHeader(&in[0], size); if(error) return;
      size_t pos = 33; //first byte of the first chunk after the header
      std::vector<unsigned char> idat; //the data from idat chunks
      bool IEND = false, known_type = true;
      info.key_defined = false;
      while(!IEND) //loop through the chunks, ignoring unknown chunks and stopping at IEND chunk. IDAT data is put at the start of the in buffer
      {
        if(pos + 8 >= size) { error = 30; return; } //error: size of the in buffer too small to contain next chunk
        size_t chunkLength = read32bitInt(&in[pos]); pos += 4;
        if(chunkLength > 2147483647) { error = 63; return; }
        if(pos + chunkLength >= size) { error = 35; return; } //error: size of the in buffer too small to contain next chunk
        if(in[pos + 0] == 'I' && in[pos + 1] == 'D' && in[pos + 2] == 'A' && in[pos + 3] == 'T') //IDAT chunk, containing compressed image data
        {
          idat.insert(idat.end(), &in[pos + 4], &in[pos + 4 + chunkLength]);
          pos += (4 + chunkLength);
        }
        else if(in[pos + 0] == 'I' && in[pos + 1] == 'E' && in[pos + 2] == 'N' && in[pos + 3] == 'D')  { pos += 4; IEND = true; }
        else if(in[pos + 0] == 'P' && in[pos + 1] == 'L' && in[pos + 2] == 'T' && in[pos + 3] == 'E') //palette chunk (PLTE)
        {
          pos += 4; //go after the 4 letters
          info.palette.resize(4 * (chunkLength / 3));
          if(info.palette.size() > (4 * 256)) { error = 38; return; } //error: palette too big
          for(size_t i = 0; i < info.palette.size(); i += 4)
          {
            for(size_t j = 0; j < 3; j++) info.palette[i + j] = in[pos++]; //RGB
            info.palette[i + 3] = 255; //alpha
          }
        }
        else if(in[pos + 0] == 't' && in[pos + 1] == 'R' && in[pos + 2] == 'N' && in[pos + 3] == 'S') //palette transparency chunk (tRNS)
        {
          pos += 4; //go after the 4 letters
          if(info.colorType == 3)
          {
            if(4 * chunkLength > info.palette.size()) { error = 39; return; } //error: more alpha values given than there are palette entries
            for(size_t i = 0; i < chunkLength; i++) info.palette[4 * i + 3] = in[pos++];
          }
          else if(info.colorType == 0)
          {
            if(chunkLength != 2) { error = 40; return; } //error: this chunk must be 2 bytes for greyscale image
            info.key_defined = 1; info.key_r = info.key_g = info.key_b = 256 * in[pos] + in[pos + 1]; pos += 2;
          }
          else if(info.colorType == 2)
          {
            if(chunkLength != 6) { error = 41; return; } //error: this chunk must be 6 bytes for RGB image
            info.key_defined = 1;
            info.key_r = 256 * in[pos] + in[pos + 1]; pos += 2;
            info.key_g = 256 * in[pos] + in[pos + 1]; pos += 2;
            info.key_b = 256 * in[pos] + in[pos + 1]; pos += 2;
          }
          else { error = 42; return; } //error: tRNS chunk not allowed for other color models
        }
        else //it's not an implemented chunk type, so ignore it: skip over the data
        {
          if(!(in[pos + 0] & 32)) { error = 69; return; } //error: unknown critical chunk (5th bit of first byte of chunk type is 0)
          pos += (chunkLength + 4); //skip 4 letters and uninterpreted data of unimplemented chunk
          known_type = false;
        }
        pos += 4; //step over CRC (which is ignored)
      }
      unsigned long bpp = getBpp(info);
      std::vector<unsigned char> scanlines(((info.width * (info.height * bpp + 7)) / 8) + info.height); //now the out buffer will be filled
      Zlib zlib; //decompress with the Zlib decompressor
      error = zlib.decompress(scanlines, idat); if(error) return; //stop if the zlib decompressor returned an error
      size_t bytewidth = (bpp + 7) / 8, outlength = (info.height * info.width * bpp + 7) / 8;
      out.resize(outlength); //time to fill the out buffer
      unsigned char* out_ = outlength ? &out[0] : 0; //use a regular pointer to the std::vector for faster code if compiled without optimization
      if(info.interlaceMethod == 0) //no interlace, just filter
      {
        size_t linestart = 0, linelength = (info.width * bpp + 7) / 8; //length in bytes of a scanline, excluding the filtertype byte
        if(bpp >= 8) //byte per byte
        for(unsigned long y = 0; y < info.height; y++)
        {
          unsigned long filterType = scanlines[linestart];
          const unsigned char* prevline = (y == 0) ? 0 : &out_[(y - 1) * info.width * bytewidth];
          unFilterScanline(&out_[linestart - y], &scanlines[linestart + 1], prevline, bytewidth, filterType,  linelength); if(error) return;
          linestart += (1 + linelength); //go to start of next scanline
        }
        else //less than 8 bits per pixel, so fill it up bit per bit
        {
          std::vector<unsigned char> templine((info.width * bpp + 7) >> 3); //only used if bpp < 8
          for(size_t y = 0, obp = 0; y < info.height; y++)
          {
            unsigned long filterType = scanlines[linestart];
            const unsigned char* prevline = (y == 0) ? 0 : &out_[(y - 1) * info.width * bytewidth];
            unFilterScanline(&templine[0], &scanlines[linestart + 1], prevline, bytewidth, filterType, linelength); if(error) return;
            for(size_t bp = 0; bp < info.width * bpp;) setBitOfReversedStream(obp, out_, readBitFromReversedStream(bp, &templine[0]));
            linestart += (1 + linelength); //go to start of next scanline
          }
        }
      }
      else //interlaceMethod is 1 (Adam7)
      {
        size_t passw[7] = { (info.width + 7) / 8, (info.width + 3) / 8, (info.width + 3) / 4, (info.width + 1) / 4, (info.width + 1) / 2, (info.width + 0) / 2, (info.width + 0) / 1 };
        size_t passh[7] = { (info.height + 7) / 8, (info.height + 7) / 8, (info.height + 3) / 8, (info.height + 3) / 4, (info.height + 1) / 4, (info.height + 1) / 2, (info.height + 0) / 2 };
        size_t passstart[7] = {0};
        size_t pattern[28] = {0,4,0,2,0,1,0,0,0,4,0,2,0,1,8,8,4,4,2,2,1,8,8,8,4,4,2,2}; //values for the adam7 passes
        for(int i = 0; i < 6; i++) passstart[i + 1] = passstart[i] + passh[i] * ((passw[i] ? 1 : 0) + (passw[i] * bpp + 7) / 8);
        std::vector<unsigned char> scanlineo((info.width * bpp + 7) / 8), scanlinen((info.width * bpp + 7) / 8); //"old" and "new" scanline
        for(int i = 0; i < 7; i++)
          adam7Pass(&out_[0], &scanlinen[0], &scanlineo[0], &scanlines[passstart[i]], info.width, pattern[i], pattern[i + 7], pattern[i + 14], pattern[i + 21], passw[i], passh[i], bpp);
      }
    }
    void readPngHeader(const unsigned char* in, size_t inlength) //read the information from the header and store it in the Info
    {
      if(inlength < 29) { error = 27; return; } //error: the data length is smaller than the length of the header
      if(in[0] != 137 || in[1] != 80 || in[2] != 78 || in[3] != 71 || in[4] != 13 || in[5] != 10 || in[6] != 26 || in[7] != 10) { error = 28; return; } //no PNG signature
      if(in[12] != 'I' || in[13] != 'H' || in[14] != 'D' || in[15] != 'R') { error = 29; return; } //error: it doesn't start with a IHDR chunk!
      info.width = read32bitInt(&in[16]); info.height = read32bitInt(&in[20]);
      info.bitDepth = in[24]; info.colorType = in[25];
      info.compressionMethod = in[26]; if(in[26] != 0) { error = 32; return; } //error: only compression method 0 is allowed in the specification
      info.filterMethod = in[27]; if(in[27] != 0) { error = 33; return; } //error: only filter method 0 is allowed in the specification
      info.interlaceMethod = in[28]; if(in[28] > 1) { error = 34; return; } //error: only interlace methods 0 and 1 exist in the specification
      error = checkColorValidity(info.colorType, info.bitDepth);
    }
    void unFilterScanline(unsigned char* recon, const unsigned char* scanline, const unsigned char* precon, size_t bytewidth, unsigned long filterType, size_t length)
    {
      switch(filterType)
      {
        case 0: for(size_t i = 0; i < length; i++) recon[i] = scanline[i]; break;
        case 1:
          for(size_t i =         0; i < bytewidth; i++) recon[i] = scanline[i];
          for(size_t i = bytewidth; i <    length; i++) recon[i] = scanline[i] + recon[i - bytewidth];
          break;
        case 2:
          if(precon) for(size_t i = 0; i < length; i++) recon[i] = scanline[i] + precon[i];
          else       for(size_t i = 0; i < length; i++) recon[i] = scanline[i];
          break;
        case 3:
          if(precon)
          {
            for(size_t i =         0; i < bytewidth; i++) recon[i] = scanline[i] + precon[i] / 2;
            for(size_t i = bytewidth; i <    length; i++) recon[i] = scanline[i] + ((recon[i - bytewidth] + precon[i]) / 2);
          }
          else
          {
            for(size_t i =         0; i < bytewidth; i++) recon[i] = scanline[i];
            for(size_t i = bytewidth; i <    length; i++) recon[i] = scanline[i] + recon[i - bytewidth] / 2;
          }
          break;
        case 4:
          if(precon)
          {
            for(size_t i =         0; i < bytewidth; i++) recon[i] = scanline[i] + paethPredictor(0, precon[i], 0);
            for(size_t i = bytewidth; i <    length; i++) recon[i] = scanline[i] + paethPredictor(recon[i - bytewidth], precon[i], precon[i - bytewidth]);
          }
          else
          {
            for(size_t i =         0; i < bytewidth; i++) recon[i] = scanline[i];
            for(size_t i = bytewidth; i <    length; i++) recon[i] = scanline[i] + paethPredictor(recon[i - bytewidth], 0, 0);
          }
          break;
        default: error = 36; return; //error: unexisting filter type given
      }
    }
    void adam7Pass(unsigned char* out, unsigned char* linen, unsigned char* lineo, const unsigned char* in, unsigned long w, size_t passleft, size_t passtop, size_t spacex, size_t spacey, size_t passw, size_t passh, unsigned long bpp)
    { //filter and reposition the pixels into the output when the image is Adam7 interlaced. This function can only do it after the full image is already decoded. The out buffer must have the correct allocated memory size already.
      if(passw == 0) return;
      size_t bytewidth = (bpp + 7) / 8, linelength = 1 + ((bpp * passw + 7) / 8);
      for(unsigned long y = 0; y < passh; y++)
      {
        unsigned char filterType = in[y * linelength], *prevline = (y == 0) ? 0 : lineo;
        unFilterScanline(linen, &in[y * linelength + 1], prevline, bytewidth, filterType, (w * bpp + 7) / 8); if(error) return;
        if(bpp >= 8) for(size_t i = 0; i < passw; i++) for(size_t b = 0; b < bytewidth; b++) //b = current byte of this pixel
          out[bytewidth * w * (passtop + spacey * y) + bytewidth * (passleft + spacex * i) + b] = linen[bytewidth * i + b];
        else for(size_t i = 0; i < passw; i++)
        {
          size_t obp = bpp * w * (passtop + spacey * y) + bpp * (passleft + spacex * i), bp = i * bpp;
          for(size_t b = 0; b < bpp; b++) setBitOfReversedStream(obp, out, readBitFromReversedStream(bp, &linen[0]));
        }
        unsigned char* temp = linen; linen = lineo; lineo = temp; //swap the two buffer pointers "line old" and "line new"
      }
    }
    static unsigned long readBitFromReversedStream(size_t& bitp, const unsigned char* bits) { unsigned long result = (bits[bitp >> 3] >> (7 - (bitp & 0x7))) & 1; bitp++; return result;}
    static unsigned long readBitsFromReversedStream(size_t& bitp, const unsigned char* bits, unsigned long nbits)
    {
      unsigned long result = 0;
      for(size_t i = nbits - 1; i < nbits; i--) result += ((readBitFromReversedStream(bitp, bits)) << i);
      return result;
    }
    void setBitOfReversedStream(size_t& bitp, unsigned char* bits, unsigned long bit) { bits[bitp >> 3] |=  (bit << (7 - (bitp & 0x7))); bitp++; }
    unsigned long read32bitInt(const unsigned char* buffer) { return (buffer[0] << 24) | (buffer[1] << 16) | (buffer[2] << 8) | buffer[3]; }
    int checkColorValidity(unsigned long colorType, unsigned long bd) //return type is a LodePNG error code
    {
      if((colorType == 2 || colorType == 4 || colorType == 6)) { if(!(bd == 8 || bd == 16)) return 37; else return 0; }
      else if(colorType == 0) { if(!(bd == 1 || bd == 2 || bd == 4 || bd == 8 || bd == 16)) return 37; else return 0; }
      else if(colorType == 3) { if(!(bd == 1 || bd == 2 || bd == 4 || bd == 8            )) return 37; else return 0; }
      else return 31; //unexisting color type
    }
    unsigned long getBpp(const PNGInfo& info)
    {
      if(info.colorType == 2) return (3 * info.bitDepth);
      else if(info.colorType >= 4) return (info.colorType - 2) * info.bitDepth;
      else return info.bitDepth;
    }

    unsigned char paethPredictor(short a, short b, short c) //Paeth predicter, used by PNG filter type 4
    {
      short p = a + b - c, pa = p > a ? (p - a) : (a - p), pb = p > b ? (p - b) : (b - p), pc = p > c ? (p - c) : (c - p);
      return (unsigned char)((pa <= pb && pa <= pc) ? a : pb <= pc ? b : c);
    }
  };

  PNG decoder; decoder.decode(out_image, in_png, in_size);
  out_info = decoder.info;
  return decoder.error;
}




Image::Image()
{
    width=0;
    height=0;
    mode=0;
    data=NULL;
    texture=0;
}

void Image::kill()
{
    if (data) delete []data;
}

void Image::read(File &file)
{
    /*const int channelmap[]= { 0, ALPHA, ALPHA, RGB, RGBA };

    int *header = (int*)file.contents;

    width = header[0];
    height = header[1];
    mode = channelmap[header[2]];

    if (width*height*mode>file.length-6) throw(1);

    data = new unsigned char[width*height*mode];
    memcpy(data,file.contents+6,width*height*mode);*/

    std::vector<unsigned char> image;
    PNGInfo info;

    decodePNG(image,info,file.contents,file.length);

    width = info.width;
    height = info.height;

    //Log::log() << "width: " << info.width << " height: " << info.height << " color type: " << info.colorType << "<br>";

    switch (info.colorType)
    {
    case 2: mode = RGB; break;
    case 6: mode = RGBA; break;
    default: throw(1);
    }

    data = new unsigned char[info.width*info.height*(mode==RGBA?4:3)];
    memcpy(data,&image[0],info.width*info.height*(mode==RGBA?4:3));
}

void Image::link()
{
    static const int channel0[]={0,GL_RED,0,GL_RGB,GL_RGBA,GL_DEPTH_COMPONENT32};
    static const int channel1[]={0,GL_RED,0,GL_RGB,GL_RGBA,GL_DEPTH_COMPONENT};

    unsigned char *t;
    int i,r,g,b;

    glGenTextures(1,(unsigned int*)&texture);
    glBindTexture(GL_TEXTURE_2D,texture);

    if(mode<Image::KEYED)
    {
        glTexImage2D(GL_TEXTURE_2D,0,channel0[mode],width,height,0,channel1[mode],GL_UNSIGNED_BYTE,data);
    }
    else if (mode==Image::KEYED)
    {
        t=new unsigned char[width*height*4];

        for(i=0;i<height*width;i++)
        {
            r=data[i*3+0];
            g=data[i*3+1];
            b=data[i*3+2];
            if(r>=254&&g<=1&&b>=254)
            {
                t[i*4+0]=0;
                t[i*4+1]=0;
                t[i*4+2]=0;
                t[i*4+3]=0;
            }
            else
            {
                t[i*4+0]=r;
                t[i*4+1]=g;
                t[i*4+2]=b;
                t[i*4+3]=255;
            }
        }

        glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,width,height,0,GL_RGBA,GL_UNSIGNED_BYTE,t);

        delete []t;

    }

    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);


}

void Image::unlink()
{
    glDeleteTextures(1,(unsigned int*)&texture);
}

void Image::use(int unit) const
{
    glActiveTexture(GL_TEXTURE0+unit);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D,texture);
}


