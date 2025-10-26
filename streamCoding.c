#include <stddef.h>
#include <string.h>

#include "streamCoding.h"


static inline uint32_t ZigZag_Encode32(int32_t i32Value)
{
    return (2 * i32Value) ^ (i32Value >> (sizeof(int32_t) * 8 - 1));
}

static inline int32_t ZigZag_Decode32(uint32_t u32Encoded)
{
    return (u32Encoded >> 1) ^ (-(u32Encoded & 1));
}

static uint32_t Varint_EncodeUnsigned(uint32_t u32Value, uint8_t *pu8OutputBuffer)
{
    uint32_t u32ByteIndex = 0;

    do
    {
        uint8_t u8Byte = u32Value & 0x7F;
        u32Value >>= 7;

        if (u32Value != 0) 
        {
            u8Byte |= 0x80;
        }

        pu8OutputBuffer[u32ByteIndex++] = u8Byte;

    } while (u32Value != 0);

    return u32ByteIndex;
}

static uint32_t Varint_DecodeUnsigned(const uint8_t *pu8InputBuffer, uint32_t *pu32DecodedValue)
{
    uint32_t u32Result = 0;
    uint32_t u32Shift = 0;
    uint32_t u32Index = 0;

    for (; u32Index < 5; u32Index++) 
    {
        uint8_t u8Byte = pu8InputBuffer[u32Index];
        u32Result |= (uint32_t)(u8Byte & 0x7F) << u32Shift;

        if ((u8Byte & 0x80) == 0) 
        {
            *pu32DecodedValue = u32Result;
            u32Index += 1;
            break;
        }

        u32Shift += 7;
    }

    return u32Index;
}

static uint32_t Varint_EncodeSigned(int32_t i32Value, uint8_t *pu8OutputBuffer)
{
    uint32_t u32ZigZagged = ZigZag_Encode32(i32Value);
    return Varint_EncodeUnsigned(u32ZigZagged, pu8OutputBuffer);
}

// Decodes a signed 32-bit ZigZag varint
static uint32_t Varint_DecodeSigned(const uint8_t *pu8InputBuffer, int32_t *pi32DecodedValue)
{
    uint32_t u32Temp = 0;
    uint32_t u32Length = Varint_DecodeUnsigned(pu8InputBuffer, &u32Temp);

    *pi32DecodedValue = ZigZag_Decode32(u32Temp);
    return u32Length;
}

void Encode_SingleStream(uint8_t *pu8OutputStream, int32_t *pi32InputBuffer, uint32_t u32InputLength, uint32_t *pu32OutputStreamLength)
{
    int32_t i32PreviousValue = 0;
    uint32_t u32BufferIndex = sizeof(int32_t);

    if (pu8OutputStream == NULL || pi32InputBuffer == NULL || u32InputLength == 0)
    {
        return;
    }

    memcpy(pu8OutputStream, &pi32InputBuffer[0], sizeof(int32_t));
    i32PreviousValue = pi32InputBuffer[0];
    for (uint32_t u32Index = 1; u32Index < u32InputLength; u32Index++)
    {
        int32_t i32Value = pi32InputBuffer[u32Index] - i32PreviousValue;
        u32BufferIndex += Varint_EncodeSigned(i32Value, &pu8OutputStream[u32BufferIndex]);
        i32PreviousValue = pi32InputBuffer[u32Index];
    }
    *pu32OutputStreamLength = u32BufferIndex;
}

void Decode_SingleStream(uint8_t *pu8InputStream, uint32_t u32InputStreamLength, int32_t *pi32OutputBuffer, uint32_t *pu32OutputLength)
{
    int32_t i32PreviousValue = 0;
    uint32_t u32Index = 0;
    uint8_t u8DecodedLength = 0;
    uint32_t u32SampleIndex = 1;
    
    if (pu8InputStream == NULL || pi32OutputBuffer == NULL || pu32OutputLength == NULL)
    {
        return;
    }

    memcpy(&i32PreviousValue, pu8InputStream, sizeof(int32_t));
    pi32OutputBuffer[0] = i32PreviousValue;

    for (u32Index = 4; u32Index < u32InputStreamLength; u32Index += u8DecodedLength)
    {
        int32_t i32Internal = 0;
        u8DecodedLength = Varint_DecodeSigned(&pu8InputStream[u32Index], &i32Internal);
        pi32OutputBuffer[u32SampleIndex] = i32Internal + i32PreviousValue;
        i32PreviousValue = pi32OutputBuffer[u32SampleIndex];
        u32SampleIndex++;
    }
    *pu32OutputLength = u32SampleIndex;
}

