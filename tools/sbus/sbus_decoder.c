/* gcc -o sbus_decoder.out sbus_decoder.c */

#include <stdint.h>
#include <stdio.h>

#define SBUS_PACKET_LENGTH     25
#define SBUS_HEADER          0x0f
#define SBUS_FOOTER          0x00
#define SBUS_CH17_MASK       0x01
#define SBUS_CH18_MASK       0x02
#define SBUS_LOST_FRAME_MASK 0x04
#define SBUS_FAILSAFE_MASK   0x08

uint8_t buffer[SBUS_PACKET_LENGTH];

typedef struct {
  uint16_t channels[16];
  uint8_t channel17;
  uint8_t channel18;
  uint8_t lost_frame;
  uint8_t failsafe;
} sbus_state_t;

sbus_state_t sbus_state;

/*
 * This algorithm is meant to deliver frames as quick as possible.
 * There is no fancy filtering. If one packet is too short or has incorrect
 * footer we will miss two packets. If it is too long or has incorrect header,
 * we will miss only this packet.
 */
uint8_t sbus_decode_packet(uint8_t current_byte)
{
  static uint8_t packet_length = 0;
  static uint8_t previous_byte = SBUS_FOOTER;

  if (packet_length == 0) {
    if (current_byte == SBUS_HEADER && previous_byte == SBUS_FOOTER) {
      buffer[packet_length] = current_byte;
      packet_length++;
    }
  } else {
    buffer[packet_length] = current_byte;
    if (packet_length < SBUS_PACKET_LENGTH - 1) {
      packet_length++;
    } else {
      packet_length = 0;
      if (current_byte == SBUS_FOOTER) {
        return 1;
      }
    }
  }
  previous_byte = current_byte;
  return 0;
}

int main(int argc, char **argv)
{
  if (argc != 2) {
      printf("Please provide a file to parse.\n");
      return 1;
  }

  FILE *fhandle;
  fhandle = fopen(argv[1], "r");

  if (!fhandle) {
    printf("Cannot open file: %s\n", argv[1]);
      return 1;
  }

  int byte;
  while ((byte = fgetc(fhandle)) != EOF) {
    if (1u == sbus_decode_packet((uint8_t)byte)) {
      // uint8_t i = 0;
      // while (i < SBUS_PACKET_LENGTH) {
      //   printf("%.2x ", buffer[i]);
      //   i++;
      // }
      // printf("\n");

      sbus_state.channels[0]  = buffer[1]       | ((uint16_t)buffer[2]  << 8  & 0x07FF);
      sbus_state.channels[1]  = buffer[2]  >> 3 | ((uint16_t)buffer[3]  << 5  & 0x07FF);
      sbus_state.channels[2]  = buffer[3]  >> 6 |  (uint16_t)buffer[4]  << 2
                                                | ((uint16_t)buffer[5]  << 10 & 0x07FF);
      sbus_state.channels[3]  = buffer[5]  >> 1 | ((uint16_t)buffer[6]  << 7  & 0x07FF);
      sbus_state.channels[4]  = buffer[6]  >> 4 | ((uint16_t)buffer[7]  << 4  & 0x07FF);
      sbus_state.channels[5]  = buffer[7]  >> 7 |  (uint16_t)buffer[8]  << 1
                                                | ((uint16_t)buffer[9]  << 9  & 0x07FF);
      sbus_state.channels[6]  = buffer[9]  >> 2 | ((uint16_t)buffer[10] << 6  & 0x07FF);
      sbus_state.channels[7]  = buffer[10] >> 5 | ((uint16_t)buffer[11] << 3  & 0x07FF);
      sbus_state.channels[8]  = buffer[12]      | ((uint16_t)buffer[13] << 8  & 0x07FF);
      sbus_state.channels[9]  = buffer[13] >> 3 | ((uint16_t)buffer[14] << 5  & 0x07FF);
      sbus_state.channels[10] = buffer[14] >> 6 |  (uint16_t)buffer[15] << 2
                                                | ((uint16_t)buffer[16] << 10 & 0x07FF);
      sbus_state.channels[11] = buffer[16] >> 1 | ((uint16_t)buffer[17] << 7  & 0x07FF);
      sbus_state.channels[12] = buffer[17] >> 4 | ((uint16_t)buffer[18] << 4  & 0x07FF);
      sbus_state.channels[13] = buffer[18] >> 7 |  (uint16_t)buffer[19] << 1
                                                | ((uint16_t)buffer[20] << 9  & 0x07FF);
      sbus_state.channels[14] = buffer[20] >> 2 | ((uint16_t)buffer[21] << 6  & 0x07FF);
      sbus_state.channels[15] = buffer[21] >> 5 | ((uint16_t)buffer[22] << 3  & 0x07FF);

      sbus_state.channel17 = buffer[23] & SBUS_CH17_MASK ? 1 : 0;
      sbus_state.channel18 = buffer[23] & SBUS_CH18_MASK ? 1 : 0;
      sbus_state.lost_frame = buffer[23] & SBUS_LOST_FRAME_MASK ? 1 : 0;
      sbus_state.failsafe = buffer[23] & SBUS_FAILSAFE_MASK ? 1 : 0;

      for (int i = 0; i < 16; i++) {
        printf("%.4d ", sbus_state.channels[i]);
      }
      printf("ch17: %d ch18: %d lost_frame: %d failsafe: %d\n",
             sbus_state.channel17, sbus_state.channel18,
             sbus_state.lost_frame, sbus_state.failsafe);
    }
  }

  fclose(fhandle);

  return 0;
}
