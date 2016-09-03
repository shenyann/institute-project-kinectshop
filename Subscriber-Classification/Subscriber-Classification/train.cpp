#include"zmq.hpp"
#include "windows.h"  
#include "iostream"    
#include <opencv2/opencv.hpp> 
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/video/background_segm.hpp"
#include "opencv2/nonfree/features2d.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/ml/ml.hpp"
#include "opencv2/opencv_modules.hpp"

using namespace std;
using namespace cv;

CvSVM svm;
Mat canny(Mat src);

int main()
{
	int file_num=1;     
	char filename[80];
	int num_files= 265;

	float labels[265]={1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}; //class(label) 

	char *category[100]={"cup","soda water","schocolade"};							

	Mat labelsMat(num_files,1,CV_32FC1,labels);						//labels mat(num_files * 1)

	Mat training_mat(num_files,85*3,CV_32FC1);						//training mat(num_files * feature dimension)

	//svm configuration
	CvSVMParams params;												//svm params
	params.svm_type=CvSVM::C_SVC;									//svm type
	params.kernel_type=CvSVM::LINEAR;								//svm kernel 
	params.term_crit=cvTermCriteria(CV_TERMCRIT_ITER,200,1e-6);     //training criteria
	params.gamma=3;													




	for(file_num;file_num<num_files;)
	{
		int ii=0;                                                         
		Mat current_img_mat=imread(format("image/train (%i).jpg",file_num),1);	 //load image

		Mat channel_2[3];												 //define a 3-channel matrix for composition	
		split(current_img_mat,channel_2);                                //split image
		Mat greenImage=channel_2[2];									 //composition rbg channel
		Mat blueImage=channel_2[1];	
		Mat redImage=channel_2[0];						
		int r_bins=85;													//split bins for histogram calculation
		int b_bins=85;
		int g_bins=85;
		int histSize_r[]={r_bins};      //hist size							
		int histSize_b[]={b_bins};
		int histSize_g[]={g_bins};

		float r_ranges[]={0,256};            //color range
		float b_ranges[]={0,256};
		float g_ranges[]={0,256};

		const float* ranges1[]={r_ranges};                            
		const float* ranges2[]={b_ranges};
		const float* ranges3[]={g_ranges};

		int channels[]={0};
		MatND hist_base_r;
		calcHist(&current_img_mat,1,channels,Mat(),hist_base_r,1,histSize_r,ranges1,true,false);
		normalize(hist_base_r,hist_base_r,0,255,NORM_MINMAX,-1,Mat());      //normalize the histogram

		int channels2[]={1};
		MatND hist_base_b;
		calcHist(&current_img_mat,1,channels2,Mat(),hist_base_b,1,histSize_b,ranges2,true,false);
		normalize(hist_base_b,hist_base_b,0,255,NORM_MINMAX,-1,Mat());

		int channels3[]={2};
		MatND hist_base_g;
		calcHist(&current_img_mat,1,channels3,Mat(),hist_base_g,1,histSize_g,ranges3,true,false);
		normalize(hist_base_g,hist_base_g,0,255,NORM_MINMAX,-1,Mat());

		Mat tmp;                                    //define a tmp matrix for concatanate rbg histogram
		vconcat(hist_base_r,hist_base_b,tmp);       
		Mat hist_base;               
		vconcat(tmp,hist_base_g,hist_base);
		transpose(hist_base,hist_base);            //make a size=(1,255) vector per image

		cout<<hist_base.cols<<endl;                //hist.cols                
		cout<<hist_base.rows<<endl;                //hist.row

		for (MatConstIterator_<float> it = hist_base.begin<float>(); it != hist_base.end<float>(); it++) //define an iterator for histogram
		{
			training_mat.at<float>(file_num,ii++)=*it ;                    //make the training mat which size of(num_files,85*3)
		}


		file_num++;

	}

	svm.train(training_mat,labelsMat,Mat(),Mat(),params);  //train svm
	svm.save("training_svm_1.xml");                        //save svm

}

