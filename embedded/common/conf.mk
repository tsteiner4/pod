CPU_DEFINES += -D__START=main

SRCS += common/startup.c
SRCS += common/board.c
SRCS += common/pcbuffer.c
SRCS += common/retarget.c
SRCS += common/console.c
SRCS += common/post.c
SRCS += common/pin_alias.c

include common/commands/conf.mk
