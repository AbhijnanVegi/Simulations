CP=CoursePortal
CP_FILES= \
	$(CP)/main.c\
	$(CP)/init.c


CFLAGS= -g -lpthread 

cp:  $(CP_FILES)
	@echo "Building simulator for course portal"
	gcc -o cp $(CP_FILES) $(CFLAGS)
	@echo "Built successfully. Executable: cp"