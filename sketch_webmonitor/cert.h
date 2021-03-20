#ifndef CERT_H_
#define CERT_H_
unsigned char example_crt_DER[] = {
  0x30, 0x82, 0x02, 0x18, 0x30, 0x82, 0x01, 0x81, 0x02, 0x01, 0x02, 0x30,
  0x0d, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x01, 0x0b,
  0x05, 0x00, 0x30, 0x54, 0x31, 0x0b, 0x30, 0x09, 0x06, 0x03, 0x55, 0x04,
  0x06, 0x13, 0x02, 0x44, 0x45, 0x31, 0x0b, 0x30, 0x09, 0x06, 0x03, 0x55,
  0x04, 0x08, 0x0c, 0x02, 0x42, 0x45, 0x31, 0x0f, 0x30, 0x0d, 0x06, 0x03,
  0x55, 0x04, 0x07, 0x0c, 0x06, 0x42, 0x65, 0x72, 0x6c, 0x69, 0x6e, 0x31,
  0x12, 0x30, 0x10, 0x06, 0x03, 0x55, 0x04, 0x0a, 0x0c, 0x09, 0x4d, 0x79,
  0x43, 0x6f, 0x6d, 0x70, 0x61, 0x6e, 0x79, 0x31, 0x13, 0x30, 0x11, 0x06,
  0x03, 0x55, 0x04, 0x03, 0x0c, 0x0a, 0x6d, 0x79, 0x63, 0x61, 0x2e, 0x6c,
  0x6f, 0x63, 0x61, 0x6c, 0x30, 0x1e, 0x17, 0x0d, 0x32, 0x31, 0x30, 0x33,
  0x31, 0x36, 0x32, 0x30, 0x32, 0x38, 0x30, 0x31, 0x5a, 0x17, 0x0d, 0x33,
  0x31, 0x30, 0x33, 0x31, 0x34, 0x32, 0x30, 0x32, 0x38, 0x30, 0x31, 0x5a,
  0x30, 0x55, 0x31, 0x0b, 0x30, 0x09, 0x06, 0x03, 0x55, 0x04, 0x06, 0x13,
  0x02, 0x44, 0x45, 0x31, 0x0b, 0x30, 0x09, 0x06, 0x03, 0x55, 0x04, 0x08,
  0x0c, 0x02, 0x42, 0x45, 0x31, 0x0f, 0x30, 0x0d, 0x06, 0x03, 0x55, 0x04,
  0x07, 0x0c, 0x06, 0x42, 0x65, 0x72, 0x6c, 0x69, 0x6e, 0x31, 0x12, 0x30,
  0x10, 0x06, 0x03, 0x55, 0x04, 0x0a, 0x0c, 0x09, 0x4d, 0x79, 0x43, 0x6f,
  0x6d, 0x70, 0x61, 0x6e, 0x79, 0x31, 0x14, 0x30, 0x12, 0x06, 0x03, 0x55,
  0x04, 0x03, 0x0c, 0x0b, 0x65, 0x73, 0x70, 0x33, 0x32, 0x2e, 0x6c, 0x6f,
  0x63, 0x61, 0x6c, 0x30, 0x81, 0x9f, 0x30, 0x0d, 0x06, 0x09, 0x2a, 0x86,
  0x48, 0x86, 0xf7, 0x0d, 0x01, 0x01, 0x01, 0x05, 0x00, 0x03, 0x81, 0x8d,
  0x00, 0x30, 0x81, 0x89, 0x02, 0x81, 0x81, 0x00, 0xd1, 0x4d, 0xd7, 0xd2,
  0xd0, 0xb3, 0x67, 0x7b, 0x35, 0x03, 0xc3, 0x48, 0x5b, 0x88, 0xc9, 0x17,
  0xbf, 0x6a, 0xb0, 0x99, 0x17, 0x76, 0xbd, 0xed, 0xc5, 0xff, 0xc6, 0x16,
  0xa8, 0x3c, 0x2d, 0xb3, 0xe4, 0xb8, 0x17, 0xf3, 0xde, 0x97, 0xe3, 0x47,
  0xad, 0xa8, 0x67, 0xe2, 0x4f, 0x23, 0x38, 0x00, 0x22, 0x7b, 0x6a, 0x40,
  0x10, 0xcc, 0x9c, 0x5a, 0xdc, 0xd5, 0x82, 0x2b, 0x50, 0xca, 0xae, 0x45,
  0x05, 0x3d, 0xca, 0x61, 0xc0, 0x09, 0xf6, 0xe8, 0x80, 0x2b, 0xbb, 0x30,
  0x7a, 0x64, 0x7a, 0x1a, 0x4f, 0x4a, 0x03, 0x7b, 0x19, 0x6a, 0x7a, 0x58,
  0x15, 0xf7, 0xc1, 0x3a, 0xeb, 0x4d, 0xb9, 0x67, 0xba, 0x5a, 0x21, 0xe8,
  0x75, 0x15, 0xc7, 0x90, 0xcb, 0x3f, 0xe4, 0x94, 0xb9, 0xdb, 0xac, 0x2f,
  0xfc, 0x27, 0x95, 0xc4, 0x43, 0xf0, 0xe1, 0xf5, 0x41, 0xb0, 0x4c, 0xe6,
  0xe4, 0x3e, 0x87, 0x39, 0x02, 0x03, 0x01, 0x00, 0x01, 0x30, 0x0d, 0x06,
  0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x01, 0x0b, 0x05, 0x00,
  0x03, 0x81, 0x81, 0x00, 0x3a, 0x33, 0xc8, 0xad, 0x89, 0x55, 0xce, 0xbe,
  0x01, 0x0f, 0x81, 0xe6, 0x9f, 0x23, 0x31, 0xa4, 0x4c, 0x5e, 0xfd, 0xd2,
  0x06, 0x4f, 0xc9, 0x7e, 0x15, 0xef, 0x40, 0x35, 0xe4, 0x3f, 0xe4, 0x23,
  0x32, 0xfb, 0xb6, 0x0d, 0x0b, 0xec, 0x71, 0x98, 0xf9, 0xb7, 0x16, 0xda,
  0xfe, 0x24, 0x26, 0x7b, 0x9b, 0xfe, 0xea, 0x50, 0x76, 0xa8, 0x0d, 0xae,
  0x9b, 0x12, 0x0c, 0x4f, 0x09, 0x53, 0x32, 0x19, 0x93, 0xc0, 0x84, 0xaa,
  0xc2, 0x7f, 0xed, 0x7e, 0xaa, 0xc6, 0xd6, 0x64, 0x62, 0x83, 0xbd, 0xf3,
  0xaa, 0x8b, 0x38, 0x8e, 0x98, 0x65, 0x7f, 0x2b, 0x7e, 0xf6, 0xf6, 0x68,
  0x9d, 0x07, 0x82, 0xf2, 0x48, 0x42, 0xa7, 0xd4, 0xb8, 0x07, 0xbf, 0x89,
  0x20, 0xa2, 0x51, 0x0d, 0x39, 0x23, 0xe9, 0xd7, 0x01, 0x8e, 0x4f, 0xb9,
  0x32, 0xae, 0x37, 0x96, 0x64, 0x4b, 0x04, 0x0a, 0x64, 0xf5, 0x1f, 0x9e
};
unsigned int example_crt_DER_len = 540;
#endif