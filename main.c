#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "streamCoding.h"

int main(int argc, char *argv[])
{

    uint8_t u8Buffer[500] = { 0 };

    int32_t i32InputSamples[7] = { 12345, 23456, 34567, 45678, 34567, 23456, 12345 };
    int32_t i32OutputSamples[100] = { 0 }; 

    uint32_t u32StreamLength = 0;
    uint32_t u32OutputStreamLength = 0;

    Encode_SingleStream(u8Buffer, i32InputSamples, 7, &u32StreamLength);

    printf("original length = %d compressed length is %d\n\r", sizeof(i32InputSamples), u32StreamLength);

    for (uint32_t u32Index = 0; u32Index < u32StreamLength; u32Index++)
    {
        printf("0x%02x ", u8Buffer[u32Index]);
    }
    printf("\n\r");

    Decode_SingleStream(u8Buffer, u32StreamLength, i32OutputSamples, &u32OutputStreamLength);


    printf("Length is %d\n\r", u32OutputStreamLength);


    for (uint32_t u32Index = 0; u32Index < u32OutputStreamLength; u32Index++)
    {
        printf("%d %d\n\r", i32InputSamples[u32Index], i32OutputSamples[u32Index]);
    }
    
    uint8_t u8Buffer2[500] = { 0 };

    int32_t i32InputSamples2[14] = { 12345, 23456, 12346, 34567, 12347, 45678, 12348, 56789, 12347, 45678, 12346, 34567, 12345, 23456 };
    int32_t i32OutputSamples2[14] = { 0 }; 

    uint32_t u32StreamLength2 = 0;
    uint32_t u32OutputStreamLength2 = 0;


    Encode_MultiStream(u8Buffer2, i32InputSamples2, 2, 7, &u32StreamLength2);


    printf("original length = %d compressed length is %d\n\r", sizeof(i32InputSamples2), u32StreamLength2);

    for (uint32_t u32Index = 0; u32Index < u32StreamLength2; u32Index++)
    {
        printf("0x%02x ", u8Buffer2[u32Index]);
    }
    printf("\n\r");

    Decode_MultiStream(u8Buffer2, u32StreamLength2, 2, i32OutputSamples2, &u32OutputStreamLength2);


    printf("Length is %d\n\r", u32OutputStreamLength2);


    for (uint32_t u32Index = 0; u32Index < u32OutputStreamLength2; u32Index++)
    {
        printf("%d %d\n\r", i32InputSamples2[u32Index], i32OutputSamples2[u32Index]);
    }

    
    return 0;
}
