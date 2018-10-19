/*
 * O2xIV
 * Copyright (C) 2008 Carl Reinke
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */
#include "main.h"
#include "image.h"

#include "SDL.h"
#ifdef TARGET_GP2X
#undef HAVE_STDLIB_H
#endif
#include "jpeglib.h"

#include <setjmp.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

struct jpeg_client_data {
	jmp_buf escape;
};

static void error_exit( j_common_ptr jpeg ) {
	struct jpeg_client_data *jpeg_usr = jpeg->client_data;
	
	longjmp(jpeg_usr->escape, 1);
	
	return;
}

SDL_Surface *jpeg_load( const char *filename ) {
	struct jpeg_decompress_struct jpeg;
	
	struct jpeg_client_data jpeg_usr;
	jpeg.client_data = &jpeg_usr;
	
	struct jpeg_error_mgr jpeg_err;
	jpeg.err = jpeg_std_error(&jpeg_err);
	jpeg_err.error_exit = error_exit;
	jpeg_err.output_message = error_exit;
	
	FILE *f = fopen(filename, "rb");
	
	if (f == NULL)
		return NULL;
	
	Uint8 *dst_mem = NULL;
	SDL_Surface *dst = NULL;
	
	if (setjmp(jpeg_usr.escape)) {
		printf("error: JPEG decoding failed for '%s'\n\t", filename);
		printf(jpeg_err.jpeg_message_table[jpeg_err.msg_code], jpeg_err.msg_parm.i[0], jpeg_err.msg_parm.i[1]);
		printf("\n");
		
		goto jpeg_cleanup;
	}
	
	jpeg_create_decompress(&jpeg);
	jpeg_stdio_src(&jpeg, f);
	jpeg_read_header(&jpeg, true);
	
	jpeg.mem->max_memory_to_use = 1000000;
	
	jpeg.out_color_space = JCS_RGB;
	jpeg.dct_method = JDCT_FASTEST;
	//jpeg.do_fancy_upsampling = false; // disabling this causes some images to crash libjpeg
	jpeg.do_block_smoothing = false;
	jpeg.quantize_colors = false;
	
	jpeg_calc_output_dimensions(&jpeg);
	
	int i = 0;
	while ((jpeg.output_width >> i) * (jpeg.output_height >> i) > 1500 * 1500)
		i++;
	jpeg.scale_num = 1;
	jpeg.scale_denom = 1 << i;
	
	jpeg_calc_output_dimensions(&jpeg);
	
	jpeg_start_decompress(&jpeg);
	
	dst_mem = malloc(jpeg.output_width * jpeg.output_height * 3);
	if (dst_mem) {
		JSAMPROW dst_temp[1] = { (JSAMPROW)dst_mem };
		dst = SDL_CreateRGBSurfaceFrom(dst_mem, jpeg.output_width, jpeg.output_height, 24, jpeg.output_width * 3, 0x0000ff, 0x00ff00, 0xff0000, 0);
		dst->flags &= ~SDL_PREALLOC; // this is not what we should be doing, but it'll work for now
		
		while (jpeg.output_scanline < jpeg.output_height) {
			jpeg_read_scanlines(&jpeg, dst_temp, jpeg.rec_outbuf_height);
			dst_temp[0] += jpeg.output_width * 3 * jpeg.rec_outbuf_height;
		}
	} else {
		printf("error: JPEG decoding failed for '%s'\n\t", filename);
		printf("out of memory!\n");
	}
	
	jpeg_finish_decompress(&jpeg);
	
jpeg_cleanup:
	jpeg_destroy_decompress(&jpeg);
	
	if (f)
		fclose(f);
	
	return dst;
}

// kate: tab-width 4;
