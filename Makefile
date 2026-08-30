CC      = gcc
TARGET  = mandelbrot
LOGIN   = jems2
SRCDIR  = source
OBJDIR  = objects

CFLAGS  = -std=c11 -Wall -Wextra -O2 -D_POSIX_C_SOURCE=200809L -fopenmp -pthread
LDFLAGS = -fopenmp -pthread
LDLIBS  = -lm

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
  BREW_OMP := $(shell brew --prefix libomp 2>/dev/null)
  ifneq ($(BREW_OMP),)
    CC      = clang
    CFLAGS  = -std=c11 -Wall -Wextra -O2 -D_POSIX_C_SOURCE=200809L \
              -Xpreprocessor -fopenmp -I$(BREW_OMP)/include
    LDFLAGS = -L$(BREW_OMP)/lib
    LDLIBS  = -lomp -lpthread -lm
  endif
endif

SRCS = $(wildcard $(SRCDIR)/*.c)
OBJS = $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SRCS))
DEPS = $(OBJS:.o=.d)

PGMS = mandelbrot_$(LOGIN)_serial.pgm    \
       mandelbrot_$(LOGIN)_openmp.pgm    \
       mandelbrot_$(LOGIN)_pthreads1.pgm \
       mandelbrot_$(LOGIN)_pthreads2.pgm

.PHONY: all clean distclean rebuild

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

$(OBJDIR):
	mkdir -p $(OBJDIR)

rebuild: clean all

clean:
	rm -rf $(OBJDIR) $(TARGET)

distclean: clean
	rm -f $(PGMS) times.txt

-include $(DEPS)