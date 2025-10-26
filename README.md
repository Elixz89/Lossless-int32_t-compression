
# Lossless integer compression

This library is written to lossless compress data streams of int32_t type variables into a bytearray. This bytearray can be transmitted over a medium and converted back into the int32_t stream.

What it does is it takes a stream of int32_t variables and it keeps the first value as is and for each following value it takes the difference between the current and previous value. It then converts the signed value to only unsigned using zig-zag conversion. Lastly it converts the unsigned value to a variable length integer.
This is done for every variable in the int32_t stream and converts it into a byte array

For instance I have the values 12345 23456 34567 45678 34567 23456 12345.
This has a length of 28 bytes if it were in int32_t. Using the compression its reduced to 22 bytes.

The reduction of the compression is determined by the difference between two values. If the difference is small, the compression is high. If the difference is large, the compression is small.

There are two encode and decode functions, one for a single stream of data and the other for a multi stream data.

For multi stream the channels are interleaved, example { ch0, ch1, ch0, ch1, ch0, ch1 }

# Usage

## Single stream

On the transmitter side:
```    
uint8_t u8Buffer[500] = { 0 };

int32_t i32InputSamples[7] = { 12345, 23456, 34567, 45678, 34567, 23456, 12345 };
uint32_t u32StreamLength = 0;

Encode_SingleStream(u8Buffer, i32InputSamples, 7, &u32StreamLength);
```

This will compress the i32InputSamples array into the u8Buffer. 
The u32StreamLength will contain the number of bytes written to the u8Buffer.
The u8Buffer content will look like this
```
0x39 0x30 0x00 0x00 0xce 0xad 0x01 0xce 0xad 0x01 0xce 0xad 0x01 0xcd 0xad 0x01 0xcd 0xad 0x01 0xcd 0xad 0x01 
```

On the receiver side:
```
uint8_t u8Buffer[500];
int32_t i32OutputSamples[7] = { 0 }; 
uint32_t u32StreamLength = 22;
uint32_t u32OutputStreamLength = 0;

Decode_SingleStream(u8Buffer, u32StreamLength, i32OutputSamples, &u32OutputStreamLength);
```
u32OutputStreamLength will hold the number of samples it has decoded, in the example case this will hold the value 7.


## Multi stream

On the transmitter side:

```
uint8_t u8NumberOfChannels = 2;
uint8_t u8SamplesPerChannel = 7;
uint8_t u8Buffer[500] = { 0 };

int32_t i32InputSamples[14] = { 12345, 23456, 12346, 34567, 12347, 45678, 12348, 56789, 12347, 45678, 12346, 34567, 12345, 23456 };

uint32_t u32StreamLength = 0;

Encode_MultiStream(u8Buffer, i32InputSamples, u8NumberOfChannels, u8SamplesPerChannel, &u32StreamLength);

```
This will compress the i32InputSamples array into the u8Buffer. 
The u32StreamLength will contain the number of bytes written to the u8Buffer.
The u8Buffer content will look like this
```
0x39 0x30 0x00 0x00 0xa0 0x5b 0x00 0x00 0x02 0xce 0xad 0x01 0x02 0xce 0xad 0x01 0x02 0xce 0xad 0x01 0x01 0xcd 0xad 0x01 0x01 0xcd 0xad 0x01 0x01 0xcd 0xad 0x01 
```

On the receiver side:
```
uint8_t u8Buffer[500];
uint8_t u8NumberOfChannels = 2;
int32_t i32OutputSamples2[14] = { 0 }; 

uint32_t u32StreamLength = 32;

uint32_t u32OutputStreamLength = 0;

Decode_MultiStream(u8Buffer, u32StreamLength, u8NumberOfChannels, i32OutputSamples, &u32OutputStreamLength);
```
u32OutputStreamLength will hold the number of samples it has decoded, in the example case this will hold the value 14.
