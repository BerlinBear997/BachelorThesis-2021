LIBMSR_PATH=/usr/local
build:
	g++ -DDEBUG -DHASWELL -std=c++11 -Wl,-rpath=${LIBMSR_PATH}/lib -I${LIBMSR_PATH}/include -L${LIBMSR_PATH}/lib set_uncore.c -o set_uncore -lmsr
	g++ -DDEBUG -DHASWELL -std=c++11 -Wl,-rpath=${LIBMSR_PATH}/lib -I${LIBMSR_PATH}/include -L${LIBMSR_PATH}/lib read_uncore.c -o read_uncore -lmsr
	g++ -DDEBUG -DHASWELL -std=c++11 -Wl,-rpath=${LIBMSR_PATH}/lib -I${LIBMSR_PATH}/include -L${LIBMSR_PATH}/lib set_cap.c -o set_cap -lmsr
	g++ -DDEBUG -DHASWELL -std=c++11 -Wl,-rpath=${LIBMSR_PATH}/lib -I${LIBMSR_PATH}/include -L${LIBMSR_PATH}/lib get_cap.c -o get_cap -lmsr
	g++ -DDEBUG -DHASWELL -std=c++11 -Wl,-rpath=${LIBMSR_PATH}/lib -I${LIBMSR_PATH}/include -L${LIBMSR_PATH}/lib set_core.c -o set_core -lmsr
	g++ -DDEBUG -DHASWELL -std=c++11 -Wl,-rpath=${LIBMSR_PATH}/lib -I${LIBMSR_PATH}/include -L${LIBMSR_PATH}/lib read_core.c -o read_core -lmsr

clean:
	rm -f set_uncore  read_uncore  set_cap get_cap set_core read_core
