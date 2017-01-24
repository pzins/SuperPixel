#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/ml/ml.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>


using namespace std;

struct Coo {
    int x_;
    int y_;
};
void printCoo(Coo c){std::cout << c.x_ << " " << c.y_ << std::endl;}


int distance(cv::Vec3b pix1, Coo coo1, cv::Vec3b pix2, Coo coo2){

    float dist_spatiale = sqrt(pow(coo2.x_ - coo1.x_, 2) + pow(coo2.y_ - coo1.x_,2));
    float dist_color = sqrt(pow(pix1.val[0] - pix2.val[0],2) + pow(pix1.val[1] - pix2.val[1],2) + pow(pix1.val[2] - pix2.val[2],2));
    return dist_color + dist_spatiale;
}

int main(int argc, char *argv[])
{

    cv::Mat img = cv::imread("lyon.jpg");
    std::cout << img.rows << " " << img.cols << std::endl;
    int nbSuperPixel = 144; //sqrt(nbSuperPixel) doit être entier
    std::vector<Coo> cooSuperPixel;
    for(int i = 0; i < sqrt(nbSuperPixel); ++i)
    {
        for(int j = 0; j < sqrt(nbSuperPixel); ++j){
            int length_x = int(img.cols / sqrt(nbSuperPixel));
            int length_y = int(img.rows / sqrt(nbSuperPixel));
            Coo c;
            c.x_ = (i+1)*length_x;
            c.y_ = (j+1)*length_y;
            cooSuperPixel.push_back(c);
        }
    }
//    for(auto i : cooSuperPixel)
//        printCoo(i);
    cv::Mat res(img.rows, img.cols, CV_8UC1);
    cv::Mat sp(img.rows, img.cols, CV_8UC1);
    std::vector<cv::Vec3b> superPixel;
    for(auto i : cooSuperPixel)
        superPixel.push_back(img.at<cv::Vec3b>(i.x_, i.y_));

    for(int i = 0; i < img.rows; ++i)
    {
        for(int j = 0; j < img.cols; ++j)
        {
            int centroide = -1;
            int mini_dist = 10000000;
            for(int k = 0; k < superPixel.size(); ++k)
            {
                Coo c;
                c.x_ = i;
                c.y_ = j;
                int dist = distance(img.at<cv::Vec3b>(i,j), c, superPixel.at(k), cooSuperPixel.at(k));
                if(dist < mini_dist)
                {
                    centroide = k;
                    mini_dist = dist;
                }
            }
            sp.at<uchar>(i,j) = centroide;
        }
    }

    for(int i = 0; i < sp.rows; ++i)
    {
        for(int j = 0; j < sp.cols; ++j)
        {
//            std::cout <<  int(sp.at<uchar>(i,j)) << std::endl;
            int intensite = 255*(float(sp.at<uchar>(i,j))/64);
//            std::cout << intensite << std::endl;
            res.at<uchar>(i,j) = intensite;
        }
    }
    cv::namedWindow( "res");
    cv::imshow( "res", res);
    cv::waitKey(0);
    return 0;
}
