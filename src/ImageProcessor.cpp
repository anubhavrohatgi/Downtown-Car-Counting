#include "ImageProcessor.h"

using namespace cv;
using namespace std;

ImageProcessor::ImageProcessor(CarCounter * c) :
    frameCount(0),
    useROI(false),
    carCounter(c),
    showFrames(showFrames),
    labelImg(NULL),
    dstImg(NULL)
{

}

void ImageProcessor::processVideoFile(const char * path)
{
    VideoCapture video(path); // open the default camera
    if(!video.isOpened()) { // check if we succeeded
        printf("Couldn't open video %s\n", path);
        return;
    }

    while (true) {
        Mat frame;
        video >> frame;
        processFrame(frame);
        // TODO: make app escape using ESC key
    }
}

int ImageProcessor::processFrame(Mat frame)
{
    CvBlobs cvBlobs;
    frameCount++;

    try {

        if (useROI) {
            frame = frame(roi);
        }

        // Convert to RGB required for background subtractor
        cvtColor(frame, frame, CV_RGBA2RGB);

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
        bg_model.operator()(frame,fgmask, learningRate);

        // Just do BG model for the first few frames (no blob detection)
        if (frameCount < 10) {
            return 0;
        }

        // Create a foreground image based on the foreground mask
        // This is essentially subtracting the background
        Mat fgimg;
        frame.copyTo(fgimg, fgmask);

// Help with quality of data?
//cv::erode(fgimg,fgimg,cv::Mat());
//cv::dilate(fgimg,fgimg,cv::Mat());

//cv::imshow("ERODED", fgimg);

        // Convert to grayscale, required by cvBlob
        cvtColor(fgimg, fgimg, CV_RGB2GRAY);

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

        printf("Blobs Size: %d\n", cvBlobs.size());
        if (carCounter) {
            vector<Blob> blobs;
            for (CvBlobs::iterator it = cvBlobs.begin(); it != cvBlobs.end(); ++it) {
                    CvBlob blob = *(it->second);
                    Blob * b = new Blob(blob, frameCount);
                    blobs.push_back(*b);
            }
        }

        if (showFrames) {
            if (frameCount == 1) {
                cvNamedWindow("display", CV_WINDOW_AUTOSIZE);
            }
            Mat bgImg;
            bg_model.getBackgroundImage(bgImg);

            cv::imshow("BG", bgImg);
            cv::imshow("FGMASK", fgmask);
            cv::imshow("FGIMG", fgimg);

            cvRenderBlobs(labelImg, cvBlobs, &filtered_img, dstImg);
            cvShowImage("dstImg", dstImg);
        }
    } catch (cv::Exception& e) {
        printf("Caught Exception in ImageProcessor: %s\n", e.what());
    }
    return cvBlobs.size();
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
