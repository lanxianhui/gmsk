// FIR filter used for 48k_to_8k and 8k_to_48 resampling

// originally part of "codec2-devel" software package
// original (C) 2012 David Rowe
// distributed under term of the GNU lesser General Public License version 2.1
 
// Used by Kristoff Bonne ON1ARF for the "audiotool" application
// part of the "c2_gmskmodem" package

/*
 *      Copyright (C) 2013 by Kristoff Bonne, ON1ARF
 *
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; version 2 of the License.
 *
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 */

// version 20121222: initial release
// version 20130404: added halfduplex support


/* Generate using fir1(47,1/6) in Octave */
const float fdmdv_os_filter[]= {
    -3.55606818e-04,
    -8.98615286e-04,
    -1.40119781e-03,
    -1.71713852e-03,
    -1.56471179e-03,
    -6.28128960e-04,
    1.24522223e-03,
    3.83138676e-03,
    6.41309478e-03,
    7.85893186e-03,
    6.93514929e-03,
    2.79361991e-03,
    -4.51051400e-03,
    -1.36671853e-02,
    -2.21034939e-02,
    -2.64084653e-02,
    -2.31425052e-02,
    -9.84218694e-03,
    1.40648474e-02,
    4.67316298e-02,
    8.39615986e-02,
    1.19925275e-01,
    1.48381174e-01,
    1.64097819e-01,
    1.64097819e-01,
    1.48381174e-01,
    1.19925275e-01,
    8.39615986e-02,
    4.67316298e-02,
    1.40648474e-02,
    -9.84218694e-03,
    -2.31425052e-02,
    -2.64084653e-02,
    -2.21034939e-02,
    -1.36671853e-02,
    -4.51051400e-03,
    2.79361991e-03,
    6.93514929e-03,
    7.85893186e-03,
    6.41309478e-03,
    3.83138676e-03,
    1.24522223e-03,
    -6.28128960e-04,
    -1.56471179e-03,
    -1.71713852e-03,
    -1.40119781e-03,
    -8.98615286e-04,
    -3.55606818e-04
};

