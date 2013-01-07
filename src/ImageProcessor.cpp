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
    long double time = 0;
    while (framesProcessed < numFrames) {
        Mat frame;
        video >> frame;
        processFrame(frame, time);
        framesProcessed++;
        time += (1 / fps);
        printf("%d / %d  %f pct\n", framesProcessed, numFrames, ((double)framesProcessed / (double)numFrames) * 100);
        if (waitKey(1) >= 0) break;
    }
    carCounter->classifyObjects(true);
}

int ImageProcessor::processFrame(Mat inFrame, long currentTime)
{
    // Profiling and Stats
    long frameStart = getTime();
    if (frameCount == 0) {
        tStart = frameStart;
    }

    CvBlobs cvBlobs;
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
        } catch (cv::Exception& e) {
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
            cv::imshow("INPUT", inFrame);
            cv::imshow("BG", bgImg);
            cv::imshow("FGMASK", fgmask);
            cv::imshow("FGIMG", fgimg);
        }

        // Just do BG model for the first few frames (no blob detection)
        if (frameCount < 10) {
            return 0;
        }

// Help with quality of data?
//cv::erode(fgimg,fgimg,cv::Mat());
//cv::dilate(fgimg,fgimg,cv::Mat());

//cv::imshow("ERODED", fgimg);

        // Convert to grayscale, required by cvBlob
        try {
            cvtColor(fgimg, fgimg, CV_RGB2GRAY);
        } catch (cv::Exception& e) {
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

        printf("Blobs Size: %d\n", (int)cvBlobs.size());
        if (carCounter && cvBlobs.size() > 0) {
            vector<Blob> blobs;
            for (CvBlobs::iterator it = cvBlobs.begin(); it != cvBlobs.end(); ++it) {
                    CvBlob blob = *(it->second);
                    Blob b(blob.centroid.x, blob.centroid.y, blob.area, currentTime);
                    blobs.push_back(b);
            }
            carCounter->updateStats(blobs, currentTime);
        }

        if (showFrames) {
            cvRenderBlobs(labelImg, cvBlobs, &filtered_img, dstImg);
            cvShowImage("dstImg", dstImg);
        }
    } catch (cv::Exception& e) {
        printf("Caught Exception in ImageProcessor: %s\n", e.what());
    }

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
    return cvBlobs.size();
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
    if (labelImg) {
        cvReleaseImage(&labelImg);
    }

    if (dstImg) {
        cvReleaseImage(&dstImg);
    }
}
