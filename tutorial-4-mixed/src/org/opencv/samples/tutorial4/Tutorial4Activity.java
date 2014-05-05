package org.opencv.samples.tutorial4;

import java.text.SimpleDateFormat;
import java.util.Date;

import org.opencv.android.BaseLoaderCallback;
import org.opencv.android.CameraBridgeViewBase.CvCameraViewFrame;
import org.opencv.android.LoaderCallbackInterface;
import org.opencv.android.OpenCVLoader;
import org.opencv.core.CvType;
import org.opencv.core.Mat;
import org.opencv.core.Point;
import org.opencv.android.CameraBridgeViewBase;
import org.opencv.android.CameraBridgeViewBase.CvCameraViewListener2;
import org.opencv.imgproc.Imgproc;
import org.opencv.samples.tutorial4.Tutorial4Activity;

import android.app.Activity;
import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Paint.Style;
import android.os.Bundle;
import android.os.Environment;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.MotionEvent;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.view.View.OnTouchListener;
import android.view.ViewGroup;
import android.view.WindowManager;
import android.widget.LinearLayout;
import android.widget.Toast;

public class Tutorial4Activity extends Activity implements CvCameraViewListener2,OnTouchListener {
    private static final String    TAG = "OCVSample::Activity";

    private static final int       VIEW_MODE_RGBA     = 0;
    //private static final int       VIEW_MODE_GRAY     = 1;
    //private static final int       VIEW_MODE_CANNY    = 2;
    private static final int       VIEW_MODE_FEATURES = 5;
    private static final int       VIEW_MODE_MEASURE = 6;

    private int                    mViewMode;
    private Mat                    mRgba;
    private Mat                    mIntermediateMat;
    private Mat                    mGray;
    private Mat                    pTs;

    private MenuItem               mItemPreviewRGBA;
    //private MenuItem               mItemPreviewGray;
    //private MenuItem               mItemPreviewCanny;
    private MenuItem                mItemPreviewFeatures;
    private MenuItem                mItemPreviewComputeMetric;

    private CameraBridgeViewBase   mOpenCvCameraView;

   
    
    private BaseLoaderCallback  mLoaderCallback = new BaseLoaderCallback(this) {
        @Override
        public void onManagerConnected(int status) {
            switch (status) {
                case LoaderCallbackInterface.SUCCESS:
                {
                    Log.i(TAG, "OpenCV loaded successfully");

                    // Load native library after(!) OpenCV initialization
                    System.loadLibrary("mixed_sample");

                    mOpenCvCameraView.enableView();
                    mOpenCvCameraView.setOnTouchListener(Tutorial4Activity.this);
                   
                } break;
                default:
                {
                    super.onManagerConnected(status);
                } break;
            }
        }
    };

    public Tutorial4Activity() {
        Log.i(TAG, "Instantiated new " + this.getClass());
    }

    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        Log.i(TAG, "called onCreate");
        super.onCreate(savedInstanceState);
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

        setContentView(R.layout.tutorial4_surface_view);

        mOpenCvCameraView = (CameraBridgeViewBase) findViewById(R.id.tutorial4_activity_surface_view);
        mOpenCvCameraView.setCvCameraViewListener(this);
            
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        Log.i(TAG, "called onCreateOptionsMenu");
        mItemPreviewRGBA = menu.add("Preview RGBA");
       // mItemPreviewGray = menu.add("Preview GRAY");
       // mItemPreviewCanny = menu.add("Canny");
        mItemPreviewComputeMetric = menu.add("Compute metric");
        mItemPreviewFeatures = menu.add("Find features");
        
