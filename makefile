CP=CoursePortal
CP_FILES= \
	$(CP)/main.c\
	$(CP)/init.c\
	$(CP)/sims.c


CFLAGS= -g -lpthread 

all: cp

cp:  $(CP_FILES)
	@echo "🚧 Building simulator for course portal"
	gcc -o cp $(CP_FILES) $(CFLAGS)
	@echo "🎉 Built successfully. Executable: cp"

cpdebug: $(CP_FILES)
	@echo "🚧 Building simulator for course portal"
	gcc -o cp $(CP_FILES) $(CFLAGS) -D DEBUG
	@echo "🎉 Built successfully. Executable: cp"