void Encode_MultiStream(uint8_t *pu8OutputStream, int32_t *pi32InputBuffer, uint32_t u32ColumnWidth, uint32_t u32RowLength, uint32_t *pu32OutputStreamLength)
{
    int32_t i32PreviousValue[u32ColumnWidth];
    uint32_t u32BufferIndex = 0;

    if (pu8OutputStream == NULL || pi32InputBuffer == NULL || pu32OutputStreamLength == NULL || u32ColumnWidth == 0 || u32RowLength == 0)
    {
        return;
    }
    
    for (uint32_t u32ColumnIndex = 0; u32ColumnIndex < u32ColumnWidth; u32ColumnIndex++)
    {
        memcpy(&pu8OutputStream[u32BufferIndex], &pi32InputBuffer[u32ColumnIndex], sizeof(int32_t));
        u32BufferIndex += sizeof(int32_t);
        i32PreviousValue[u32ColumnIndex] = pi32InputBuffer[u32ColumnIndex];
    }
    for (uint32_t u32RowIndex = 1; u32RowIndex < u32RowLength; u32RowIndex++)
    {
        for (uint32_t u32ColumnIndex = 0; u32ColumnIndex < u32ColumnWidth; u32ColumnIndex++)
        {
            uint32_t u32ArrayIndex = u32RowIndex * u32ColumnWidth + u32ColumnIndex;
            int32_t i32Value = pi32InputBuffer[u32ArrayIndex] - i32PreviousValue[u32ColumnIndex];
            u32BufferIndex += Varint_EncodeSigned(i32Value, &pu8OutputStream[u32BufferIndex]);
            i32PreviousValue[u32ColumnIndex] = pi32InputBuffer[u32ArrayIndex];
        }
    }
    *pu32OutputStreamLength = u32BufferIndex;
}

void Decode_MultiStream(uint8_t *pu8InputStream, uint32_t u32InputStreamLength, uint32_t u32ColumnWidth, int32_t *pi32OutputBuffer, uint32_t *pu32OutputLength)
{
    int32_t i32PreviousValue[u32ColumnWidth];
    uint32_t u32StreambufferIndex = 0;
    uint8_t u8DecodedLength = 0;
    uint32_t u32SampleIndex = 0;
    
    if (pu8InputStream == NULL || pi32OutputBuffer == NULL || pu32OutputLength == NULL)
    {
        return;
    }

    for (uint32_t u32ColumnIndex = 0; u32ColumnIndex < u32ColumnWidth; u32ColumnIndex++)
    {
        memcpy(&i32PreviousValue[u32ColumnIndex], &pu8InputStream[u32StreambufferIndex], sizeof(int32_t));
        u32StreambufferIndex += sizeof(int32_t);
        pi32OutputBuffer[u32SampleIndex] = i32PreviousValue[u32ColumnIndex];
        u32SampleIndex++;

    }

    uint32_t u32ColumnIndex = 0;

    for (; u32StreambufferIndex < u32InputStreamLength; u32StreambufferIndex += u8DecodedLength)
    {
        int32_t i32Internal = 0;
        u8DecodedLength = Varint_DecodeSigned(&pu8InputStream[u32StreambufferIndex], &i32Internal);
        pi32OutputBuffer[u32SampleIndex] = i32Internal + i32PreviousValue[u32ColumnIndex];
        i32PreviousValue[u32ColumnIndex] = pi32OutputBuffer[u32SampleIndex];
        u32ColumnIndex = (u32ColumnIndex + 1) % u32ColumnWidth;
        u32SampleIndex++;
    }
    *pu32OutputLength = u32SampleIndex;
}