        return true;
    }

    @Override
    public void onPause()
    {
        super.onPause();
        if (mOpenCvCameraView != null)
            mOpenCvCameraView.disableView();
    }

    @Override
    public void onResume()
    {
        super.onResume();
        OpenCVLoader.initAsync(OpenCVLoader.OPENCV_VERSION_2_4_3, this, mLoaderCallback);
    }

    
    
        public boolean onTouch (View v, MotionEvent event) {
    	
        	
        Log.i(TAG,"onTouch event");
        Log.i(TAG,"Here");
        System.out.println(event.getX());
        System.out.println(event.getY());
        Mat a = new Mat(1, 2, CvType.CV_64F); // A matrix with 1 row and 3 columns
        a.put(0, 0, event.getX()); // Set row 1 , column 1
        a.put(0, 1, event.getY()); // Set row 1 , column 2   
        
        System.out.println("ok1");
        if (pTs.empty()){
        	pTs=a.clone();
        }else{
        	pTs.push_back(a);
        }
        System.out.println("ok2");
        if(pTs.rows()<=2)
        drawPts(pTs.getNativeObjAddr(),mRgba.getNativeObjAddr());
        System.out.println("ok3");
        return false;
    }

    
    public void onDestroy() {
        super.onDestroy();
        if (mOpenCvCameraView != null)
            mOpenCvCameraView.disableView();
    }

    public void onCameraViewStarted(int width, int height) {
    	
        mRgba = new Mat(height, width, CvType.CV_8UC4);
        mIntermediateMat = new Mat(height, width, CvType.CV_8UC4);
        mGray = new Mat(height, width, CvType.CV_8UC1);
        pTs = new Mat();
        load();
    }

    public void onCameraViewStopped() {
        mRgba.release();
        mGray.release();
        mIntermediateMat.release();
    }

    public Mat onCameraFrame(CvCameraViewFrame inputFrame) {
        final int viewMode = mViewMode;
        switch (viewMode) {
        /*
        case VIEW_MODE_GRAY:
            // input frame has gray scale format
            Imgproc.cvtColor(inputFrame.gray(), mRgba, Imgproc.COLOR_GRAY2RGBA, 4);
            break;
        
        case VIEW_MODE_CANNY:
            // input frame has gray scale format
            mRgba = inputFrame.rgba();
            Imgproc.Canny(inputFrame.gray(), mIntermediateMat, 80, 100);
            Imgproc.cvtColor(mIntermediateMat, mRgba, Imgproc.COLOR_GRAY2RGBA, 4);
            break;
            */
        case VIEW_MODE_RGBA:
             //input frame has RBGA format
            mRgba = inputFrame.rgba();
            break;
        case VIEW_MODE_FEATURES:
            // input frame has RGBA format
        	pTs.release();
            mRgba = inputFrame.rgba();
            mGray = inputFrame.gray();
            FindFeatures(mGray.getNativeObjAddr(), mRgba.getNativeObjAddr());
            
            mViewMode=VIEW_MODE_MEASURE;
            break;            
        case VIEW_MODE_MEASURE:
        	if(pTs.rows()==2)
        	 computeMetric(pTs.getNativeObjAddr(),mRgba.getNativeObjAddr());  
        	if(pTs.rows()>2)
        		pTs = new Mat();
        	break;
         default:
        	mRgba = inputFrame.rgba();
        	
        }

        return mRgba;
    }

    public boolean onOptionsItemSelected(MenuItem item) {
        Log.i(TAG, "called onOptionsItemSelected; selected item: " + item);

        
         if (item == mItemPreviewRGBA) {
         
            mViewMode = VIEW_MODE_RGBA;
            
        }
         /*else if (item == mItemPreviewGray) {
            mViewMode = VIEW_MODE_GRAY;
        } else if (item == mItemPreviewCanny) {
            mViewMode = VIEW_MODE_CANNY;
        } else
        */
         else if (item == mItemPreviewFeatures) {
            mViewMode = VIEW_MODE_FEATURES;
        }else if (item == mItemPreviewComputeMetric) {
            mViewMode = VIEW_MODE_MEASURE;
        }

        return true;
    }

    public native void FindFeatures(long matAddrGr, long matAddrRgba);
    public native void load();
    public native void drawPts(long matAddrPts, long matAddrRgba);
    public native void computeMetric(long matAddrPts, long matAddrRgba);


}