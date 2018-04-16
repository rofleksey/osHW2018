#include <stdio.h>
#include <dlfcn.h>

char* f_static();
char* f_dynamic_link();

int main() {
	printf("%s",f_static());
	printf("%s",f_dynamic_link());
	void *dynlib;
	char * (*func)();
	dynlib = dlopen("lib_dynamic_runtime.so", RTLD_LAZY);
	if (!dynlib){
		fprintf(stderr,"Can't open library libdynamic2.so: %s\n", dlerror());
		return 0;
	};
	func = dlsym(dynlib, "f_dynamic_runtime");
	printf("%s",(*func)());
	dlclose(dynlib);
	return 0;
}
