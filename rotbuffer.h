/*
 * Copyright (c) 2008 Thierry FOURNIER
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 2 of the License.
 *
 */

/** @file */

#ifndef __ROTBUFFER_H__
#define __ROTBUFFER_H__

struct rotbuffer {
	char *buff_end;
	char *buff_start;
	int buff_len;
	int buff_size;
	char *buff;
};

/** 
 * read max data from fd 
 * @param r struct rotbuffer
 * @param fd file descriptor
 */
int rotbuffer_read_fd(struct rotbuffer *r, int fd);

/**
 * copy data from buffer
 * @param r struct rotbuffer
 * @param buff buffer
 * @param blen buffer len
 * @return size writed
 */
int rotbuffer_read_buff(struct rotbuffer *r, const char *buff, int blen);

/** 
 * write max data to fd
 * @param r struct rotbuffer
 * @param fd file descriptor
 */
int rotbuffer_write_fd(struct rotbuffer *r, int fd);

/** 
 * add byte at end of stream. don't check for avalaible space
 * @param r struct rotbuffer
 * @param c char
 * @return 0 no space left, 1 if ok
 */
static inline void rotbuffer_add_byte_wc(struct rotbuffer *r, char c) {
	*r->buff_end = c;
	r->buff_end++;
	r->buff_len++;
	if (r->buff_end >= r->buff + r->buff_size)
		r->buff_end = r->buff;
}

/** 
 * add byte at end of stream
 * @param r struct rotbuffer
 * @param c char
 * @return 0 no space left, 1 if ok
 */
static inline int rotbuffer_add_byte(struct rotbuffer *r, char c) {
	if (r->buff_len + 1 > r->buff_size)
		return 0;
	rotbuffer_add_byte_wc(r, c);
	return 1;
}

/** 
 * add byte at position defined in stream.
 * warning: function without integrity control.
 * @param r struct rotbuffer
 * @param pos position to add
 * @param c char
 * @return next postion
 */
static inline char *rotbuffer_add_byte_at_pos(struct rotbuffer *r,
                                              char *pos, char c) {
	*pos = c;
	pos++;
	if (pos >= r->buff + r->buff_size)
		pos = r->buff;
	return pos;
}

/**
 * return free space on rot buffer
 * @param r struct rotbuffer
 * @return space avalaible
 */
static inline int rotbuffer_free_size(struct rotbuffer *r) {
	return r->buff_size - r->buff_len;
}

/**
 * get current position in buffer
 * @param r struct rotbuffer
 * @return current position
 */
static inline char *rotbuffer_store(struct rotbuffer *r) {
	return r->buff_end;
}

/**
 * seek space into buffer without space check.
 * @param r struct rotbuffer
 * @param seek seek value
 */
static inline void rotbuffer_seek_wc(struct rotbuffer *r, int seek) {
	r->buff_len += seek;
	r->buff_end += seek;
	if (seek >= 0) {
		if (r->buff_end >= r->buff + r->buff_size)
			r->buff_end -= r->buff_size;
	} else {
		if (r->buff_end < r->buff)
			r->buff_end += r->buff_size;
	}
}

/**
 * seek space into buffer with space check.
 * @param r struct rotbuffer
 * @param seek seek value
 * @return 0: no space avalaible, 1 ok
 */
static inline int rotbuffer_seek(struct rotbuffer *r, int seek) {
	if (r->buff_len + seek > r->buff_size)
		return 0;
	rotbuffer_seek_wc(r, seek);
	return 1;
}

/**
 * return the len from pos1 to pos2
 * @param r struct rotbuffer
 * @param p1 position 1 (returned by rotbuffer_store)
 * @param p2 position 2 (returned by rotbuffer_store)
 * @return the space used
 * @see rotbuffer_store
 */
static inline int rotbuffer_pos_diff(struct rotbuffer *r, char *p1, char *p2) {
	int l;
	l = p2 - p1;
	if (l<0)
		l+=r->buff_size;
	return l;
}
#endif
