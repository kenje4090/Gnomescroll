
#include "sequence_numbers.h"

struct packet_sequence {
    int seq;
    int ack;
    //int active;
    //int tick; //better than sequence for determining dropped packets
};

//unsigned int packet_buffer[32];

struct packet_sequence packet_sequence_buffer[64];
int packet_sequence_buffer_n = 0;

int packet_sequence_number = 0;

void init_sequence_numbers() {
    int i;
    for(i=0;i<64;i++) {
        //packet_buffer[i].active = 0;
        packet_sequence_buffer[i].ack = 0;
        packet_sequence_buffer[i].seq = -1;
    }
}

void process_acks(unsigned short seq, unsigned int flag) {
    unsigned int n = 1;
    int i,j,index;
    for(i=0;i<32;i++) {
        index = (seq -i) % 2048; //seq is id of highest packet server has seen yet
        if(index < 0) { printf("warning! index is negative!\n"); }

        if(flag & (n != 0)) {
            //ack that packet
            for(j=0;j<64;j++) { //could use offset
                if(packet_sequence_buffer[j].seq == index) {
                    if(packet_sequence_buffer[j].ack == 0) {
                        printf("Packet Acked: %i\n", index);
                    }
                    packet_sequence_buffer[j].ack = 1;
                    //printf("Packet Acked: %i\n", index);
                    if(j == seq%64) {
                        printf("j == seq mod 64 \n");
                    }
                    break;
                }
            }
        }
        n*=2;
    }
}

uint16_t get_next_sequence_number() {
    packet_sequence_number += 1;
    //printf("get_next_sequence_number: seq=%i \n", packet_sequence_number);
    int index = packet_sequence_number%64;

    //packet_sequence_buffer[index].active = 1;
    packet_sequence_buffer[index].seq = packet_sequence_number;
    packet_sequence_buffer[index].ack = 0;
    return packet_sequence_number;
}

void check_for_dropped_packets() {
    int i;
    for(i=0;i<64;i++) {
        if(packet_sequence_buffer[i].seq >=0 && packet_sequence_number - packet_sequence_buffer[i].seq > 32) {
            printf("Packet %i assumed lost\n", packet_sequence_buffer[i].seq);
            packet_sequence_buffer[i].seq = -1;
        }
    }
}
