CPPFLAGS=-I/usr/local/include/opencv/ -g
LDFLAGS=
LDLIBS=-lvlc -lcvblob -lopencv_core -lopencv_highgui -lopencv_imgproc -lopencv_video

CarCount: main.o NetworkStream.o ImageProcessor.o DataSourceManager.o CarCounter.o ObjectIdentifier.o EastboundObjectIdentifier.o WestboundObjectIdentifier.o
	g++ $(LDFLAGS) $(CPPFLAGS) -o CarCount main.o NetworkStream.o ImageProcessor.o DataSourceManager.o CarCounter.o ObjectIdentifier.o EastboundObjectIdentifier.o WestboundObjectIdentifier.o $(LDLIBS) 

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
	
EastboundObjectIdentifier.o: EastboundObjectIdentifier.h EastboundObjectIdentifier.cpp
	g++ $(CPPFLAGS) -c EastboundObjectIdentifier.cpp

WestboundObjectIdentifier.o: WestboundObjectIdentifier.h WestboundObjectIdentifier.cpp
	g++ $(CPPFLAGS) -c WestboundObjectIdentifier.cpp
	
clean:
	rm CarCount *.o
