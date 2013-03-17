#include "ImageProcessor.h"
#include "cvblob.h"
#include <sys/time.h>

using namespace cv;
using namespace cvb;
using namespace std;

ImageProcessor::ImageProcessor(CarCounter * c) :
    frameCount(0),
    useROI(false),
    carCounter(c),
    labelImg(NULL),
    dstImg(NULL),
    tmin(99999), // TODO: maxint
    tmax(0),
    tavg(0),
    ttotal(0),
    tfps(0),
    jpegDumpPath(NULL),
    lastStatsPrinted(1)
{

}

void ImageProcessor::setShowFrames(bool d) {
    showFrames = d;
    if (showFrames) {
        cvNamedWindow("display", CV_WINDOW_AUTOSIZE);
    }
}

void ImageProcessor::processVideoFile(const char * path)
{
    VideoCapture video(path); // open the default camera
    if(!video.isOpened()) { // check if we succeeded
        printf("Couldn't open video %s\n", path);
        return;
    }

    int numFrames = video.get(CV_CAP_PROP_FRAME_COUNT);
    int fps = video.get(CV_CAP_PROP_FPS);
    int videoLength = numFrames / fps;
    int framesProcessed = 0;

    printf("Video: %s\nStats: %d frames, %d fps, len %ds\n", path, numFrames, fps, videoLength);
    long time = 0;
    bool paused = false;
    while (framesProcessed < numFrames) {
        Mat frame;
        video >> frame;
        printf("time %ld\n", time);
        char buf[60];
        sprintf(buf, "%d  %ld", framesProcessed, time);
        Point org(0,300);
        putText(frame, buf, org, FONT_HERSHEY_SIMPLEX, 1, CV_RGB(0,0,0), 2);
        processFrame(frame, time);
        framesProcessed++;
        time += (1000 / fps);
        printf("%d / %d  %f pct\n", framesProcessed, numFrames, ((double)framesProcessed / (double)numFrames) * 100);
        int key = waitKey(1);
        if (key == 'p') {
            paused = !paused;
            printf("PAUSED %d\n", paused);
        }
        if (paused) {
            // When paused, any key can be used to process next frame
            while(key < 0) {
                key = cvWaitKey(1);
                if (key == 'p') {
                    paused = !paused;
                    printf("PAUSED %d\n", paused);
                }
            }
        }
    }
    // Force classification of remaining objects
    carCounter->classifyObjects(true, 0);
}

