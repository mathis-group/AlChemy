# The subdirs we intend to build every time we say "make"
SUBDIRS = IPC/Socket IPC/Shmem LambdaReactor TypeSynthesis

all:
	@for d in $(SUBDIRS); do (make -C $$d); done

nuke:
	@for d in $(SUBDIRS); do (make -C $$d nuke); done

clean:
	@for d in $(SUBDIRS); do (make -C $$d clean); done
