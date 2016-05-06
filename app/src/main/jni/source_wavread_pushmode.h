/*
  Copyright (C) 2014 Paul Brossier <piem@aubio.org>

  This file is part of aubio.

  aubio is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  aubio is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with aubio.  If not, see <http://www.gnu.org/licenses/>.

*/

#ifndef _AUBIO_SOURCE_WAVREAD_PUSHMODE_H
#define _AUBIO_SOURCE_WAVREAD_PUSHMODE_H

/** \file

  Read from file using custom wav reading routines.

  Avoid including this file directly! Prefer using ::aubio_source_t instead to
  make your code portable.

  To write to file, use ::aubio_sink_t.

  References:

    - http://netghost.narod.ru/gff/graphics/summary/micriff.htm
    - https://ccrma.stanford.edu/courses/422/projects/WaveFormat/

  \example io/test-source_wavread.c

*/

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*wavread_pushmode_func_t) (uint_t Data, fvec_t * output);



/** wavread media source object */
typedef struct _aubio_source_wavread_pushmode_t aubio_source_wavread_pushmode_t;
/**

  create new ::aubio_source_wavread_pushmode_t

  \param uri the file path or uri to read from
  \param samplerate sampling rate to view the fie at
  \param hop_size the size of the blocks to read from

  Creates a new source object. If `0` is passed as `samplerate`, the sample
  rate of the original file is used.

  The samplerate of newly created source can be obtained using
  ::aubio_source_wavread_get_samplerate.

*/
aubio_source_wavread_pushmode_t * new_aubio_source_wavread_pushmode(uint_t Data,wavread_pushmode_func_t ProcessFuc, uint_t hop_size);

/**

  read monophonic vector of length hop_size from source object

  \param s source object, created with ::new_aubio_source_wavread
  \param read_to ::fvec_t of data to read to
  \param[out] read upon returns, equals to number of frames actually read

  Upon returns, `read` contains the number of frames actually read from the
  source. `hop_size` if enough frames could be read, less otherwise.

*/

void aubio_source_wavread_pushmode_headdata(aubio_source_wavread_pushmode_t *s, char *headbuffer, uint_t nLength);

void aubio_source_wavread_pushmode_do(aubio_source_wavread_pushmode_t * s, char *pBuffer, uint_t nLength);


/**

  get samplerate of source object

  \param s source object, created with ::new_aubio_source_wavread
  \return samplerate, in Hz

*/
uint_t aubio_source_wavread_pushmode_get_samplerate(aubio_source_wavread_pushmode_t * s);

/**

  get number of channels of source object

  \param s source object, created with ::new_aubio_source_wavread
  \return number of channels

*/
uint_t aubio_source_wavread_pushmode_get_channels (aubio_source_wavread_pushmode_t * s);


/**

  close source and cleanup memory

  \param s source object, created with ::new_aubio_source_wavread

*/
void del_aubio_source_wavread_pushmode(aubio_source_wavread_pushmode_t * s);

#ifdef __cplusplus
}
#endif

#endif /* _AUBIO_SOURCE_WAVREAD_PUSHMODE_H */
