CC := gcc
CFLAGS := -I../container/list -lpthread

DIR_BIN := bin

SRCS := $(wildcard *.c)
DEPS := $(SRCS:.c=.d)

threadpool-cm-test: thread_pool-cm-test.o
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