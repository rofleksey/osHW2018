project: main.o lib_static.a lib_dynamic_link.so lib_dynamic_runtime.so
	gcc -o project main.o -L. -l lib_static.a -ldl -l lib_dynamic_link.so -Wl,-rpath,.

main.o: main.c
	gcc -c main.c

lib_static.a: lib_static.o
	ar cr lib_static.a lib_static.o

lib_static.o: lib_static.c
	gcc -c lib_static.c

lib_dynamic_link.so: lib_dynamic_link.o
	gcc -shared -o lib_dynamic_link.so lib_dynamic_link.o

lib_dynamic_link.o: lib_dynamic_link.c
	gcc -c -fPIC lib_dynamic_link.c

lib_dynamic_runtime.so: lib_dynamic_runtime.o
	gcc -shared -o lib_dynamic_runtime.so lib_dynamic_runtime.o

lib_dynamic_runtime.o: lib_dynamic_runtime.c
	gcc -c -fPIC lib_dynamic_runtime.c

clean:
	-rm -f *.o *.so *.a
