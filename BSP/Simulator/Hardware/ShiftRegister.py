# Copyright (c) 2020 UT Longhorn Racing Solar

import csv
import os
import fcntl

# Path of file
file = "BSP/Simulator/Hardware/Data/SPI.csv"


class ShiftRegister:
    OPCODE = 0b0100000
    REGISTER_DATA = {
        'IODIRA': 0xFF,
        'IODIRB': 0xFF,
        'IPOLA': 0x00,
        'IPOLB': 0x00,
        'GPINTENA': 0x00,
        'GPINTENB': 0x00,
        'DEFVALA': 0x00,
        'DEFVALB': 0x00,
        'INTCONA': 0x00,
        'INTCONB': 0x00,
        'IOCON': 0x00,
        'GPPUA': 0x00,
        'GPPUB': 0x00,
        'INTFA': 0x00,
        'INTFB': 0x00,
        'INTCAPA': 0x00,
        'INTCAPB': 0x00,
        'GPIOA': 0x00,
        'GPIOB': 0x00,
        'OLATA': 0x00,
        'OLATB': 0x00
    }
    # Addresses with IOCON.BANK = 1
    REGISTER_ADDR_1 = {
        0x00: 'IODIRA',
        0x10: 'IODIRB',
        0x01: 'IPOLA',
        0x11: 'IPOLB',
        0x02: 'GPINTENA',
        0x12: 'GPINTENB',
        0x03: 'DEFVALA',
        0x13: 'DEFVALB',
        0x04: 'INTCONA',
        0x14: 'INTCONB',
        0x05: 'IOCON',
        0x15: 'IOCON',
        0x06: 'GPPUA',
        0x16: 'GPPUB',
        0x07: 'INTFA',
        0x17: 'INTFB',
        0x08: 'INTCAPA',
        0x18: 'INTCAPB',
        0x09: 'GPIOA',
        0x19: 'GPIOB',
        0x0A: 'OLATA',
        0x1A: 'OLATB'
    }
    # Addresses with IOCON.BANK = 0
    REGISTER_ADDR_0 = {
        0x00: 'IODIRA',
        0x01: 'IODIRB',
        0x02: 'IPOLA',
        0x03: 'IPOLB',
        0x04: 'GPINTENA',
        0x05: 'GPINTENB',
        0x06: 'DEFVALA',
        0x07: 'DEFVALB',
        0x08: 'INTCONA',
        0x09: 'INTCONB',
        0x0A: 'IOCON',
        0x0B: 'IOCON',
        0x0C: 'GPPUA',
        0x0D: 'GPPUB',
        0x0E: 'INTFA',
        0x0F: 'INTFB',
        0x10: 'INTCAPA',
        0x11: 'INTCAPB',
        0x12: 'GPIOA',
        0x13: 'GPIOB',
        0x14: 'OLATA',
        0x15: 'OLATB'
    }


    def write_register(self, register, data):
        if register[0:-1] == 'GPIO':
            # Make sure only inputs are written
            io = self.REGISTER_DATA[f'IODIR{register[-1]}']
            oldData = self.REGISTER_DATA[f'GPIO{register[-1]}']
            newData = 0
            for i in range(8):
                if (io >> i) & 0x01 == 1:
                    newData += data & (0x01 << i)
                else:
                    newData += oldData & (0x01 << i)
            self.REGISTER_DATA[f'GPIO{register[-1]}'] = newData
        else:
            self.REGISTER_DATA[register] = data


    def read_register(self, register):
        pass
    


# Global register
reg = ShiftRegister()

def read():
    pass


def write():
    pass


if __name__ == '__main__':
    reg.write_register('IODIRA', 0x32)
    reg.write_register('GPIOA', 0x42)
    print(reg.REGISTER_DATA['GPIOA'])
