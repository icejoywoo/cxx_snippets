#pragma once

#include <string>

// ported from airlift compressor Lz4 java implementation
namespace my::lz4 {
// Lz4Constants
int LAST_LITERAL_SIZE = 5;
int MIN_MATCH = 4;

int SIZE_OF_SHORT = 2;
int SIZE_OF_INT = 4;
int SIZE_OF_LONG = 8;

namespace compressor {
// Lz4RawCompressor
int MAX_INPUT_SIZE = 0x7E000000;
int HASH_LOG = 12;
int MIN_TABLE_SIZE = 16;
int MAX_TABLE_SIZE = (1 << HASH_LOG);
int COPY_LENGTH = 8;
int MATCH_FIND_LIMIT = COPY_LENGTH + MIN_MATCH;

int MIN_LENGTH = MATCH_FIND_LIMIT + 1;

int ML_BITS = 4;
int ML_MASK = (1 << ML_BITS) - 1;
int RUN_BITS = 8 - ML_BITS;
int RUN_MASK = (1 << RUN_BITS) - 1;

int MAX_DISTANCE = ((1 << 16) - 1);

int SKIP_TRIGGER = 6;  /* Increase this value ==> compression run slower on incompressible data */

uint32_t hash(uint64_t value, uint32_t mask)
{
    // Multiplicative hash. It performs the equivalent to
    // this computation:
    //
    // value * frac(a)
    //
    // for some real number 'a' with a good & random mix
    // of 1s and 0s in its binary representation
    //
    // For performance, it does it using fixed point math
    return (int) ((value * 889523592379LL >> 28) & mask);
}

long encodeRunLength(
        uint8_t* base,
        long output,
        long length)
{
    if (length >= RUN_MASK) {
        base[output++] = (RUN_MASK << ML_BITS);

        long remaining = length - RUN_MASK;
        while (remaining >= 255) {
            base[output++] = 255;
            remaining -= 255;
        }
        base[output++] = remaining;
    }
    else {
        base[output++] = length << ML_BITS;
    }

    return output;
}

uint32_t highestOneBit(uint32_t n) {
    n |= (n >>  1);
    n |= (n >>  2);
    n |= (n >>  4);
    n |= (n >>  8);
    n |= (n >> 16);
    return n - (n >> 1);
}

uint32_t numberOfTrailingZeros(uint64_t i) {
    // HD, Figure 5-14
    uint32_t x, y;
    if (i == 0) return 64;
    uint32_t n = 63;
    y = (uint32_t)i; if (y != 0) { n = n -32; x = y; } else x = (uint32_t)(i>>32);
    y = x <<16; if (y != 0) { n = n -16; x = y; }
    y = x << 8; if (y != 0) { n = n - 8; x = y; }
    y = x << 4; if (y != 0) { n = n - 4; x = y; }
    y = x << 2; if (y != 0) { n = n - 2; x = y; }
    return n - ((x << 1) >> 31);
}

int computeTableSize(uint32_t inputSize)
{
    // smallest power of 2 larger than inputSize
    int target = highestOneBit(inputSize - 1) << 1;

    // keep it between MIN_TABLE_SIZE and MAX_TABLE_SIZE
    return std::max(std::min(target, MAX_TABLE_SIZE), MIN_TABLE_SIZE);
}

long emitLastLiteral(
        uint8_t* outputBase,
        long outputAddress,
        uint8_t* inputBase,
        long inputAddress,
        long length)
{
    long output = encodeRunLength(outputBase, outputAddress, length);
    memcpy(&outputBase[output], &inputBase[inputAddress], length);

    return output + length;
}

int count(uint8_t* inputBase, long start, long matchStart, long matchLimit)
{
    long current = start;

    // first, compare long at a time
    while (current < matchLimit - (SIZE_OF_LONG - 1)) {
        uint64_t diff = *reinterpret_cast<uint64_t*>(&inputBase[matchStart]) ^ *reinterpret_cast<uint64_t*>(&inputBase[current]);
        if (diff != 0) {
            current += numberOfTrailingZeros(diff) >> 3;
            return (int) (current - start);
        }

        current += SIZE_OF_LONG;
        matchStart += SIZE_OF_LONG;
    }

    if (current < matchLimit - (SIZE_OF_INT - 1) && *reinterpret_cast<uint32_t*>(&inputBase[matchStart]) == *reinterpret_cast<uint32_t*>(&inputBase[current])) {
        current += SIZE_OF_INT;
        matchStart += SIZE_OF_INT;
    }

    if (current < matchLimit - (SIZE_OF_SHORT - 1) && *reinterpret_cast<uint16_t*>(&inputBase[matchStart]) == *reinterpret_cast<uint16_t*>(&inputBase[current])) {
        current += SIZE_OF_SHORT;
        matchStart += SIZE_OF_SHORT;
    }

    if (current < matchLimit && inputBase[matchStart] == inputBase[current]) {
        ++current;
    }

    return (int) (current - start);
}

long emitMatch(uint8_t* outputBase, long output, long tokenAddress, short offset, long matchLength)
{
    // write offset
    *reinterpret_cast<uint16_t*>(&outputBase[output]) = offset;
    output += SIZE_OF_SHORT;

    // write match length
    if (matchLength >= ML_MASK) {
        outputBase[tokenAddress] = (outputBase[tokenAddress] | ML_MASK);
        long remaining = matchLength - ML_MASK;
        while (remaining >= 510) {
            *reinterpret_cast<uint16_t*>(&outputBase[output]) = 0xFFFF;
            output += SIZE_OF_SHORT;
            remaining -= 510;
        }
        if (remaining >= 255) {
            outputBase[output++] = 255;
            remaining -= 255;
        }
        outputBase[output++] = remaining;
    }
    else {
        outputBase[tokenAddress] = (outputBase[tokenAddress] | matchLength);
    }

    return output;
}

long emitLiteral(uint8_t* inputBase, uint8_t* outputBase, long input, int literalLength, long output)
{
    output = encodeRunLength(outputBase, output, literalLength);

    long outputLimit = output + literalLength;
    do {
        *reinterpret_cast<uint64_t*>(&outputBase[output]) = *reinterpret_cast<uint64_t*>(&inputBase[input]);
        input += SIZE_OF_LONG;
        output += SIZE_OF_LONG;
    }
    while (output < outputLimit);

    return outputLimit;
}

uint32_t maxCompressedLength(uint32_t sourceLength)
{
    return sourceLength + sourceLength / 255 + 16;
}

int compress(
        uint8_t* inputBase,
        long inputAddress,
        uint32_t inputLength,
        uint8_t* outputBase,
        long outputAddress,
        uint32_t maxOutputLength,
        int* table)
{
    int tableSize = computeTableSize(inputLength);
    memset(table, 0, tableSize);

    int mask = tableSize - 1;

    if (inputLength > MAX_INPUT_SIZE) {
        throw std::runtime_error("Max input length exceeded");
    }

    if (maxOutputLength < maxCompressedLength(inputLength)) {
        throw std::runtime_error("Max output length must be larger than " + std::to_string(maxCompressedLength(inputLength)));
    }

    long input = inputAddress;
    long output = outputAddress;

    long inputLimit = inputAddress + inputLength;
    long matchFindLimit = inputLimit - MATCH_FIND_LIMIT;
    long matchLimit = inputLimit - LAST_LITERAL_SIZE;

    if (inputLength < MIN_LENGTH) {
        output = emitLastLiteral(outputBase, output, inputBase, input, inputLimit - input);
        return (int) (output - outputAddress);
    }

    long anchor = input;

    // First Byte
    // put position in hash
    table[hash(*reinterpret_cast<long*>(&inputBase[input]), mask)] = (int) (input - inputAddress);

    input++;
    uint32_t nextHash = hash(*reinterpret_cast<long*>(&inputBase[input]), mask);

    bool done = false;
    do {
        long nextInputIndex = input;
        uint32_t findMatchAttempts = 1 << SKIP_TRIGGER;
        uint32_t step = 1;

        // find 4-byte match
        long matchIndex;
        do {
            uint32_t _hash = nextHash;
            input = nextInputIndex;
            nextInputIndex += step;

            step = (findMatchAttempts++) >> SKIP_TRIGGER;

            if (nextInputIndex > matchFindLimit) {
                return (int) (emitLastLiteral(outputBase, output, inputBase, anchor, inputLimit - anchor) - outputAddress);
            }

            // get position on hash
            matchIndex = inputAddress + table[_hash];
            nextHash = hash(*reinterpret_cast<long*>(&inputBase[nextInputIndex]), mask);

            // put position on hash
            table[_hash] = (int) (input - inputAddress);
        }
        while (*reinterpret_cast<uint32_t*>(&inputBase[matchIndex]) != *reinterpret_cast<uint32_t*>(&inputBase[input]) || matchIndex + MAX_DISTANCE < input);

        // catch up
        while ((input > anchor) && (matchIndex > inputAddress) && (inputBase[input - 1] == inputBase[matchIndex - 1])) {
            --input;
            --matchIndex;
        }

        int literalLength = (int) (input - anchor);
        long tokenAddress = output;

        output = emitLiteral(inputBase, outputBase, anchor, literalLength, tokenAddress);

        // next match
        while (true) {
            // find match length
            int matchLength = count(inputBase, input + MIN_MATCH, matchIndex + MIN_MATCH, matchLimit);
            output = emitMatch(outputBase, output, tokenAddress, (short) (input - matchIndex), matchLength);

            input += matchLength + MIN_MATCH;

            anchor = input;

            // are we done?
            if (input > matchFindLimit) {
                done = true;
                break;
            }

            long position = input - 2;
            table[hash(*reinterpret_cast<long*>(&inputBase[position]), mask)] = (int) (position - inputAddress);

            // Test next position
            uint32_t _hash = hash(*reinterpret_cast<long*>(&inputBase[input]), mask);
            matchIndex = inputAddress + table[_hash];
            table[_hash] = (int) (input - inputAddress);

            if (matchIndex + MAX_DISTANCE < input || *reinterpret_cast<uint32_t*>(&inputBase[matchIndex]) != *reinterpret_cast<uint32_t*>(&inputBase[input])) {
                input++;
                nextHash = hash(*reinterpret_cast<uint32_t*>(&inputBase[input]), mask);
                break;
            }

            // go for another match
            tokenAddress = output++;
            outputBase[tokenAddress] = 0;
        }
    }
    while (!done);

    // Encode Last Literals
    output = emitLastLiteral(outputBase, output, inputBase, anchor, inputLimit - anchor);

    return (int) (output - outputAddress);
}

// Lz4Compressor
int compress(uint8_t* input, int inputOffset, uint32_t inputLength, uint8_t* output, int outputOffset, uint32_t maxOutputLength)
{
    int* table = new int[MAX_TABLE_SIZE];

    return compress(input, inputOffset, inputLength, output, outputOffset, maxOutputLength, table);
}
} // namespace compressor
namespace decompressor {
// Lz4RawDecompressor
int DEC_32_TABLE[]{4, 1, 2, 1, 4, 4, 4, 4};
int DEC_64_TABLE[]{0, 0, 0, -1, 0, 1, 2, 3};

int OFFSET_SIZE = 2;
int TOKEN_SIZE = 1;

int decompress(
        uint8_t* inputBase,
        long inputAddress,
        long inputLimit,
        uint8_t* outputBase,
        long outputAddress,
        long outputLimit)
{
    long fastOutputLimit = outputLimit - SIZE_OF_LONG; // maximum offset in output buffer to which it's safe to write long-at-a-time

    long input = inputAddress;
    long output = outputAddress;

    if (inputAddress == inputLimit) {
        throw std::runtime_error("input is empty");
    }

    if (outputAddress == outputLimit) {
        if (inputLimit - inputAddress == 1 && inputBase[inputAddress] == 0) {
            return 0;
        }
        return -1;
    }

    while (input < inputLimit) {
        uint32_t token = inputBase[input++] & 0xFF;

        // decode literal length
        uint32_t literalLength = token >> 4; // top-most 4 bits of token
        if (literalLength == 0xF) {
            int value;
            do {
                value = inputBase[input++] & 0xFF;
                literalLength += value;
            }
            while (value == 255 && input < inputLimit - 15);
        }

        // copy literal
        long literalEnd = input + literalLength;
        long literalOutputLimit = output + literalLength;
        if (literalOutputLimit > (fastOutputLimit - MIN_MATCH) || literalEnd > inputLimit - (OFFSET_SIZE + TOKEN_SIZE + LAST_LITERAL_SIZE)) {
            // copy the last literal and finish
            if (literalOutputLimit > outputLimit) {
                throw std::runtime_error("attempt to write last literal outside of destination buffer");
            }

            if (literalEnd != inputLimit) {
                throw std::runtime_error("all input must be consumed");
            }

            // slow, precise copy
            memcpy(&outputBase[output], &inputBase[input], literalLength);
            output += literalLength;
            break;
        }

        // fast copy. We may overcopy but there's enough room in input and output to not overrun them
        int index = 0;
        do {
            *reinterpret_cast<uint64_t*>(&outputBase[output]) = *reinterpret_cast<uint64_t*>(&inputBase[input]);
            output += SIZE_OF_LONG;
            input += SIZE_OF_LONG;
            index += SIZE_OF_LONG;
        }
        while (index < literalLength);
        output = literalOutputLimit;

        input = literalEnd;

        // get offset
        // we know we can read two bytes because of the bounds check performed before copying the literal above
        int offset = *reinterpret_cast<uint16_t*>(&inputBase[input]) & 0xFFFF;
        input += SIZE_OF_SHORT;

        long matchAddress = output - offset;
        if (matchAddress < outputAddress) {
            throw std::runtime_error("offset outside destination buffer");
        }

        // compute match length
        int matchLength = token & 0xF; // bottom-most 4 bits of token
        if (matchLength == 0xF) {
            int value;
            do {
                if (input > inputLimit - LAST_LITERAL_SIZE) {
                    throw std::runtime_error("Malformed input: offset=" + std::to_string(input - inputAddress));
                }

                value = inputBase[input++] & 0xFF;
                matchLength += value;
            }
            while (value == 255);
        }
        matchLength += MIN_MATCH; // implicit length from initial 4-byte match in encoder

        long matchOutputLimit = output + matchLength;

        // at this point we have at least 12 bytes of space in the output buffer
        // due to the fastLimit check before copying a literal, so no need to check again

        // copy repeated sequence
        if (offset < SIZE_OF_LONG) {
            // 8 bytes apart so that we can copy long-at-a-time below
            int increment32 = DEC_32_TABLE[offset];
            int decrement64 = DEC_64_TABLE[offset];

            outputBase[output] = outputBase[matchAddress];
            outputBase[output + 1] = outputBase[matchAddress + 1];
            outputBase[output + 2] = outputBase[matchAddress + 2];
            outputBase[output + 3] = outputBase[matchAddress + 3];
            output += SIZE_OF_INT;
            matchAddress += increment32;

            *reinterpret_cast<uint32_t*>(&outputBase[output]) = *reinterpret_cast<uint32_t*>(&outputBase[matchAddress]);
            output += SIZE_OF_INT;
            matchAddress -= decrement64;
        }
        else {
            *reinterpret_cast<uint64_t*>(&outputBase[output]) = *reinterpret_cast<uint64_t*>(&outputBase[matchAddress]);
            matchAddress += SIZE_OF_LONG;
            output += SIZE_OF_LONG;
        }

        if (matchOutputLimit > fastOutputLimit - MIN_MATCH) {
            if (matchOutputLimit > outputLimit - LAST_LITERAL_SIZE) {
                throw std::runtime_error("last" + std::to_string(LAST_LITERAL_SIZE) + " bytes must be literals: "
                                                                                      "offset=" + std::to_string(input - inputAddress));
            }

            while (output < fastOutputLimit) {
                *reinterpret_cast<uint64_t*>(&outputBase[output]) = *reinterpret_cast<uint64_t*>(&outputBase[matchAddress]);
                matchAddress += SIZE_OF_LONG;
                output += SIZE_OF_LONG;
            }

            while (output < matchOutputLimit) {
                outputBase[output++] = outputBase[matchAddress++];
            }
        }
        else {
            int i = 0;
            do {
                *reinterpret_cast<uint64_t*>(&outputBase[output]) = *reinterpret_cast<uint64_t*>(&outputBase[matchAddress]);
                output += SIZE_OF_LONG;
                matchAddress += SIZE_OF_LONG;
                i += SIZE_OF_LONG;
            }
            while (i < matchLength - SIZE_OF_LONG); // first long copied previously
        }

        output = matchOutputLimit; // correction in case we overcopied
    }

    return (int) (output - outputAddress);
}
} // decompressor
} // my::lz4