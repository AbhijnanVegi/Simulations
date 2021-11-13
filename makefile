CP=CoursePortal
CP_FILES= \
	$(CP)/main.c\
	$(CP)/init.c\
	$(CP)/sims.c


CFLAGS= -g -lpthread 

STD=Stadium
STD_FILES= \
	$(STD)/main.c\
	$(STD)/sims.c\


all: cp std 

cp:  $(CP_FILES)
	@echo "🚧 Building simulator for course portal"
	gcc -o cp $(CP_FILES) $(CFLAGS)
	@echo "🎉 Built successfully. Executable: cp"

cpdebug: $(CP_FILES)
	@echo "🚧 Building simulator for course portal"
	gcc -o cp $(CP_FILES) $(CFLAGS) -D DEBUG
	@echo "🎉 Built successfully. Executable: cp"

std: $(STD_FILES)
	@echo "🚧 Building simulator for stadium"
	gcc -o std $(STD_FILES) $(CFLAGS)
	@echo "🎉 Built successfully. Executable: std"