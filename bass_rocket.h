#pragma once

#include "rocket/sync.h"

#include <bass.h>
#include <iostream>

void bassPause(void *d, int flag);
void bassSetRow(void *d, int row);
int bassIsPlaying(void *d);
double bassGetRow(HSTREAM h);
sync_device* syncStartup();
void syncUpdate(sync_device* rocket, HSTREAM stream);
