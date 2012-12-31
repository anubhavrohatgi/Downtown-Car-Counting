CPPFLAGS=-I/usr/local/include/opencv/
LDFLAGS=
LDLIBS=-lvlc -lcvblob -lopencv_core -lopencv_highgui -lopencv_imgproc -lopencv_video

CarCount: main.o NetworkStream.o ImageProcessor.o DataSourceManager.o CarCounter.o ObjectIdentifier.o
	g++ $(LDFLAGS) -o CarCount main.o NetworkStream.o ImageProcessor.o DataSourceManager.o CarCounter.o ObjectIdentifier.o $(LDLIBS) 

main.o: main.cpp
	g++ $(CPPFLAGS) -c main.cpp

NetworkStream.o: NetworkStream.h NetworkStream.cpp
	g++ $(CPPFLAGS) -c NetworkStream.cpp

ImageProcessor.o: ImageProcessor.h ImageProcessor.cpp
	g++ $(CPPFLAGS) -c ImageProcessor.cpp

DataSourceManager.o: DataSourceManager.h DataSourceManager.cpp
	g++ $(CPPFLAGS) -c DataSourceManager.cpp

CarCounter.o: CarCounter.h CarCounter.cpp
	g++ $(CPPFLAGS) -c CarCounter.cpp

ObjectIdentifier.o: ObjectIdentifier.h ObjectIdentifier.cpp
	g++ $(CPPFLAGS) -c ObjectIdentifier.cpp

clean:
	rm CarCount *.o