#ifndef _STREAMCODING_H_
#define _STREAMCODING_H_

#include <stdint.h>

void Encode_SingleStream(uint8_t *pu8OutputStream, int32_t *pi32InputBuffer, uint32_t u32InputLength, uint32_t *pu32OutputStreamLength);
void Decode_SingleStream(uint8_t *pu8InputStream, uint32_t u32InputStreamLength, int32_t *pi32OutputBuffer, uint32_t *pu32OutputLength);

void Encode_MultiStream(uint8_t *pu8OutputStream, int32_t *pi32InputBuffer, uint32_t u32ColumnWidth, uint32_t u32RowLength, uint32_t *pu32OutputStreamLength);
void Decode_MultiStream(uint8_t *pu8InputStream, uint32_t u32InputStreamLength, uint32_t u32ColumnWidth, int32_t *pi32OutputBuffer, uint32_t *pu32OutputLength);



#endif