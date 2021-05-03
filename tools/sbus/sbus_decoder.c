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

// typedef struct {
//   uint16_t channels[16];
//   uint8_t channel17;
//   uint8_t channel18;

// } sbus_state_t;

// sbus_state_t sbus_state;

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
    } else {
      printf("x ");
    }
  } else {
    buffer[packet_length] = current_byte;
    if (packet_length < SBUS_PACKET_LENGTH - 1) {
      packet_length++;
    } else {
      packet_length = 0;
      if (current_byte == SBUS_FOOTER) {
        return 1;
      } else {
        printf("error ");
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
      uint8_t i = 0;
      while (i < SBUS_PACKET_LENGTH) {
        printf("%.2x ", buffer[i]);
        i++;
      }
      printf("\n");
    }
  }

  fclose(fhandle);

  return 0;
}
