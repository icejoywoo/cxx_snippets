package com.icejoywoo;

import io.airlift.compress.Compressor;
import io.airlift.compress.Decompressor;
import io.airlift.compress.lz4.Lz4Compressor;
import io.airlift.compress.lz4.Lz4Decompressor;
import io.airlift.compress.snappy.SnappyCompressor;
import io.airlift.compress.snappy.SnappyDecompressor;
import junit.framework.Test;
import junit.framework.TestCase;
import junit.framework.TestSuite;


public class AirliftLz4Test 
    extends TestCase
{
    public void testLz4() {
        String test = "abcde_bcdefgh_abcdefghxxxxxxx";
        byte[] compressionBuffer;
        int actualCompressedLength;
        {
            Compressor compressor = new Lz4Compressor();
            int maxCompressedLength = compressor.maxCompressedLength(test.length());
            compressionBuffer = new byte[maxCompressedLength];
            actualCompressedLength = compressor.compress(
                test.getBytes(), 0, test.length(),
                compressionBuffer, 0, maxCompressedLength);
            String compressedString = new String(compressionBuffer, 0, actualCompressedLength);
            System.out.println(hexlify(compressionBuffer, actualCompressedLength));
            checkState(compressedString.length() == actualCompressedLength);
        }

        {
            compressionBuffer = unhexlify("e161626364655f626364656667685f0e00a066676878787878787878");
            Decompressor decompressor = new Lz4Decompressor();
            byte[] uncompressed = new byte[test.length()];
            int actualUncompressedSize = decompressor.decompress(
                compressionBuffer, 0, actualCompressedLength,
                uncompressed, 0, test.length());
            checkState(test.length() == actualUncompressedSize);
            System.out.println(new String(uncompressed));
        }

        {
            compressionBuffer = unhexlify("6061626364655f0500416667685f0e00a066676878787878787878");
            Decompressor decompressor = new Lz4Decompressor();
            byte[] uncompressed = new byte[test.length()];
            int actualUncompressedSize = decompressor.decompress(
                    compressionBuffer, 0, compressionBuffer.length,
                    uncompressed, 0, test.length());
            checkState(test.length() == actualUncompressedSize);
            System.out.println(new String(uncompressed));
        }
    }


    // https://raw.githubusercontent.com/slisznia/javsy/master/src/main/java/com/pushcoin/lib/javsy/Binascii.java
    private static final char charGlyph_[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };
    public static String hexlify(byte[] bytes, int size)
    {
        StringBuilder hexAscii = new StringBuilder(bytes.length * 2);

        for (int i=0; i < size; ++i)
        {
            byte b = bytes[i];
            hexAscii.append( charGlyph_[ (int)(b & 0xf0) >> 4] );
            hexAscii.append( charGlyph_[ (int)(b & 0x0f)] );
        }
        return hexAscii.toString();
    }
    public static String hexlify(byte[] bytes)
    {
        return hexlify(bytes, bytes.length);
    }

    public static byte[] unhexlify(String asciiHex)
    {
        if(asciiHex.length()%2 != 0) {
            throw new RuntimeException( "Input to unhexlify must have even-length");
        }

        int len = asciiHex.length();
        byte[] data = new byte[len / 2];
        for (int i = 0; i < len; i += 2)
        {
            data[i / 2] = (byte) ((Character.digit(asciiHex.charAt(i), 16) << 4) +
                    Character.digit(asciiHex.charAt(i+1), 16));
        }
        return data;
    }

    public static void checkState(boolean expression) {
        if (!expression) {
            throw new IllegalStateException();
        }
    }
}
