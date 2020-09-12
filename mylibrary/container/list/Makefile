MKDIR := mkdir
RM := rm -rf      # -r递归删除， -f强制删除文件或目录

CC := gcc
CFLAGS :=

DIR_SRC := .

DIR_BIN := bin
DIR_OBJ := obj
DIR_DEP := dep

SRCS := $(wildcard *.c)
OBJS := $(SRCS:.c=.o)
OBJS := $(foreach f,$(OBJS),$(DIR_OBJ)/$(f))    # 将生成的.o文件均放到obj目录下
DEPS := $(SRCS:.c=.d)
DEPS := $(foreach f,$(DEPS),$(DIR_DEP)/$(f))    # 将生成的.d文件均放到dep目录下

CREATE_DIRS := $(DIR_BIN) $(DIR_OBJ) $(DIR_DEP)
EXE := list-test

all:$(DIR_OBJ) $(DIR_BIN) $(EXE)

# 这个不要放all前面，否则make默认执行的就不是all了
ifeq ("$(MAKECMDGOALS)","all")    # MAKECMDGOALS表示当前make命令生成的目标，若执行命令为"make"或者“make all”则包含“-include $(DEPS)”
include $(DEPS)
endif

ifeq ("$(MAKECMDGOALS)","")
include $(DEPS)
endif

ifeq ("$(MAKECMDGOALS)","list-test")
include $(DIR_DEP)/list-test.d
endif

list-test: creat_dirs $(DIR_OBJ)/list-test.o
	$(CC) $(CFLAGS) -o $(DIR_BIN)/$@ $(filter %.o, $^)

clean:
	$(RM) $(CREATE_DIRS)

creat_dirs: $(DIR_OBJ) $(DIR_BIN)

$(CREATE_DIRS) :
	$(MKDIR) $@

# 模式规则，产生.o文件
$(DIR_OBJ)/%.o : $(DIR_SRC)/%.c
	@#echo '$^'  # 这里打印的依赖文件包含头文件.h，有点神奇
	$(CC) $(CFLAGS) -o $@ -c $(filter %.c, $^)   

# 模式规则，产生.d文件
ifeq ("$(wildcard $(DIR_DEP))","")   # 根据是否含有dep文件夹，进行创建
$(DIR_DEP)/%.d : $(DIR_DEP) $(DIR_SRC)/%.c
else
$(DIR_DEP)/%.d : $(DIR_SRC)/%.c
endif
	@#echo "Creating $@ ..."
	@set -e;\
	$(CC) -MM $(CFLAGS) $(filter %.c,$^) | sed 's,\(.*\)\.o[ :]*,$(DIR_OBJ)/\1.o $@: ,g' > $@

.PHONY : all clean   # 标志标签
