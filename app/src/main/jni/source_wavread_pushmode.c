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

#include "config.h"

#ifdef HAVE_WAVREAD_PUSHMODE
#include "aubio_priv.h"
#include "fvec.h"
#include "fmat.h"
#include "source_wavread_pushmode.h"

#include <errno.h>
#include "types.h"
#define AUBIO_WAVREAD_BUFSIZE 1024*2

#define SHORT_TO_FLOAT(x) (smpl_t)(x * 3.0517578125e-05)

struct _aubio_source_wavread_pushmode_t {
  uint_t hop_size;
  uint_t samplerate;
  uint_t channels;

  // some data about the file
//  char_t *path;
//  uint_t input_samplerate;
//  uint_t channels;

  // internal stuff
//  FILE *fid;

  fmat_t *output;
  uint_t read_index;  
  uint_t read_samples;
  uint_t blockalign;
  uint_t bitspersample;
  uint_t eof;

//  size_t seek_start;
  fvec_t * read_data;
  uint_t ReadSamples;

  uint_t Data;
  wavread_pushmode_func_t ProcessFuction;
  unsigned char *short_output;
};

unsigned int read_little_endian_pushmode (unsigned char *buf, unsigned int length);
unsigned int read_little_endian_pushmode (unsigned char *buf, unsigned int length) {
  uint_t i, ret = 0;
  for (i = 0; i < length; i++) {
    ret += buf[i] << (i * 8);
  }
  return ret;
}

aubio_source_wavread_pushmode_t * new_aubio_source_wavread_pushmode(uint_t Data, wavread_pushmode_func_t ProcessFuc, uint_t hop_size) {
  aubio_source_wavread_pushmode_t * s = AUBIO_NEW(aubio_source_wavread_pushmode_t);

  if ((sint_t)hop_size <= 0) {
    AUBIO_ERR("source_wavread: Can not open with hop_size %d\n", hop_size);
    return NULL;
  }

  if (ProcessFuc == NULL) {
    AUBIO_ERR("source_wavread: ProcessFuc is null\n");
    return NULL;
  }

  s->Data = Data;
  s->hop_size = hop_size;
  s->ProcessFuction = ProcessFuc;

  return s;
}

