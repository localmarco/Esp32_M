#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Fri Feb  8 11:59:40 2019

@author: marco
"""

import os
import sys
import numpy as np
from PIL import Image

def help():
    print('''
use: img2h.py [img] (.h)
''')

if __name__ == '__main__':
    if len(sys.argv) < 2:
        help()
        sys.exit(-1)

    img = np.array(Image.open(sys.argv[1]).convert('L'))
    r, c = img.shape
    for r1 in range(r):
        for c1 in range(c):
            if img[r1, c1] < 128:
                img[r1, c1] = 1;
            else:
                img[r1, c1] = 0;

    for r1 in range(r):
        for c1 in range(c):
            if img[r1, c1] == 1:
                print(1, end='')
            else:
                print(0, end='')
        print('')

    if len(sys.argv) < 3:
        sys.exit(0)

    
    h = "#ifndef __" + sys.argv[2].split('.')[0] + "_" + sys.argv[2].split('.')[1] +'\n'
    h = h + "#define __" + sys.argv[2].split('.')[0] + "_" + sys.argv[2].split('.')[1] +'\n\n'
    h = h + "unsigned char " + sys.argv[2].split('.')[0] + "[] = {\n"
    for r1 in range((int)(r / 8)):
        for c1 in range(c):
            i = r1 * 8
            b = ((img[i+0,c1]   )&0x01)| ((img[i+1,c1]<<1)&0x02)| ((img[i+2,c1]<<2)&0x04)| ((img[i+3,c1]<<3)&0x08)| ((img[i+4,c1]<<4)&0x10)| ((img[i+5,c1]<<5)&0x20)| ((img[i+6,c1]<<6)&0x40)| ((img[i+7,c1]<<7)&0x80)
            h = h + hex(b) + ","
        h = h + '\n'
    h = h + "};\n#endif\n"

    print(h)

    f = open(sys.argv[2], 'w+')
    f.write(h)
    f.close()
