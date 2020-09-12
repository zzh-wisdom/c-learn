CC := gcc
CFLAGS := -Ipthread

DIR_BIN := bin

SRCS := $(wildcard *.c)
DEPS := $(SRCS:.c=.d)

pthread-create: pthread_create.o
	$(CC) $(CFLAGS) -o $(DIR_BIN)/$@ $^

pthread-cancel: pthread_cancel.o
	$(CC) $(CFLAGS) -o $(DIR_BIN)/$@ $^

pthread-cleanup: pthread_cleanup.o
	$(CC) $(CFLAGS) -o $(DIR_BIN)/$@ $^

pthread-attr-t: pthread_attr_t.o
	$(CC) $(CFLAGS) -o $(DIR_BIN)/$@ $^

pthread-setschedparam: pthread_setschedparam.o
	$(CC) $(CFLAGS) -o $(DIR_BIN)/$@ $^

# 模式规则，产生.o文件
%.o : %.c
	$(CC) $(CFLAGS) -o $@ -c $(filter %.c, $^)

include $(DEPS)

# 模式规则，产生.d文件
%.d : %.c
	@set -e;\
	$(CC) -MM $(CFLAGS) $(filter %.c,$^) | sed 's,\(.*\)\.o[ :]*,\1.o $@: ,g' > $@

format:
	@#for f in $(shell find . -name '*.c' -or -name '*.cpp' -or -name '*.h' -type f); do astyle --style=kr $$f; done
	astyle --recursive *.h,*.c,*.cpp

clean:
	rm -f *.o bin/* *.d