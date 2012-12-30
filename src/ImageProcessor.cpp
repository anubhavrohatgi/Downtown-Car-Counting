#include "ImageProcessor.h"

using namespace cv;
using namespace std;

#define SHOW_FRAMES (TRUE)

ImageProcessor::ImageProcessor(CarCounter * c) :
    frameCount(0),
    useROI(false),
    carCounter(c),
    labelImg(NULL),
    dstImg(NULL)
{
#if SHOW_FRAMES
    cvNamedWindow("display", CV_WINDOW_AUTOSIZE);
#endif
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
#if SHOW_FRAMES
            dstImg = cvCreateImage(imgSize, IPL_DEPTH_8U, 3);
#endif
        } else {
            // Clear past img data
            cvSet(labelImg, cvScalar(0));
#if SHOW_FRAMES
            cvSet(dstImg, cvScalar(0,0,0));
#endif
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

#if SHOW_FRAMES
        Mat bgImg;
        bg_model.getBackgroundImage(bgImg);

        cv::imshow("BG", bgImg);
        cv::imshow("FGMASK", fgmask);
        cv::imshow("FGIMG", fgimg);

        cvRenderBlobs(labelImg, cvBlobs, &filtered_img, dstImg);
        cvShowImage("dstImg", dstImg);
#endif

    } catch (cv::Exception& e) {
        printf("Caught Exception in ImageProcessor: %s\n", e.what());
    }
    return cvBlobs.size();
}

ImageProcessor::~ImageProcessor()
{
    cvReleaseImage(&labelImg);
    cvReleaseImage(&dstImg);
}
