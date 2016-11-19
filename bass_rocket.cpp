#include "bass_rocket.h"

#include <direct.h>  
#include <stdio.h>  
#include <stdlib.h>  

static const bool syncClient = false;

// Configure your song
static const float bpm = 125.0; /* beats per minute */
static const int rpb = 4; /* rows per beat */
static const double row_rate = (double(bpm) / 60) * rpb;

// A bunch of functions straight from the Rocket example code
void bassPause(void *d, int flag) {
    HSTREAM h = *((HSTREAM *)d);
    if (flag) {
        BASS_ChannelPause(h);
    } 
    else {
        BASS_ChannelPlay(h, false);
    }
}

void bassSetRow(void *d, int row) {
    HSTREAM h = *((HSTREAM *)d);
    QWORD pos = BASS_ChannelSeconds2Bytes(h, row / row_rate);
    BASS_ChannelSetPosition(h, pos, BASS_POS_BYTE);
}

int bassIsPlaying(void *d) {
    HSTREAM h = *((HSTREAM *)d);
    return BASS_ChannelIsActive(h) == BASS_ACTIVE_PLAYING;
}

double bassGetRow(HSTREAM h) {
    QWORD pos = BASS_ChannelGetPosition(h, BASS_POS_BYTE);
    double time = BASS_ChannelBytes2Seconds(h, pos);
    return time * row_rate;
}

struct sync_cb bass_cb = {
    bassPause,
    bassSetRow,
    bassIsPlaying
};

// Start rocket
sync_device* syncStartup() {
    sync_device *rocket = sync_create_device("sync");

    if(syncClient) {
        if (sync_tcp_connect(rocket, "localhost", SYNC_DEFAULT_PORT)) {
            std::cerr << "Started in client mode, but could not connect to editor!" << std::endl;
            exit(-1);
        }
    }

    return rocket;
}

// Perform row update
void syncUpdate(sync_device* rocket, HSTREAM stream) {
    if(sync_update(rocket, (int)floor(bassGetRow(stream)), &bass_cb, (void *)&stream)) {
        if(syncClient) {
            if(sync_tcp_connect(rocket, "localhost", SYNC_DEFAULT_PORT)) {
                std::cerr << "Editor died" << std::endl;
                exit(-1);
            }
        }
    }
}
