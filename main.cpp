#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/ml/ml.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>


using namespace std;

struct Coo {
    int x;
    int y;
    Coo(int _x, int _y){x = _x; y = _y;}
};
void printCoo(Coo c){std::cout << c.x << " " << c.y << std::endl;}


class SuperPixel
{
private:
    Coo coordonnees;
    cv::Vec3b intensite;
public:
    SuperPixel(Coo _coordonnees, cv::Vec3b  _intensite) : coordonnees(_coordonnees), intensite(_intensite){}
    SuperPixel(int _x, int _y, cv::Vec3b _intensite) : intensite(_intensite), coordonnees(Coo(_x, _y)){}
};


int distance(cv::Vec3b pix1, Coo coo1, cv::Vec3b pix2, Coo coo2){

    float dist_spatiale = sqrt(pow(coo2.x - coo1.x, 2) + pow(coo2.y - coo1.x,2));
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
            Coo c((i+1)*length_x, c.y = (j+1)*length_y);
            cooSuperPixel.push_back(c);
        }
    }
//    for(auto i : cooSuperPixel)
//        printCoo(i);
    cv::Mat res(img.rows, img.cols, CV_8UC3);
    cv::Mat sp(img.rows, img.cols, CV_8UC1);
    std::vector<cv::Vec3b> superPixel;
    std::vector<std::vector<Coo>> superPixelElements;
    for(auto i : cooSuperPixel)
        superPixel.push_back(img.at<cv::Vec3b>(i.x, i.y));
    for(auto i : superPixel)
    {
        std::vector<Coo> tmp;
        superPixelElements.push_back(tmp);
    }
    for(int a = 0; a < 2; ++a){

        for(int i = 0; i < img.rows; ++i)
        {
            for(int j = 0; j < img.cols; ++j)
            {
                int centroide = -1;
                int mini_dist = 10000000;
                for(int k = 0; k < superPixel.size(); ++k)
                {
                    Coo c(i, j);
                    int dist = distance(img.at<cv::Vec3b>(i,j), c, superPixel.at(k), cooSuperPixel.at(k));
                    if(dist < mini_dist)
                    {
                        centroide = k;
                        mini_dist = dist;
                    }
                }
                sp.at<uchar>(i,j) = centroide;
                Coo tmp(i, j);
                superPixelElements.at(centroide).push_back(tmp);
            }
        }
        for(int i = 0; i < sp.rows; ++i)
        {
            for(int j = 0; j < sp.cols; ++j)
            {
    //            std::cout <<  int(sp.at<uchar>(i,j)) << std::endl;
                cv::Vec3b intensite(255*(float(sp.at<uchar>(i,j))/64),0,0);
    //            std::cout << intensite << std::endl;
                res.at<cv::Vec3b>(i,j) = intensite;
            }
        }

        for(int i = 0; i < superPixelElements.size(); ++i){
            double newx=0, newy=0;
            for(auto ele : superPixelElements.at(i))
            {
                newx += ele.x;
                newy += ele.y;
            }
            if(newx != 0 && newy != 0) {
            newx /= superPixelElements.at(i).size();
            newy /= superPixelElements.at(i).size();
            Coo tmp(newx, newy);
            cooSuperPixel.at(i) = tmp;
            superPixel.at(i) = img.at<cv::Vec3b>(tmp.x, tmp.y);
            }
        }
        std::string numero = std::to_string(a);
        cv::namedWindow( "res_"+numero);
        cv::imshow( "res_"+numero, res);
    }
    cv::waitKey(0);
    return 0;
}