int ImageProcessor::processFrame(Mat& inFrame, long currentTime)
{
    // Profiling and Stats
    long frameStart = getTime();
    if (frameCount == 0) {
        tStart = frameStart;
    }

    CvBlobs cvBlobs;
    vector<Blob*> blobs;
    frameCount++;

    try {
        Mat frame;
        if (useROI) {
            frame = inFrame(roi);
        } else {
            frame = inFrame;
        }

        // Convert to RGB required for background subtractor
        try {
            cvtColor(frame, frame, CV_RGBA2RGB);
        } catch (Exception& e) {
            printf("Caught Exception in cvtColor(frame, frame, CV_RGBA2RGB): %s\n", e.what());
            return 0;
        }

        // Slow down learning rate over time as we have enough images for a stable BG image
        double learningRate;
        if (frameCount < 100) {
            learningRate = -1;
        } else if (frameCount < 1000) {
            learningRate = -.1;
        } else {
            learningRate = -0.001;
        }

        Mat fgmask;
        bg_model.operator()(frame, fgmask, learningRate);

        // Create a foreground image based on the foreground mask
        // This is essentially subtracting the background
        Mat fgimg;
        frame.copyTo(fgimg, fgmask);

        if (showFrames) {
            Mat bgImg;
            bg_model.getBackgroundImage(bgImg);
            imshow("INPUT", inFrame);
            imshow("BG", bgImg);
            imshow("FGMASK", fgmask);
            imshow("FGIMG", fgimg);
        }

        // Just do BG model for the first few frames (no blob detection)
        if (frameCount < 10) {
            return 0;
        }

// Help with quality of data?
//erode(fgimg,fgimg,Mat());
//dilate(fgimg,fgimg,Mat());

//imshow("ERODED", fgimg);

        // Convert to grayscale, required by cvBlob
        try {
            cvtColor(fgimg, fgimg, CV_RGB2GRAY);
        } catch (Exception& e) {
            printf("Caught Exception in cvtColor(fgimg, fgimg, CV_RGB2GRAY): %s\n", e.what());
            //return 0;
        }

        //threshold(fgimg, fgimg, 100, 255, 3);
        //fgimg = GetThresholdedImage(fgimg);

        // Use old-style OpenCV IplImage required by cvBlob
        IplImage filtered_img = fgimg;

        if (labelImg == NULL) {
            CvSize imgSize;
            imgSize.width = frame.cols;
            imgSize.height = frame.rows;
            labelImg = cvCreateImage(imgSize, IPL_DEPTH_LABEL, 1);
            if (showFrames) {
                dstImg = cvCreateImage(imgSize, IPL_DEPTH_8U, 3);
            }
        } else {
            // Clear past img data
            cvZero(labelImg);
            if (showFrames) {
                cvZero(dstImg);
            }
        }

        // Run blob-detection algorithm from cvBlob
        cvLabel(&filtered_img, labelImg, cvBlobs);

        // Filter out really small blobs
        cvFilterByArea(cvBlobs, 100, 20000);

        for (CvBlobs::iterator it = cvBlobs.begin(); it != cvBlobs.end(); ++it) {
            CvBlob *blob = it->second;
            Blob* b = new Blob(blob->centroid.x, blob->centroid.y, blob->minx, blob->maxx, blob->miny, blob->maxy, blob->area, currentTime);
            blobs.push_back(b);
        }
        if (carCounter) {
            carCounter->processBlobs(blobs, currentTime);
        }

        // Add classified blobs on top of original image
        for (unsigned int i = 0; (showFrames || jpegDumpPath) && i < blobs.size(); i++) {
            Blob *b = blobs.at(i);
            printf("b == NULL %d\n", (b==NULL));
            if (b != NULL) {
                Rect r = Rect(b->minx + roi.x, b->miny + roi.y, b->maxx - b->minx, b->maxy - b->miny);
                CvScalar color;
                switch(b->getClusterId()) {
                case 2:
                    color = CV_RGB(0xFF,0,0); // RED
                    break;
                case 3:
                    color = CV_RGB(0,0xFF,0); // GREEN
                    break;
                case 4:
                    color = CV_RGB(0,0,0xFF); // BLUE
                    break;
                case 5:
                    color = CV_RGB(0xC0,0xC0,0xC0); // SILVER
                    break;
                case 6:
                    color = CV_RGB(0xFF, 0x63, 0x47); // TOMATO
                    break;
                case 7:
                    color = CV_RGB(0x80,0,0x80); // PURPLE
                    break;
                case 8:
                    color = CV_RGB(0xFF,0xFF,0xFF); // WHITE
                    break;
                default:
                    color = CV_RGB(0xFF,0xFF,0); // YELLOW
                    break;
                }
                printf("RECT %d\n", b->getClusterId());
                rectangle(inFrame, r, color, 5); //thickness can be CV_FILLED
            }
        }

        if (jpegDumpPath) {
            // Write jpeg to file
            char buf[64];
            sprintf(buf, "%s/image%09d.jpg", jpegDumpPath, (frameCount - 10));
            bool res = imwrite(buf, inFrame);
            if (!res) {
                printf("Could not write %s\n", buf);
            }
        }

        if (showFrames) {
            cvRenderBlobs(labelImg, cvBlobs, &filtered_img, dstImg);
            cvShowImage("dstImg", dstImg);
            imshow("ALGO", inFrame);
        }
    } catch (Exception& e) {
        printf("Caught Exception in ImageProcessor: %s\n", e.what());
    }

    // Release all CvBlob memory
    cvReleaseBlobs(cvBlobs);

    // Handle Stats
    long frameEnd = getTime();
    long processTime = frameEnd - frameStart;
    tmin = (processTime < tmin) ? processTime : tmin;
    tmax = (processTime > tmax) ? processTime : tmax;
    ttotal += processTime;
    tfps += processTime;
    tavg = ttotal / frameCount;

    // Print stats every 10 frames
    if (frameCount % 10 == 0) {
        double fps = (frameCount - lastStatsPrinted) * 1000 / tfps;
        double global_fps = frameCount * 1000 / ttotal;
        printf("min,max,avg   %ld,%ld,%ld\n", tmin,tmax,tavg);
        printf("recent_fps,global_fps  %f,%f\n", fps, global_fps);
        lastStatsPrinted = frameCount;
        tfps = 0;
    }
    return blobs.size();
}

// Returns time in ms
long ImageProcessor::getTime()
{
    timeval profile;
    gettimeofday(&profile, NULL);
    long time = (profile.tv_sec * 1000 * 1000) + (profile.tv_usec);
    return time / 1000;
}

ImageProcessor::~ImageProcessor()
{
    printf("~ImageProcessor\n");
    if (labelImg) {
        cvReleaseImage(&labelImg);
    }

    if (dstImg) {
        cvReleaseImage(&dstImg);
    }
}
