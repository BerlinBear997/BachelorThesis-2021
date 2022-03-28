LIBMSR_PATH=/usr/local/
CXX=g++
#FLAG=-DDEBUG
#FLAG=-DOVERHEAD -std=c++11 -Wl,-rpath=${LIBMSR_PATH}//lib -I${LIBMSR_PATH}//include -L${LIBMSR_PATH}//lib
#FLAG=-DPOWERCAP -DBROADWELL -std=c++11 -Wl,-rpath=${LIBMSR_PATH}//lib -I${LIBMSR_PATH}//include -L${LIBMSR_PATH}//lib
#FLAG=-DDEBUG -DMANAGEMENT -DPOWERCAP -DBROADWELL -std=c++11 -Wl,-rpath=${LIBMSR_PATH}/lib -I${LIBMSR_PATH}/include -L${LIBMSR_PATH}/lib
FLAG=-DDEBUG -DSAMPLING -DMANAGEMENT -DENERGY -DHASWELL -DROOFLINE -std=c++11 -Wl,-rpath=${LIBMSR_PATH}/lib -I${LIBMSR_PATH}/include -L${LIBMSR_PATH}/lib

build: 
	${CXX} -fPIC ${FLAG}  -c node.cpp 
	#${CXX} -fPIC ${FLAG}  -c execution.cpp
	#${CXX} ${FLAG} -c standalone.cpp 
	#${CXX} ${FLAG} standalone.o node.o -o standalone -lmsr
	#${CXX} -fPIC  -I/work/bw579735/libmsr/install/include -DSKYLAKE -c node.cpp 
	#${CXX} -fPIC  -std=c++11 -DSKYLAKE -c execution.cpp

	${CXX} ${FLAG} -fopenmp -c timesampling.cpp 
	${CXX} ${FLAG} -fopenmp node.o timesampling.o -o timesampling -lmsr
#	${CXX} ${FLAG} -fopenmp -c ins_sampling.cpp
#	${CXX} ${FLAG} -fopenmp node.o ins_sampling.o -o inssampling -lmsr

clean:
	rm -rf *.o timesampling