void aubio_source_wavread_pushmode_headdata(aubio_source_wavread_pushmode_t *s, char *headbuffer, uint_t nLength){
  size_t bytes_read = 0, bytes_expected = 44;
  unsigned char* buf= NULL;
  unsigned int format, channels, sr, byterate, blockalign, bitspersample;//, data_size;

  if (headbuffer == NULL) {
	AUBIO_ERR("source_wavread_pushmode: Aborted headbuf is null\n");
	goto beach;
  }

  if(nLength < 44){
	  AUBIO_ERR("source_wavread_pushmode: Aborted the lenght(%d) is wrong\n", nLength);
	  goto beach;
  }

//	s->path = path;
//	s->samplerate = samplerate;
//  s->hop_size = hop_size;


  buf = headbuffer;


  // ChunkID
  buf+=bytes_read;
  bytes_read = 4;

  if ( memcmp((const char *)buf, "RIFF", 4) != 0 ) {
	AUBIO_ERR("source_wavread: could not find RIFF header\n");
	goto beach;
  }

  // ChunkSize
  buf+=bytes_read;
  bytes_read = 4;

  // Format
  buf+=bytes_read;
  bytes_read = 4;

  if ( memcmp((const char *)buf, "WAVE", 4) != 0 ) {
	AUBIO_ERR("source_wavread: wrong format in RIFF header\n", s);
	goto beach;
  }

  // Subchunk1ID
  buf+=bytes_read;
  bytes_read = 4;
  if ( memcmp((const char *)buf, "fmt ",4) != 0 ) {
	AUBIO_ERR("source_wavread: fmt RIFF header in\n");
	goto beach;
  }

  // Subchunk1Size
  buf+=bytes_read;
  bytes_read = 4;
  format = read_little_endian_pushmode(buf, 4);
  if ( format != 16 ) {
	// TODO accept format 18
	AUBIO_ERR("source_wavread: file is not encoded with PCM\n");
	goto beach;
  }
  if ( buf[1] || buf[2] | buf[3] ) {
	AUBIO_ERR("source_wavread: Subchunk1Size should be 0\n");
	goto beach;
  }

  // AudioFormat
  buf+=bytes_read;
  bytes_read = 2;
  if ( buf[0] != 1 || buf[1] != 0) {
	AUBIO_ERR("source_wavread: AudioFormat should be PCM\n");
	goto beach;
  }

  // NumChannels
  buf+=bytes_read;
  bytes_read = 2;
  channels = read_little_endian_pushmode(buf, 2);

  // SampleRate
  buf+=bytes_read;
  bytes_read = 4;
  sr = read_little_endian_pushmode(buf, 4);

  // ByteRate
  buf+=bytes_read;
  bytes_read = 4;
  byterate = read_little_endian_pushmode(buf, 4);

  // BlockAlign
  buf+=bytes_read;
  bytes_read = 2;
  blockalign = read_little_endian_pushmode(buf, 2);

  // BitsPerSample
  buf+=bytes_read;
  bytes_read = 2;
  bitspersample = read_little_endian_pushmode(buf, 2);
#if 0
  if ( bitspersample != 16 ) {
	AUBIO_ERR("source_wavread: can not process %dbit file\n",
		bitspersample);
	goto beach;
  }
#endif

  if ( byterate * 8 != sr * channels * bitspersample ) {
	AUBIO_ERR("source_wavread: wrong byterate\n");
	goto beach;
  }

  if ( blockalign * 8 != channels * bitspersample ) {
	AUBIO_ERR("source_wavread: wrong blockalign\n");
	goto beach;
  }

  s->samplerate = sr;
  s->channels = channels;

#if 0
  AUBIO_DBG("channels %d\n", channels);
  AUBIO_DBG("sr %d\n", sr);
  AUBIO_DBG("byterate %d\n", byterate);
  AUBIO_DBG("blockalign %d\n", blockalign);
  AUBIO_DBG("bitspersample %d\n", bitspersample);

  AUBIO_DBG("found %d channels\n", s->channels);
  AUBIO_DBG("found %d sampleraten", s->input_samplerate);

  if (samplerate == 0) {
	s->samplerate = s->input_samplerate;
  } else if (samplerate != s->input_samplerate) {
	AUBIO_ERR("source_wavread: can not resample from %d to %dHz\n",, s->input_samplerate, samplerate);
	goto beach;
  }
#endif
  AUBIO_DBG("blockalign %d\n", blockalign);

  // Subchunk2ID
  buf+=bytes_read;
  bytes_read = 4;
  if ( memcmp((const char *)buf, "data", 4) != 0 ) {
	AUBIO_ERR("source_wavread: data RIFF header not found\n");
	goto beach;
  }

  // Subchunk2Size
  buf+=bytes_read;
  bytes_read = 4;
  //data_size = buf[0] + (buf[1] << 8) + (buf[2] << 16) + (buf[3] << 24);
  //AUBIO_MSG("found %d frames in %s\n", 8 * data_size / bitspersample / channels, s->path);

#if 0
  // check the total number of bytes read is correct
  if ( bytes_read != bytes_expected ) {
#ifndef HAVE_WIN_HACKS
	AUBIO_ERR("source_wavread: short read (%zd instead of %zd) in %s\n",
#else // mingw does not know about %zd...
	AUBIO_ERR("source_wavread: short read (%d instead of %d) in %s\n",
#endif
		bytes_read, bytes_expected, s->path);
	goto beach;
  }
#endif

  s->output = new_fmat(s->channels, AUBIO_WAVREAD_BUFSIZE);
  s->blockalign= blockalign;
  s->bitspersample = bitspersample;

  s->short_output = (unsigned char *)calloc(s->blockalign, AUBIO_WAVREAD_BUFSIZE);
  s->read_index = 0;
  s->read_samples = 0;
  s->eof = 0;

  s->read_data = new_fvec (s->hop_size);

  return;

beach:
  //AUBIO_ERR("source_wavread: can not read %s at samplerate %dHz with a hop_size of %d\n",
  //	s->path, s->samplerate, s->hop_size);
  del_aubio_source_wavread_pushmode(s);
  return;

}



void aubio_source_wavread_pushmode_readframe(aubio_source_wavread_pushmode_t *s, char *pBuffer, uint_t nLength);

void aubio_source_wavread_pushmode_readframe(aubio_source_wavread_pushmode_t *s, char *pBuffer, uint_t nLength) {
	uint_t samples = nLength/s->blockalign;
	unsigned char *short_ptr = s->short_output;

	uint_t i, j, b, bitspersample = s->bitspersample;
	uint_t wrap_at = (1 << ( bitspersample - 1 ) );
	uint_t wrap_with = (1 << bitspersample);
	smpl_t scaler = 1. / wrap_at;
	int signed_val = 0;
	unsigned int unsigned_val = 0;

	//This means the stream is end
  if (nLength == 0){
	  s->eof = 1;
	  return;
  } 

  if (pBuffer == NULL){
	  return;
  } 

  memset(short_ptr, 0x0, s->blockalign*AUBIO_WAVREAD_BUFSIZE);
  memcpy(short_ptr, pBuffer, nLength);
  
//  size_t read = fread(short_ptr, s->blockalign, AUBIO_WAVREAD_BUFSIZE, s->fid);


  for (j = 0; j < samples; j++) {
    for (i = 0; i < s->channels; i++) {
      unsigned_val = 0;
      for (b = 0; b < bitspersample; b+=8 ) {
        unsigned_val += *(short_ptr) << b;
        short_ptr++;
      }
      signed_val = unsigned_val;
      // FIXME why does 8 bit conversion maps [0;255] to [-128;127]
      // instead of [0;127] to [0;127] and [128;255] to [-128;-1]
      if (bitspersample == 8) signed_val -= wrap_at;
      else if (unsigned_val >= wrap_at) signed_val = unsigned_val - wrap_with;
      s->output->data[i][j] = signed_val * scaler;
    }
  }
  
  s->read_samples = samples;
  s->read_index = 0;

}

void aubio_source_wavread_pushmode_do(aubio_source_wavread_pushmode_t * s, char *pBuffer, uint_t nLength){
  uint_t i, j;
  uint_t end = 0;
  uint_t total_wrote = 0;

  aubio_source_wavread_pushmode_readframe(s, pBuffer, nLength);

  while (1) {

	end = (s->read_samples - s->read_index >= s->hop_size - s->ReadSamples)? (s->hop_size - s->ReadSamples) : (s->read_samples - s->read_index);

    for (i = s->ReadSamples; i < end; i++) {
      s->read_data->data[i + total_wrote] = 0;
      for (j = 0; j < s->channels; j++ ) {
        s->read_data->data[i + total_wrote] += s->output->data[j][i + s->read_index];
      }
      s->read_data->data[i + total_wrote] /= (smpl_t)(s->channels);
    }
	s->ReadSamples = end;
	s->read_index+=end;

	if(end < s->hop_size){
		break;
	}

	//send the data to process
	s->ProcessFuction(s->Data, s->read_data);
	s->ReadSamples = 0;
  }
  
}
/*
void aubio_source_wavread_pushmode_do_multi(aubio_source_wavread_pushmode_t * s, fmat_t * read_data, uint_t * read){
  uint_t i,j;
  uint_t end = 0;
  uint_t total_wrote = 0;
  while (total_wrote < s->hop_size) {
    end = MIN(s->read_samples - s->read_index, s->hop_size - total_wrote);
    for (j = 0; j < read_data->height; j++) {
      for (i = 0; i < end; i++) {
        read_data->data[j][i + total_wrote] = s->output->data[j][i];
      }
    }
    total_wrote += end;
    if (total_wrote < s->hop_size) {
      uint_t wavread_read = 0;
      aubio_source_wavread_pushmode_readframe(s, &wavread_read);
      if (s->eof) {
        break;
      }
    } else {
      s->read_index += end;
    }
  }
  if (total_wrote < s->hop_size) {
    for (j = 0; j < read_data->height; j++) {
      for (i = end; i < s->hop_size; i++) {
        read_data->data[j][i] = 0.;
      }
    }
  }
  *read = total_wrote;
}
*/

uint_t aubio_source_wavread_pushmode_get_samplerate(aubio_source_wavread_pushmode_t * s) {
  return s->samplerate;
}

uint_t aubio_source_wavread_pushmode_get_channels(aubio_source_wavread_pushmode_t * s) {
  return s->channels;
}

void del_aubio_source_wavread_pushmode(aubio_source_wavread_pushmode_t * s) {
  if (!s) return;
  if (s->short_output) AUBIO_FREE(s->short_output);
  if (s->output) del_fmat(s->output);
  if (s->read_data) del_fvec(s->read_data);
  AUBIO_FREE(s);

}

#endif /* HAVE_WAVREAD */
