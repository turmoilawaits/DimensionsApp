#include <jni.h>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <vector>
#include <stdio.h>
#include <opencv2/opencv.hpp>
#include "opencv2/core/core.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/calib3d/calib3d.hpp"
#include <opencv2/legacy/legacy.hpp>
#include "opencv2/nonfree/nonfree.hpp"
#include<android/log.h>
using namespace std;
using namespace cv;


#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO,"native-activity",__VA_ARGS__))


extern "C" {
JNIEXPORT void JNICALL Java_org_opencv_samples_tutorial4_Tutorial4Activity_FindFeatures(JNIEnv*, jobject, jlong addrGray, jlong addrRgba);
JNIEXPORT void JNICALL Java_org_opencv_samples_tutorial4_Tutorial4Activity_load(JNIEnv*, jobject);
JNIEXPORT void JNICALL Java_org_opencv_samples_tutorial4_Tutorial4Activity_drawPts(JNIEnv*, jobject, jlong addrPts, jlong addrRgba);
JNIEXPORT void JNICALL Java_org_opencv_samples_tutorial4_Tutorial4Activity_computeMetric(JNIEnv*, jobject, jlong addrPts, jlong addrRgba);

vector<Point2f> p;
vector<Point2f> q;
vector<Point2f> metric_points;
Mat vanishing_line;
Mat vanishing_point;
vector<Point2f> scene_points, scene_points2;

std::vector<KeyPoint> keypoints_scene;
std::vector<KeyPoint> keypoints_object1;
std::vector<KeyPoint> keypoints_object2;
Mat descriptors_scene;
Mat descriptors_object1;
Mat descriptors_object2;
Mat img,img2;

//OrbFeatureDetector detector(5000);
//OrbDescriptorExtractor extractor;

SiftFeatureDetector detector(500);
SiftDescriptorExtractor extractor;
FlannBasedMatcher matcher;
  //BruteForceMatcher<HammingLUT> matcher;

vector<Point2f> perspectiveTransform_obj_points(vector<Point2f> obj_points, Mat H)
{
  std::vector<Point2f> scene_points;
  perspectiveTransform( obj_points, scene_points, H);
  return scene_points;
}

Mat getHomogeneousPoint(Point2f a)
{
  Mat l = Mat(3,1,CV_32F);
  l.at<float>(0,0) = a.x;
  l.at<float>(1,0) = a.y;
  l.at<float>(2,0) = 1;
  return l;
}

Mat getHomogeneousLine(Point2f a , Point2f b)
{
  // Mat l = Mat(3,1,CV_32F);
  // l.at<float>(0,0) = a.y - b.y;
  // l.at<float>(1,0) = b.x - a.x;
  // l.at<float>(2,0) = a.x*b.y - b.x*a.y;
  Mat l = getHomogeneousPoint(a).cross(getHomogeneousPoint(b));
  return l;
}
Mat get_vanishing_line(vector<Point2f> scene_points)
{
  Mat l0 = getHomogeneousLine(scene_points[0] , scene_points [1]);
  Mat l1 = getHomogeneousLine(scene_points[3] , scene_points [2]);
  Mat l2 = getHomogeneousLine(scene_points[3] , scene_points [0]);
  Mat l3 = getHomogeneousLine(scene_points[2] , scene_points [1]);
  Mat point_l1_l2 = l0.cross(l1);
  Mat point_l3_l4 = l2.cross(l3);
  Mat vanishing_line = point_l1_l2.cross(point_l3_l4);
  vanishing_line = vanishing_line/norm(vanishing_line);


  return vanishing_line;
}

Mat get_vanishing_point(vector<Point2f> scene_points)
{
   Mat l0 = getHomogeneousLine(scene_points[0] , scene_points [1]);
   Mat l1 = getHomogeneousLine(scene_points[2] , scene_points [3]);
   Mat vanishing_point = l0.cross(l1);
   vanishing_point = vanishing_point/vanishing_point.at<float>(2,0);
   return vanishing_point;
}

Mat get_homography(std::vector<KeyPoint> keypoints_object, Mat descriptors_object)
{
  int minHessian = 400;



  std::vector< DMatch > matches;
  matcher.match( descriptors_object, descriptors_scene, matches );

  double max_dist = 0; double min_dist = 1000;

  for( int i = 0; i < descriptors_object.rows; i++ )
  { double dist = matches[i].distance;
    if( dist < min_dist ) min_dist = dist;
    if( dist > max_dist ) max_dist = dist;
  }

    std::vector< DMatch > good_matches;



  for( int i = 0; i < descriptors_object.rows; i++ )
  { if( matches[i].distance < 3*min_dist )
     { good_matches.push_back( matches[i]); }
  }


  std::vector<Point2f> obj;
  std::vector<Point2f> scene;

  for( int i = 0; i < good_matches.size(); i++ )
  {
    obj.push_back( keypoints_object[ good_matches[i].queryIdx ].pt );
    scene.push_back( keypoints_scene[ good_matches[i].trainIdx ].pt );
  }

  Mat H = findHomography( obj, scene, CV_RANSAC );
  return H;
}

JNIEXPORT void JNICALL Java_org_opencv_samples_tutorial4_Tutorial4Activity_load(JNIEnv*, jobject)
{

//    q.push_back(Point2f(210, 196));
//    q.push_back(Point2f(212, 53));
//    q.push_back(Point2f(432, 57));
//    q.push_back(Point2f(430, 198));

	    q.push_back(Point2f(154, 158));
	    q.push_back(Point2f(139, 20));
	    q.push_back(Point2f(285, 23));
	    q.push_back(Point2f(285, 161));

//    p.push_back(Point2f(192, 432));
//    p.push_back(Point2f(192, 296));
//    p.push_back(Point2f(269, 298));
//    p.push_back(Point2f(267, 434));
	    p.push_back(Point2f(257, 239));
	        p.push_back(Point2f(268, 151));
	        p.push_back(Point2f(351, 150));
	        p.push_back(Point2f(346, 240));


	img = imread("/sdcard/Pictures/1_2.jpg");
	img2 = imread("/sdcard/Pictures/2.jpg");
	detector.detect( img, keypoints_object1 );
    extractor.compute( img, keypoints_object1, descriptors_object1 );
    detector.detect( img2, keypoints_object2 );
    extractor.compute( img2, keypoints_object2, descriptors_object2 );
}

JNIEXPORT void JNICALL Java_org_opencv_samples_tutorial4_Tutorial4Activity_FindFeatures(JNIEnv*, jobject, jlong addrGray, jlong addrRgba)
{
    Mat& mGr  = *(Mat*)addrGray;
    Mat& mRgb = *(Mat*)addrRgba;


    detector.detect( mRgb, keypoints_scene );
    extractor.compute( mRgb, keypoints_scene, descriptors_scene );


    Mat H1 = get_homography(keypoints_object1, descriptors_object1);
    Mat H2 = get_homography(keypoints_object2, descriptors_object2);
    scene_points = perspectiveTransform_obj_points(p, H1);
    vanishing_line = get_vanishing_line(scene_points);
    scene_points2 = perspectiveTransform_obj_points(q, H2);



    vanishing_point = get_vanishing_point(scene_points2);
    line(mRgb,scene_points[0],scene_points[1],Scalar(0,255,0),3);
    line(mRgb,scene_points[2],scene_points[3],Scalar(0,255,0),3);
    //line(mRgb,scene_points[4],scene_points[5],Scalar(0,255,0),3);

    line(mRgb,scene_points2[0],scene_points2[1],Scalar(0,0,255),3);
        line(mRgb,scene_points2[2],scene_points2[3],Scalar(0,0,255),3);

    char text[100];
    sprintf(text,"%.d %d", mRgb.rows, mRgb.cols);
    putText(mRgb, text, Point(100,100), 1,2, Scalar(0,255,0));

}

JNIEXPORT void JNICALL Java_org_opencv_samples_tutorial4_Tutorial4Activity_drawPts(JNIEnv*, jobject, jlong addPts, jlong addrRgba)
{

	    Mat& pTs  = *(Mat*)addPts;
	    Mat& mRgb = *(Mat*)addrRgba;
	    int i = 0;
	    for( i = 0; i < pTs.rows; i++ )
	    {
		 	float a = pTs.at<double>(i,0);
		 	float b = pTs.at<double>(i,1);
		 	LOGI("x :%f    y :%f",a,b);
		 	circle(mRgb, Point(a,b), 30, Scalar(125,125,125),-1,1);
		 	circle(mRgb, Point(a,b), 15, Scalar(100,100,100),-1,1);
	    }

}


float get_metric( Mat l, Mat v, Point2f b_reference, Point2f t_reference)
{
  Mat b_ref = getHomogeneousPoint(b_reference);
  Mat t_ref = getHomogeneousPoint(t_reference);
  double numerator = norm(b_ref.cross(t_ref));
  double denominator = l.dot(b_ref)*norm(v.cross(t_ref));
  double AlphaZ = -numerator/denominator;
  double Alpha = AlphaZ/3.7;

  Mat b_obj = getHomogeneousPoint(metric_points[0]);
  Mat t_obj = getHomogeneousPoint(metric_points[1]);
  double numer = norm(b_obj.cross(t_obj));
  double denom = l.dot(b_obj)*norm(v.cross(t_obj));
  double Zx = -numer/(Alpha*denom);
  return Zx;
}


JNIEXPORT void JNICALL Java_org_opencv_samples_tutorial4_Tutorial4Activity_computeMetric(JNIEnv*, jobject, jlong addPts, jlong addrRgba)
{
	  Mat& mRgb = *(Mat*)addrRgba;
	  Mat& pTs  = *(Mat*)addPts;
	  metric_points.clear();
	  metric_points.push_back(Point2f((float)pTs.at<double>(0,0),(float)pTs.at<double>(0,1)));
	  metric_points.push_back(Point2f((float)pTs.at<double>(1,0),(float)pTs.at<double>(1,1)));
	  Mat par_line = vanishing_point.cross(getHomogeneousPoint(metric_points[0]));
	  metric_points[1].x = (-par_line.at<float>(1,0)*metric_points[1].y - par_line.at<float>(2,0))/par_line.at<float>(0,0);
	  circle(mRgb, metric_points[0], 30, Scalar(255,0,0),-1,1);
	  circle(mRgb, metric_points[0], 15, Scalar(128,0,0),-1,1);

	  circle(mRgb, metric_points[1], 30, Scalar(255,0,0),-1,1);
	  circle(mRgb, metric_points[1], 15, Scalar(128,0,0),-1,1);
	  float height =get_metric(vanishing_line, vanishing_point , scene_points2[0], scene_points2[1]);

	  line(mRgb, metric_points[0], metric_points[1] ,Scalar(128,0,0),2);
	  char text[100];
	  sprintf(text," %.2f cm",height+0.3);
	  putText(mRgb,text, Point2f(18,17)+ 0.5*metric_points[0] + 0.5*metric_points[1], 1,3.5,Scalar(120,0,0),3);
	  putText(mRgb,text, Point2f(15,15)+ 0.5*metric_points[0] + 0.5*metric_points[1], 1,3.5,Scalar(255,0,0),3);


}



}
