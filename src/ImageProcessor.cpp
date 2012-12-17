#include "ImageProcessor.h"

using namespace cv;
using namespace std;

ImageProcessor::ImageProcessor(const char * imgMaskPath, int mediaWidth, int mediaHeight, bool displayFrame, CarCounter * c) :
    frameCount(0),
    displayFrame(displayFrame),
    counter(c)
{
    imgSize.width = mediaWidth;
    imgSize.height = mediaHeight;

    imgMask = imread(imgMaskPath, CV_LOAD_IMAGE_ANYCOLOR); // Read the file
    if (imgMask.cols == 0) {
        printf("Couldn't Read: %s\n", imgMaskPath);
    }

    bgImg = imread("/Users/j3bennet/king_st_bg_temp.jpg", CV_LOAD_IMAGE_ANYCOLOR); // Read the file // TODO: WTF is this ?
    if (bgImg.cols == 0) {
        printf("Couldn't Read BG IMG: %s\n", "PATH");
    }
    if (displayFrame) {
        cvNamedWindow("display", CV_WINDOW_AUTOSIZE);
    }
}

int ImageProcessor::processFrame(Mat in_frame)
{
    // Mask Image
    CvBlobs cvBlobs;
    Mat masked;

    // Crop image
    Rect roi(265, 230, 375, 250);
    Mat cropped = in_frame(roi);

    // To gray scale image
    cvtColor(cropped, cropped, CV_BGR2GRAY);

    //cv::imshow("dstImg", cropped);

#if 1
    //cvtColor(frame, frame, CV_BGR2RGB); // TODO: fix innefficiency
    try {
    //cv::add(frame, imgMask, masked);
    }     catch (cv::Exception& e) {
        printf("Caught 1st Exception in ImageProcessor: %s\n", e.what());
    }

try {
    // Background Subtraction
    if (fgimg.empty()) {
        fgimg.create(cropped.size(), cropped.type());
    }

    //update the model
    bool update_bg_model = true; //(blobs.size() > 0 && frameCount > 300) ? false : true;
    bg_model(cropped, fgmask, update_bg_model ? -1 : 0);

    fgimg = Scalar::all(0);
    masked.copyTo(fgimg, fgmask);
    //filtered.copyTo(fgimg, fgimg);
} catch (cv::Exception& e) {
    printf("Caught 2nd Exception in ImageProcessor: %s\n", e.what());
}
    //Mat bgimg;
    //bg_model.getBackgroundImage(bgimg);

    //cvtColor(fgimg, fgimg, CV_BGR2GRAY);
    //threshold(fgimg, fgimg, 100, 255, 3);
    //fgimg = GetThresholdedImage(fgimg);
    IplImage filtered_img = fgimg;

    IplImage *dstImg = cvCreateImage(imgSize, IPL_DEPTH_8U, 3);
#endif
    imgSize.width = 640;
    imgSize.height = 480;
    IplImage *labelImg = cvCreateImage(imgSize, IPL_DEPTH_LABEL, 1);
    IplImage croppedIpl = in_frame;
    unsigned int result = cvLabel(&filtered_img, labelImg, cvBlobs);
    printf("RES %d\n", result);
    cvShowImage("HMMM", &croppedIpl);
    printf("BLOBS %d\n", cvBlobs.size());

    cvFilterByArea(cvBlobs, 100, 2000);

    if (counter) {
        vector<Blob> blobs;
        for (CvBlobs::iterator it = cvBlobs.begin(); it != cvBlobs.end(); ++it) {
                CvBlob blob = *(it->second);
                Blob * b = new Blob(blob, frameCount++);
                blobs.push_back(*b);
        }
    }

    if (displayFrame && false) {
        IplImage frame = in_frame;
        cvRenderBlobs(labelImg, cvBlobs, &frame, &frame);
        ///cv::imshow("dstImg", cropped);
    }


    //cvReleaseImage(&labelImg);
    //cvReleaseImage(&dstImg);

    return cvBlobs.size();
}

ImageProcessor::~ImageProcessor()
{

}

