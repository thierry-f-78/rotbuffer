/*
 * Copyright (c) 2009 Thierry FOURNIER
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 2 of the License.
 *
 */

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/uio.h>

#include "rotbuffer.h"

int rotbuffer_read_buff(struct rotbuffer *r, const char *buff, int blen) {
	char *theo_end;
	int len;
	int w;

	// cant read data because the buffer is full
	if (r->buff_len == r->buff_size)
		return 0;

	// circular buffer vectors
	theo_end = r->buff_end + r->buff_size - r->buff_len;

	// first copy
	if (theo_end <= r->buff + r->buff_size) {
		len = theo_end - r->buff_end;
		if (blen < len )
			len = blen;
		memcpy(r->buff_end, buff, len);
	}

	// two vectors (rotate)
	else {
		len = r->buff + r->buff_size - r->buff_end;
		if ( blen < len )
			len = blen;
		memcpy(r->buff_end, buff, len);
		blen -= len;
		buff += len;
		w = len;

		if ( blen > 0 ) {
			len = r->buff_size - r->buff_len - len;
			if ( blen < len )
				len = blen;
			memcpy(r->buff, buff, len);
			len += w;
		}
	}	
	
	// update circular buffer ptrs
	r->buff_end += len;
	r->buff_len += len;
	if (r->buff_end >= r->buff + r->buff_size)
		r->buff_end -= r->buff_size;

	return len;
}

int rotbuffer_read_fd(struct rotbuffer *r, int fd) {
	struct iovec vector[2];
	char *theo_end;
	int vector_nb;
	int len;

	// cant read data because the buffer is full
	if (r->buff_len == r->buff_size)
		return 0;

	// circular buffer vectors
	theo_end = r->buff_end + r->buff_size - r->buff_len;

	// one vector
	if (theo_end <= r->buff + r->buff_size) {
		vector_nb			 = 1;
		vector[0].iov_base = r->buff_end;
		vector[0].iov_len  = theo_end - r->buff_end;
	}

	// two vectors (rotate)
	else {
		vector_nb			 = 2;
		vector[0].iov_base = r->buff_end;
		vector[0].iov_len  = r->buff + r->buff_size - r->buff_end;
		vector[1].iov_base = r->buff;
		vector[1].iov_len  = r->buff_size - r->buff_len - vector[0].iov_len;
	}	
	
	// read data
	len = readv(fd, (const struct iovec *)&vector, vector_nb);

	// read error
	if (len == -1) {
		if (errno == EAGAIN || errno == EINTR)
			return 0;
		else
			return -1;
	}

	// update circular buffer ptrs
	r->buff_end += len;
	r->buff_len += len;
	if (r->buff_end >= r->buff + r->buff_size)
		r->buff_end -= r->buff_size;

	return 0;
}

int rotbuffer_write_fd(struct rotbuffer *r, int fd) {
	struct iovec vector[2];
	int vector_nb;
	int len;
	
	// nothing to write
	if (r->buff_len == 0)
		return 0;

	// find writing vector
	// one vector case
	if (r->buff_start < r->buff_end) {
		vector_nb          = 1;
		vector[0].iov_base = r->buff_start;
		vector[0].iov_len  = r->buff_len;
	}
	// two vectors case
	else {
		vector_nb          = 2;
		vector[0].iov_base = r->buff_start;
		vector[0].iov_len  = r->buff + r->buff_size - r->buff_start;
		vector[1].iov_base = r->buff;
		vector[1].iov_len  = r->buff_len - vector[0].iov_len;
	}

	// writing datas for the client
	len = writev(fd, (const struct iovec *)&vector, vector_nb);

	// writing errors
	if (len == -1) {
		if (errno == EAGAIN || errno == EINTR)
			return 0;

		// in real error case
		return -1;
	}

	// update buffer
	r->buff_len -= len;
	if (r->buff_len == 0) {
		r->buff_start = r->buff;
		r->buff_end   = r->buff;
	}
	else {
		r->buff_start += len;
		if (r->buff_start >= r->buff + r->buff_size)
			r->buff_start -= r->buff_size;
	}
	return 0;
}


int rotbuffer_write_buff(struct rotbuffer *r, char *buff, int blen) {
	int len;
	int plen;
	char *wstart;
	
	/* nothing to write */
	if (r->buff_len == 0)
		return 0;

	/* set writing buffer pointer */
	wstart = r->buff_start;


	/* one vector case */
	if (r->buff_start < r->buff_end) {

		/* choose copy length */
		if (r->buff_len < blen)
			len = r->buff_len;
		else
			len = blen;

		/* copy */
		memcpy(buff, r->buff_start, len);
	}


	/* two vectors case */
	else {

		/* copy all data */
		plen = r->buff + r->buff_size - r->buff_start;
		if (plen < blen) {

			/* first buffer part */
			len = plen;
			memcpy(buff, r->buff_start, len);
			buff += len;

			/* second buffer part */

			/* copy all data */
			if (len + r->buff_len - plen < blen) {
				memcpy(buff, r->buff, len + r->buff_len - plen);
				len += r->buff_len - plen;
			}

			/* copy part */
			else {
				memcpy(buff, r->buff, blen - len);
				len = blen;
			}
				
		}

		/* copy all buffer */
		else {
			len = blen;
			memcpy(buff, r->buff_start, len);
		}
	}

	/* update buffer */
	r->buff_len -= len;
	if (r->buff_len == 0) {
		r->buff_start = r->buff;
		r->buff_end   = r->buff;
	}
	else {
		r->buff_start += len;
		if (r->buff_start >= r->buff + r->buff_size)
			r->buff_start -= r->buff_size;
	}

	/* return the len copyed */
	return len;
}
 
