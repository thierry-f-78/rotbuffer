CPPFLAGS = 
CFLAGS = -O0 -g -Wall
LDFLAGS = 

OBJS = rotbuffer.o

librotbuffer.a: $(OBJS)
	$(AR) -rvc librotbuffer.a $(OBJS)

clean:
	rm -f $(OBJS)

doc:
	doxygen rotbuffer.doxygen
 
cleandoc:
	rm -rf html man

