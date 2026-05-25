//
// Created by Amir Diaa on 24/05/2026.
//

#include "CPU.h"


void CPU::reset() {  // TODO SP and flag init

    A = X = Y = 0;
    PC = 0xFFFC;
}
