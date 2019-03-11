// Copyright (C) 2013-2017 Altera Corporation, San Jose, California, USA. All rights reserved.
// Permission is hereby granted, free of charge, to any person obtaining a copy of this
// software and associated documentation files (the "Software"), to deal in the Software
// without restriction, including without limitation the rights to use, copy, modify, merge,
// publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to
// whom the Software is furnished to do so, subject to the following conditions:
// The above copyright notice and this permission notice shall be included in all copies or
// substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
// OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
// HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
// WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
// OTHER DEALINGS IN THE SOFTWARE.
// 
// This agreement shall be governed in all respects by the laws of the State of California and
// by the laws of the United States of America.

// Handy macros for defining channel constructs
// Can be included by both the kernels and the hosts.

// channel -> DDR adapter definition
#define CHANNEL_TO_DDR_NAME(channel) "channel_to_ddr_" channel
#define CHANNEL_TO_DDR(type,channel) \
kernel void channel_to_ddr_##channel (global type* ptr) { \
  int offset = 0; \
  while (1) { ptr[offset++] = read_channel_intel (channel); } \
}

// DDR -> channel adapter definition
#define DDR_TO_CHANNEL_NAME(channel) "ddr_to_channel_" channel
#define DDR_TO_CHANNEL(type,channel) \
kernel void ddr_to_channel_##channel (global type* ptr) { \
  int offset = 0; \
  while (1) { write_channel_intel (channel, ptr[offset++]); } \
}


// Channel tee
#define TEE_NAME(channel1,channel2,channel3) "tee_" channel1 "_" channel2 "_" channel3
#define TEE(type,channel1,channel2,channel3) \
kernel void tee_##channel_##channel2_##channel2 (void) { \
  while (1) { \
    type value = read_channel_intel (channel); \
    write_channel_intel (channel2, value); \
    write_channel_intel (channel3, value); \
  } \
}

// Channel tee, with one branch going to DDR.
#define TEE_TO_DDR_NAME(channel1,channel2) "tee_to_ddr_" channel "_" channel2
#define TEE_TO_DDR(type,channel1,channel2) \
kernel void tee_to_ddr_##channel_##channel2 (global type* ptr) { \
  int offset = 0;\
  while (1) { \
    type value = read_channel_intel (channel); \
    ptr[offset++] = value; \
    write_channel_intel (channel2, value); \
  } \
}
