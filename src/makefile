CPPFLAGS=-g -I/usr/local/include/opencv/
LDFLAGS=-g
LDLIBS=-lvlc -lopencv_calib3d -lopencv_contrib -lopencv_core -lopencv_features2d -lopencv_flann \
	-lopencv_gpu -lopencv_highgui -lopencv_imgproc -lopencv_legacy -lopencv_ml -lopencv_nonfree \
	-lopencv_objdetect -lopencv_photo -lopencv_stitching -lopencv_ts -lopencv_video -lopencv_videostab -lcvblob

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